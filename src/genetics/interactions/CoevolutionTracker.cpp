#include "genetics/interactions/CoevolutionTracker.hpp"
#include "genetics/organisms/Plant.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "objects/creature/creature.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Constructors
// ============================================================================

CoevolutionTracker::CoevolutionTracker()
    : currentGeneration_(0)
    , cacheValid_(false)
{
    initializeDefaultPairs();
}

CoevolutionTracker::CoevolutionTracker(const ArmsRaceConfig& config)
    : config_(config)
    , currentGeneration_(0)
    , cacheValid_(false)
{
    // If no pairs specified in config, use defaults
    if (config_.trackedPairs.empty()) {
        initializeDefaultPairs();
    }
}

void CoevolutionTracker::initializeDefaultPairs() {
    // Toxin arms race
    config_.trackedPairs.push_back({
        UniversalGenes::TOXIN_TOLERANCE,
        UniversalGenes::TOXIN_PRODUCTION
    });
    config_.trackedPairs.push_back({
        UniversalGenes::TOXIN_METABOLISM,
        UniversalGenes::TOXIN_PRODUCTION
    });
    
    // Physical defense arms race
    config_.trackedPairs.push_back({
        UniversalGenes::HIDE_THICKNESS,
        UniversalGenes::THORN_DENSITY
    });
    
    // Seed survival arms race
    config_.trackedPairs.push_back({
        UniversalGenes::SEED_DESTRUCTION_RATE,
        UniversalGenes::SEED_COAT_DURABILITY
    });
    
    // Fruit attraction coevolution (mutualistic)
    config_.trackedPairs.push_back({
        UniversalGenes::COLOR_VISION,
        UniversalGenes::FRUIT_APPEAL
    });
    
    // Scent detection coevolution
    config_.trackedPairs.push_back({
        UniversalGenes::SCENT_DETECTION,
        UniversalGenes::FRUIT_APPEAL
    });
    
    // Digestion efficiency vs nutrient content
    config_.trackedPairs.push_back({
        UniversalGenes::PLANT_DIGESTION_EFFICIENCY,
        UniversalGenes::NUTRIENT_VALUE
    });
}

// ============================================================================
// Population Recording
// ============================================================================

void CoevolutionTracker::recordCreatureGeneration(const std::vector<Creature>& creatures) {
    currentCreatureData_ = GeneFrequencyRecord();
    currentCreatureData_.generation = currentGeneration_;
    
    if (creatures.empty()) {
        return;
    }
    
    // Collect values for each tracked creature gene
    std::map<std::string, std::vector<float>> geneValues;
    
    for (const auto& pair : config_.trackedPairs) {
        geneValues[pair.first] = std::vector<float>();
    }
    
    // Extract gene values from each creature
    for (const auto& creature : creatures) {
        // Get phenotype from creature's new genetics system
        const Phenotype* phenotype = creature.getPhenotype();
        if (!phenotype) {
            continue;
        }
        
        for (const auto& pair : config_.trackedPairs) {
            const std::string& geneName = pair.first;
            if (phenotype->hasTrait(geneName.c_str())) {
                geneValues[geneName].push_back(phenotype->getTrait(geneName.c_str()));
            }
        }
    }
    
    // Calculate statistics for each gene
    for (auto& kv : geneValues) {
        const std::string& geneName = kv.first;
        const std::vector<float>& values = kv.second;
        
        if (!values.empty()) {
            currentCreatureData_.meanValues[geneName] = calculateMean(values);
            currentCreatureData_.variances[geneName] = calculateVariance(values);
            currentCreatureData_.sampleCounts[geneName] = static_cast<int>(values.size());
        }
    }
    
    cacheValid_ = false;
}

void CoevolutionTracker::recordPlantGeneration(const std::vector<Plant>& plants) {
    currentPlantData_ = GeneFrequencyRecord();
    currentPlantData_.generation = currentGeneration_;
    
    if (plants.empty()) {
        return;
    }
    
    // Collect values for each tracked plant gene
    std::map<std::string, std::vector<float>> geneValues;
    
    for (const auto& pair : config_.trackedPairs) {
        geneValues[pair.second] = std::vector<float>();
    }
    
    // Extract gene values from each plant
    for (const auto& plant : plants) {
        const Phenotype& phenotype = plant.getPhenotype();
        
        for (const auto& pair : config_.trackedPairs) {
            const std::string& geneName = pair.second;
            if (phenotype.hasTrait(geneName.c_str())) {
                geneValues[geneName].push_back(phenotype.getTrait(geneName.c_str()));
            }
        }
    }
    
    // Calculate statistics for each gene
    for (auto& kv : geneValues) {
        const std::string& geneName = kv.first;
        const std::vector<float>& values = kv.second;
        
        if (!values.empty()) {
            currentPlantData_.meanValues[geneName] = calculateMean(values);
            currentPlantData_.variances[geneName] = calculateVariance(values);
            currentPlantData_.sampleCounts[geneName] = static_cast<int>(values.size());
        }
    }
    
    cacheValid_ = false;
}

void CoevolutionTracker::advanceGeneration() {
    // Store current data in history
    creatureHistory_.push_back(currentCreatureData_);
    plantHistory_.push_back(currentPlantData_);
    
    // Trim history if too long
    while (creatureHistory_.size() > static_cast<size_t>(config_.maxHistoryGenerations)) {
        creatureHistory_.pop_front();
    }
    while (plantHistory_.size() > static_cast<size_t>(config_.maxHistoryGenerations)) {
        plantHistory_.pop_front();
    }
    
    // Advance generation counter
    currentGeneration_++;
    
    // Reset current data
    currentCreatureData_ = GeneFrequencyRecord();
    currentPlantData_ = GeneFrequencyRecord();
    
    cacheValid_ = false;
}

// ============================================================================
// Arms Race Detection
// ============================================================================

bool CoevolutionTracker::isArmsRaceActive(
    const std::string& creatureGene,
    const std::string& plantGene
) const {
    // Need minimum generations of data
    if (static_cast<int>(creatureHistory_.size()) < config_.minGenerationsForTrend) {
        return false;
    }
    
    // Get correlation
    float strength = getCoevolutionStrength(creatureGene, plantGene);
    
    if (strength < config_.correlationThreshold) {
        return false;
    }
    
    // Get trend
    CoevolutionStats stats = getCoevolutionStats(creatureGene, plantGene);
    
    // Arms race is active if escalating or oscillating
    return stats.trend == "escalating" || stats.trend == "oscillating";
}

float CoevolutionTracker::getCoevolutionStrength(
    const std::string& creatureGene,
    const std::string& plantGene
) const {
    // Check cache
    auto cacheKey = std::make_pair(creatureGene, plantGene);
    if (cacheValid_) {
        auto it = correlationCache_.find(cacheKey);
        if (it != correlationCache_.end()) {
            return it->second;
        }
    }
    
    // Build value vectors from history
    std::vector<float> creatureValues;
    std::vector<float> plantValues;
    
    for (const auto& record : creatureHistory_) {
        auto it = record.meanValues.find(creatureGene);
        if (it != record.meanValues.end()) {
            creatureValues.push_back(it->second);
        }
    }
    
    for (const auto& record : plantHistory_) {
        auto it = record.meanValues.find(plantGene);
        if (it != record.meanValues.end()) {
            plantValues.push_back(it->second);
        }
    }
    
    // Need matching data points
    size_t minSize = std::min(creatureValues.size(), plantValues.size());
    if (minSize < 2) {
        return 0.0f;
    }
    
    creatureValues.resize(minSize);
    plantValues.resize(minSize);
    
    // Calculate correlation
    float correlation = calculateCorrelation(creatureValues, plantValues);
    float strength = std::abs(correlation);
    
    // Cache result
    correlationCache_[cacheKey] = strength;
    
    return strength;
}

// ============================================================================
// Statistics Retrieval
// ============================================================================

CoevolutionStats CoevolutionTracker::getCoevolutionStats(
    const std::string& creatureGene,
    const std::string& plantGene
) const {
    CoevolutionStats stats;
    stats.creatureGene = creatureGene;
    stats.plantGene = plantGene;
    
    // Build value vectors from history
    std::vector<float> creatureValues;
    std::vector<float> plantValues;
    
    for (const auto& record : creatureHistory_) {
        auto it = record.meanValues.find(creatureGene);
        if (it != record.meanValues.end()) {
            creatureValues.push_back(it->second);
        }
    }
    
    for (const auto& record : plantHistory_) {
        auto it = record.meanValues.find(plantGene);
        if (it != record.meanValues.end()) {
            plantValues.push_back(it->second);
        }
    }
    
    // Need matching data points
    size_t minSize = std::min(creatureValues.size(), plantValues.size());
    if (minSize < 2) {
        stats.generationsTracked = 0;
        stats.trend = "insufficient_data";
        return stats;
    }
    
    creatureValues.resize(minSize);
    plantValues.resize(minSize);
    
    stats.generationsTracked = static_cast<int>(minSize);
    stats.correlationCoefficient = calculateCorrelation(creatureValues, plantValues);
    stats.trend = detectTrend(creatureValues, plantValues);
    
    // Current values (most recent)
    if (!creatureValues.empty()) {
        stats.creatureMeanValue = creatureValues.back();
        stats.creatureVariance = calculateVariance(creatureValues);
    }
    if (!plantValues.empty()) {
        stats.plantMeanValue = plantValues.back();
        stats.plantVariance = calculateVariance(plantValues);
    }
    
    return stats;
}

std::vector<CoevolutionStats> CoevolutionTracker::getActiveArmsRaces() const {
    std::vector<CoevolutionStats> active;
    
    for (const auto& pair : config_.trackedPairs) {
        if (isArmsRaceActive(pair.first, pair.second)) {
            active.push_back(getCoevolutionStats(pair.first, pair.second));
        }
    }
    
    return active;
}

std::vector<CoevolutionStats> CoevolutionTracker::getAllTrackedPairs() const {
    std::vector<CoevolutionStats> all;
    
    for (const auto& pair : config_.trackedPairs) {
        all.push_back(getCoevolutionStats(pair.first, pair.second));
    }
    
    return all;
}

// ============================================================================
// Configuration
// ============================================================================

void CoevolutionTracker::addTrackedPair(
    const std::string& creatureGene,
    const std::string& plantGene
) {
    // Check if already tracked
    for (const auto& pair : config_.trackedPairs) {
        if (pair.first == creatureGene && pair.second == plantGene) {
            return;
        }
    }
    
    config_.trackedPairs.push_back({creatureGene, plantGene});
    cacheValid_ = false;
}

void CoevolutionTracker::removeTrackedPair(
    const std::string& creatureGene,
    const std::string& plantGene
) {
    auto it = std::remove_if(
        config_.trackedPairs.begin(),
        config_.trackedPairs.end(),
        [&](const std::pair<std::string, std::string>& pair) {
            return pair.first == creatureGene && pair.second == plantGene;
        }
    );
    
    config_.trackedPairs.erase(it, config_.trackedPairs.end());
    cacheValid_ = false;
}

void CoevolutionTracker::setConfig(const ArmsRaceConfig& config) {
    config_ = config;
    cacheValid_ = false;
}

// ============================================================================
// History Access
// ============================================================================

std::vector<std::pair<int, float>> CoevolutionTracker::getCreatureGeneHistory(
    const std::string& geneName
) const {
    std::vector<std::pair<int, float>> history;
    
    for (const auto& record : creatureHistory_) {
        auto it = record.meanValues.find(geneName);
        if (it != record.meanValues.end()) {
            history.push_back({record.generation, it->second});
        }
    }
    
    return history;
}

std::vector<std::pair<int, float>> CoevolutionTracker::getPlantGeneHistory(
    const std::string& geneName
) const {
    std::vector<std::pair<int, float>> history;
    
    for (const auto& record : plantHistory_) {
        auto it = record.meanValues.find(geneName);
        if (it != record.meanValues.end()) {
            history.push_back({record.generation, it->second});
        }
    }
    
    return history;
}

void CoevolutionTracker::clearHistory() {
    creatureHistory_.clear();
    plantHistory_.clear();
    correlationCache_.clear();
    cacheValid_ = false;
}

void CoevolutionTracker::reset() {
    clearHistory();
    currentGeneration_ = 0;
    currentCreatureData_ = GeneFrequencyRecord();
    currentPlantData_ = GeneFrequencyRecord();
}

// ============================================================================
// Internal Methods
// ============================================================================

float CoevolutionTracker::calculateCorrelation(
    const std::vector<float>& creatureValues,
    const std::vector<float>& plantValues
) const {
    if (creatureValues.size() != plantValues.size() || creatureValues.size() < 2) {
        return 0.0f;
    }
    
    size_t n = creatureValues.size();
    
    // Calculate means
    float meanX = calculateMean(creatureValues);
    float meanY = calculateMean(plantValues);
    
    // Calculate covariance and standard deviations
    float covariance = 0.0f;
    float varX = 0.0f;
    float varY = 0.0f;
    
    for (size_t i = 0; i < n; ++i) {
        float dx = creatureValues[i] - meanX;
        float dy = plantValues[i] - meanY;
        covariance += dx * dy;
        varX += dx * dx;
        varY += dy * dy;
    }
    
    float denominator = std::sqrt(varX * varY);
    if (denominator < 0.0001f) {
        return 0.0f;
    }
    
    return covariance / denominator;
}

std::string CoevolutionTracker::detectTrend(
    const std::vector<float>& creatureValues,
    const std::vector<float>& plantValues
) const {
    if (creatureValues.size() < static_cast<size_t>(config_.minGenerationsForTrend)) {
        return "insufficient_data";
    }
    
    float creatureSlope = calculateSlope(creatureValues);
    float plantSlope = calculateSlope(plantValues);
    
    // Both increasing = escalating arms race
    if (creatureSlope > config_.escalationThreshold && plantSlope > config_.escalationThreshold) {
        return "escalating";
    }
    
    // Both decreasing = declining
    if (creatureSlope < -config_.escalationThreshold && plantSlope < -config_.escalationThreshold) {
        return "declining";
    }
    
    // Opposite directions = oscillating
    if ((creatureSlope > config_.escalationThreshold && plantSlope < -config_.escalationThreshold) ||
        (creatureSlope < -config_.escalationThreshold && plantSlope > config_.escalationThreshold)) {
        return "oscillating";
    }
    
    // Neither changing significantly = stable
    return "stable";
}

float CoevolutionTracker::calculateSlope(const std::vector<float>& values) const {
    if (values.size() < 2) {
        return 0.0f;
    }
    
    size_t n = values.size();
    
    // Linear regression: y = mx + b
    // m = (n*sum(xy) - sum(x)*sum(y)) / (n*sum(x^2) - (sum(x))^2)
    
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumXY = 0.0f;
    float sumX2 = 0.0f;
    
    for (size_t i = 0; i < n; ++i) {
        float x = static_cast<float>(i);
        float y = values[i];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    float denominator = static_cast<float>(n) * sumX2 - sumX * sumX;
    if (std::abs(denominator) < 0.0001f) {
        return 0.0f;
    }
    
    return (static_cast<float>(n) * sumXY - sumX * sumY) / denominator;
}

float CoevolutionTracker::calculateVariance(const std::vector<float>& values) const {
    if (values.size() < 2) {
        return 0.0f;
    }
    
    float mean = calculateMean(values);
    
    float sumSquaredDiff = 0.0f;
    for (float v : values) {
        float diff = v - mean;
        sumSquaredDiff += diff * diff;
    }
    
    return sumSquaredDiff / static_cast<float>(values.size() - 1);
}

float CoevolutionTracker::calculateMean(const std::vector<float>& values) const {
    if (values.empty()) {
        return 0.0f;
    }
    
    float sum = std::accumulate(values.begin(), values.end(), 0.0f);
    return sum / static_cast<float>(values.size());
}

} // namespace Genetics
} // namespace EcoSim
