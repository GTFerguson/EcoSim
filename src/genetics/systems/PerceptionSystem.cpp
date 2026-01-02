/**
 * @file PerceptionSystem.cpp
 * @brief Implementation of organism-agnostic perception system
 *
 * All methods use Organism& interface and phenotype traits.
 * NO type-specific code (Plant, Creature, DietType, etc.)
 *
 * @see include/genetics/systems/PerceptionSystem.hpp
 */

#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/PhenotypeUtils.hpp"
#include "world/ScentLayer.hpp"

#include <cmath>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

using PhenotypeUtils::getTraitSafe;

// =============================================================================
// Scent Signature Generation
// =============================================================================

std::array<float, 8> PerceptionSystem::buildScentSignature(
    const Organism& organism) const
{
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Build signature from phenotype traits only - no type-specific code
    // Signature indices are consistent for ALL organism types:
    // [0] nutrition     - nutrient_value / 100 (normalized)
    // [1] fruit_appeal  - how appealing/fragrant
    // [2] toxicity      - warning scent
    // [3] hardiness     - toughness indicator
    // [4] scent_production - how much scent emitted
    // [5] color_hue     - color information
    // [6] size_gene     - size indicator
    // [7] reserved      - for future use
    
    std::array<float, 8> signature;
    
    // Normalize nutrient_value (typically 0-100) to 0-1 range
    float nutrientValue = getTraitSafe(phenotype, "nutrient_value", 0.0f);
    signature[0] = clamp01(nutrientValue / 100.0f);
    
    // Other traits are already 0-1 normalized
    signature[1] = clamp01(getTraitSafe(phenotype, "fruit_appeal", 0.0f));
    signature[2] = clamp01(getTraitSafe(phenotype, "toxicity", 0.0f));
    signature[3] = clamp01(getTraitSafe(phenotype, "hardiness", 0.0f));
    signature[4] = clamp01(getTraitSafe(phenotype, "scent_production", 0.0f));
    signature[5] = clamp01(getTraitSafe(phenotype, "color_hue", 0.0f));
    signature[6] = clamp01(getTraitSafe(phenotype, "size_gene", 0.0f));
    signature[7] = 0.0f;  // Reserved
    
    return signature;
}

// =============================================================================
// Scent Emission
// =============================================================================

void PerceptionSystem::depositScent(
    const Organism& organism,
    int posX, int posY,
    int sourceId,
    ScentLayer& scentLayer,
    ScentType type,
    unsigned int currentTick) const
{
    const Phenotype& phenotype = organism.getPhenotype();
    
    // Get scent production from phenotype
    float scentProduction = getTraitSafe(phenotype, "scent_production", 0.0f);
    
    // Below threshold = no scent emitted (allows "scentless" organisms)
    if (scentProduction < MIN_SCENT_PRODUCTION) {
        return;
    }
    
    // Build signature from phenotype traits
    auto signature = buildScentSignature(organism);
    
    // Calculate intensity based on scent_production and fruit_appeal
    // Fragrant, appealing organisms emit stronger scent
    float fruitAppeal = getTraitSafe(phenotype, "fruit_appeal", 0.0f);
    float intensity = scentProduction * (0.5f + 0.5f * fruitAppeal);
    intensity = clamp01(intensity);
    
    // Create and deposit the scent
    ScentDeposit deposit(
        type,
        sourceId,
        intensity,
        signature,
        currentTick,
        DEFAULT_DECAY_RATE
    );
    
    scentLayer.deposit(posX, posY, deposit);
}

// =============================================================================
// Scent Detection: Food
// =============================================================================

std::optional<std::pair<int, int>> PerceptionSystem::detectFoodDirection(
    const Organism& seeker,
    int seekerX, int seekerY,
    const ScentLayer& scentLayer) const
{
    // Calculate scent detection range from phenotype
    int scentRange = static_cast<int>(calculateScentRange(seeker));
    
    if (scentRange <= 0) {
        return std::nullopt;  // No scent detection ability
    }
    
    // Search for FOOD_TRAIL scents in range
    int bestX = 0, bestY = 0;
    float bestScore = 0.0f;
    bool foundFood = false;
    
    // Get all food scents in radius
    auto scentsInRadius = scentLayer.getScentsInRadius(
        seekerX, seekerY, scentRange, ScentType::FOOD_TRAIL);
    
    for (const auto& [scent, x, y] : scentsInRadius) {
        // Skip if not edible for this seeker
        if (!isEdibleScent(scent.signature, seeker)) {
            continue;
        }
        
        // Calculate score based on intensity and distance
        float distance = calculateDistance(
            static_cast<float>(seekerX), static_cast<float>(seekerY),
            static_cast<float>(x), static_cast<float>(y));
        
        // Closer and stronger scents are better
        // Use raw intensity since we don't have current tick here
        float score = scent.intensity / (1.0f + distance * 0.1f);
        
        if (score > bestScore) {
            bestScore = score;
            bestX = x;
            bestY = y;
            foundFood = true;
        }
    }
    
    if (foundFood) {
        return std::make_pair(bestX, bestY);
    }
    
    return std::nullopt;
}

// =============================================================================
// Scent Detection: Mate
// =============================================================================

std::optional<std::pair<int, int>> PerceptionSystem::detectMateDirection(
    const Organism& seeker,
    int seekerX, int seekerY,
    int seekerId,
    const ScentLayer& scentLayer) const
{
    // Calculate scent detection range from phenotype
    int scentRange = static_cast<int>(calculateScentRange(seeker));
    
    if (scentRange <= 0) {
        return std::nullopt;
    }
    
    // Search for MATE_SEEKING scents
    int outX = 0, outY = 0;
    ScentDeposit strongest = scentLayer.getStrongestScentInRadius(
        seekerX, seekerY, scentRange,
        ScentType::MATE_SEEKING,
        outX, outY);
    
    // Check if valid scent found (not own scent)
    if (strongest.intensity > 0.0f && strongest.creatureId != seekerId) {
        return std::make_pair(outX, outY);
    }
    
    return std::nullopt;
}

// =============================================================================
// Edibility Checking
// =============================================================================

bool PerceptionSystem::isEdibleScent(
    const std::array<float, 8>& signature,
    const Organism& eater) const
{
    const Phenotype& phenotype = eater.getPhenotype();
    
    // Extract values from signature
    float nutrition = signature[0];      // [0] = nutrient_value / 100
    float toxicity = signature[2];       // [2] = toxicity
    float hardiness = signature[3];      // [3] = hardiness
    
    // Get eater's tolerance traits from phenotype
    float toxinResistance = getTraitSafe(phenotype, "toxin_resistance", 0.0f);
    float eaterHardiness = getTraitSafe(phenotype, "hardiness", 0.0f);
    
    // Get digestion capabilities (organism-agnostic diet checking)
    // plant_digestion > 0.1 means can eat plants
    // meat_digestion > 0.1 means can eat meat (for future creature scents)
    float plantDigestion = getTraitSafe(phenotype, "plant_digestion", 0.0f);
    
    // Check 1: Can they digest plant matter at all?
    // (FOOD_TRAIL scents are primarily from plants currently)
    if (plantDigestion < 0.1f) {
        return false;
    }
    
    // Check 2: Is it too toxic for them?
    // Toxicity must not exceed resistance by too much
    if (toxicity > toxinResistance * 1.5f) {
        return false;
    }
    
    // Check 3: Is it too tough to eat?
    // Target hardiness shouldn't vastly exceed eater's ability
    if (hardiness > eaterHardiness + 0.5f) {
        return false;
    }
    
    // Check 4: Is there actually nutrition?
    if (nutrition < 0.05f) {
        return false;  // Not nutritious enough to bother
    }
    
    return true;
}

// =============================================================================
// Detection Range Calculations
// =============================================================================

float PerceptionSystem::calculateVisualRange(
    const Organism& seeker,
    float targetColorfulness) const
{
    const Phenotype& phenotype = seeker.getPhenotype();
    
    // Get sight range from phenotype (base visual range)
    float sightRange = getTraitSafe(phenotype, "sight_range", 0.0f);
    
    // Get color vision ability (0-1)
    float colorVision = getTraitSafe(phenotype, "color_vision", 0.0f);
    
    // Visual bonus: high color vision helps spot colorful targets
    // Formula: sightRange + (colorVision * targetColorfulness * 100)
    float visualBonus = colorVision * clamp01(targetColorfulness) * COLOR_VISION_RANGE_MULTIPLIER;
    
    return sightRange + visualBonus;
}

float PerceptionSystem::calculateScentRange(const Organism& seeker) const
{
    const Phenotype& phenotype = seeker.getPhenotype();
    
    // Scent detection gene determines range (0-1 scaled to 0-100 tiles)
    float scentDetection = getTraitSafe(phenotype, "scent_detection", 0.0f);
    
    return scentDetection * SCENT_RANGE_MULTIPLIER;
}

float PerceptionSystem::calculateEffectiveRange(
    const Organism& seeker,
    float targetColorfulness) const
{
    // Effective range is max of visual and scent
    // Detection occurs if target is within EITHER range
    float visualRange = calculateVisualRange(seeker, targetColorfulness);
    float scentRange = calculateScentRange(seeker);
    
    return std::max(visualRange, scentRange);
}

// =============================================================================
// Utility Methods
// =============================================================================

float PerceptionSystem::calculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

bool PerceptionSystem::isWithinRange(
    float seekerX, float seekerY,
    float targetX, float targetY,
    float range)
{
    float distance = calculateDistance(seekerX, seekerY, targetX, targetY);
    return distance <= range;
}

float PerceptionSystem::calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2) const
{
    // Calculate similarity as 1 - normalized distance
    // Uses first 7 elements (index 7 is reserved)
    float sumSquaredDiff = 0.0f;
    
    for (size_t i = 0; i < 7; ++i) {
        float diff = sig1[i] - sig2[i];
        sumSquaredDiff += diff * diff;
    }
    
    // Normalize: max possible distance is sqrt(7) ≈ 2.65
    float distance = std::sqrt(sumSquaredDiff);
    float maxDistance = std::sqrt(7.0f);
    
    // Convert to similarity (1 = identical, 0 = completely different)
    float similarity = 1.0f - (distance / maxDistance);
    
    return clamp01(similarity);
}

} // namespace Genetics
} // namespace EcoSim
