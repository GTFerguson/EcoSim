/**
 * @file CreatureScent.cpp
 * @brief Scent system functionality extracted from Creature class
 *
 * This module handles scent signature generation, scent deposits, and scent detection.
 */

#include "objects/creature/CreatureScent.hpp"
#include "objects/creature/creature.hpp"
#include "world/ScentLayer.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <cmath>
#include <algorithm>
#include <optional>

namespace CreatureScent {

//============================================================================
//  Scent Signature Generation
//============================================================================

std::array<float, 8> computeScentSignature(const Creature& creature) {
    std::array<float, 8> signature;
    
    // Use genome traits to create a unique fingerprint
    // This allows related creatures to have similar signatures
    
    // Component 0: Diet-based (0.0-0.2 range based on diet type)
    signature[0] = static_cast<float>(static_cast<int>(creature.getDietType())) * 0.2f;
    
    // Component 1: Lifespan-based (normalized)
    signature[1] = std::min(1.0f, static_cast<float>(creature.getLifespan()) / 1000000.0f);
    
    // Component 2: Sight range (normalized)
    signature[2] = std::min(1.0f, static_cast<float>(creature.getSightRange()) / 200.0f);
    
    // Component 3: Metabolism-based (normalized)
    signature[3] = std::min(1.0f, creature.getMetabolism() * 100.0f);
    
    // Component 4: Social behavior (flee/pursue ratio)
    float fleeVal = static_cast<float>(creature.getFlee());
    float pursueVal = static_cast<float>(creature.getPursue());
    signature[4] = (pursueVal > 0) ? std::min(1.0f, fleeVal / pursueVal) : 0.5f;
    
    // Component 5: Flocking tendency
    signature[5] = creature.ifFlocks() ? 0.8f : 0.2f;
    
    // Components 6-7: Use olfactory genes if available
    const auto& phenotype = creature.getPhenotype();
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_SIGNATURE_VARIANCE)) {
        signature[6] = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_SIGNATURE_VARIANCE);
    } else {
        // Derive from other traits for legacy creatures
        signature[6] = std::min(1.0f, creature.getTHunger() + creature.getTThirst());
    }
    
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION)) {
        signature[7] = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION);
    } else {
        signature[7] = std::min(1.0f, creature.getTFatigue() + creature.getTMate());
    }
    
    return signature;
}

float calculateSignatureSimilarity(
    const std::array<float, 8>& sig1,
    const std::array<float, 8>& sig2) {
    // Calculate cosine similarity
    float dotProduct = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    
    for (size_t i = 0; i < 8; ++i) {
        dotProduct += sig1[i] * sig2[i];
        norm1 += sig1[i] * sig1[i];
        norm2 += sig2[i] * sig2[i];
    }
    
    if (norm1 == 0.0f || norm2 == 0.0f) {
        return 0.0f;
    }
    
    return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
}

//============================================================================
//  Scent Deposits
//============================================================================

void depositBreedingScent(Creature& creature, EcoSim::ScentLayer& layer, unsigned int currentTick) {
    // Only deposit if in breeding profile
    if (creature.getProfile() != Profile::breed) {
        return;
    }
    
    // Get scent production rate from phenotype
    float scentProduction = DEFAULT_SCENT_PRODUCTION;
    float scentMasking = 0.0f;     // Default - no masking
    
    const auto& phenotype = creature.getPhenotype();
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION)) {
        scentProduction = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_PRODUCTION);
    }
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_MASKING)) {
        scentMasking = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_MASKING);
    }
    
    // Apply scent masking - high masking reduces effective production
    float effectiveProduction = scentProduction * (1.0f - scentMasking * 0.8f);
    
    // Only deposit if effective production is significant
    if (effectiveProduction < 0.05f) {
        return;
    }
    
    // Create scent deposit
    EcoSim::ScentDeposit deposit;
    deposit.type = EcoSim::ScentType::MATE_SEEKING;
    deposit.creatureId = creature.getId();
    deposit.intensity = effectiveProduction;
    deposit.signature = computeScentSignature(creature);
    deposit.tickDeposited = currentTick;
    
    // Decay rate based on intensity - stronger scents last longer
    // Base decay: 50-200 ticks depending on intensity
    deposit.decayRate = static_cast<unsigned int>(50 + effectiveProduction * 150);
    
    // Deposit at creature's current position
    layer.deposit(creature.tileX(), creature.tileY(), deposit);
}

//============================================================================
//  Scent Detection
//============================================================================

bool hasScentDetection(const Creature& creature) {
    const auto& phenotype = creature.getPhenotype();
    
    // Check if scent_detection trait exists, default to 0.0 if not
    float scentDetection = 0.0f;
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION)) {
        scentDetection = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION);
    }
    return scentDetection > 0.1f;  // Threshold for meaningful scent ability
}

int getDetectionRange(const Creature& creature) {
    const auto& phenotype = creature.getPhenotype();
    
    // Get olfactory acuity from phenotype (default to moderate if not available)
    float olfactoryAcuity = DEFAULT_OLFACTORY_ACUITY;
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY)) {
        olfactoryAcuity = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::OLFACTORY_ACUITY);
    }
    
    // Calculate detection range: base + acuity * multiplier
    return static_cast<int>(SCENT_DETECTION_BASE_RANGE + olfactoryAcuity * SCENT_DETECTION_ACUITY_MULT);
}

std::optional<Direction> detectMateDirection(const Creature& creature, const EcoSim::ScentLayer& scentLayer) {
    // Only detect if we're in breeding mode
    if (creature.getProfile() != Profile::breed) {
        return std::nullopt;
    }
    
    int detectionRange = getDetectionRange(creature);
    
    // Query the scent layer for strongest MATE_SEEKING scent in range
    int scentX = creature.tileX(), scentY = creature.tileY();
    EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
        creature.tileX(), creature.tileY(), detectionRange,
        EcoSim::ScentType::MATE_SEEKING,
        scentX, scentY
    );
    
    // Check if we found a valid scent
    if (strongestScent.intensity <= 0.0f || strongestScent.creatureId == creature.getId()) {
        // No scent found, or it's our own scent
        return std::nullopt;
    }
    
    // Check if scent is at our current position (we've arrived!)
    if (scentX == creature.tileX() && scentY == creature.tileY()) {
        return Direction::none;  // At the scent source
    }
    
    // Calculate direction toward scent source
    int dx = scentX - creature.tileX();
    int dy = scentY - creature.tileY();
    
    // Convert delta to one of 8 directions
    Direction result = Direction::none;
    
    if (dx > 0 && dy > 0) {
        result = Direction::SE;
    } else if (dx > 0 && dy < 0) {
        result = Direction::NE;
    } else if (dx > 0 && dy == 0) {
        result = Direction::E;
    } else if (dx < 0 && dy > 0) {
        result = Direction::SW;
    } else if (dx < 0 && dy < 0) {
        result = Direction::NW;
    } else if (dx < 0 && dy == 0) {
        result = Direction::W;
    } else if (dx == 0 && dy > 0) {
        result = Direction::S;
    } else if (dx == 0 && dy < 0) {
        result = Direction::N;
    }
    
    return result;
}

bool findMateScent(const Creature& creature, const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) {
    // Only detect if we're in breeding mode
    if (creature.getProfile() != Profile::breed) {
        return false;
    }
    
    int detectionRange = getDetectionRange(creature);
    
    // Query the scent layer for strongest MATE_SEEKING scent in range
    int scentX = creature.tileX(), scentY = creature.tileY();
    EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
        creature.tileX(), creature.tileY(), detectionRange,
        EcoSim::ScentType::MATE_SEEKING,
        scentX, scentY
    );
    
    // Check if we found a valid scent (not our own)
    if (strongestScent.intensity <= 0.0f || strongestScent.creatureId == creature.getId()) {
        return false;
    }
    
    // Check if scent is at our current position (shouldn't navigate to self)
    if (scentX == creature.tileX() && scentY == creature.tileY()) {
        return false;
    }
    
    // Return the scent coordinates for A* pathfinding
    outX = scentX;
    outY = scentY;
    return true;
}

bool findFoodScent(const Creature& creature, const EcoSim::ScentLayer& scentLayer, int& outX, int& outY) {
    const auto& phenotype = creature.getPhenotype();
    
    // Get scent detection range from phenotype
    float scentDetection = DEFAULT_OLFACTORY_ACUITY;
    if (phenotype.hasTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION)) {
        scentDetection = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::SCENT_DETECTION);
    }
    
    // Calculate detection range: base + scent_detection * multiplier
    int detectionRange = static_cast<int>(SCENT_DETECTION_BASE_RANGE + scentDetection * SCENT_DETECTION_ACUITY_MULT);
    
    // Query the scent layer for strongest FOOD_TRAIL scent in range
    int scentX = creature.tileX(), scentY = creature.tileY();
    EcoSim::ScentDeposit strongestScent = scentLayer.getStrongestScentInRadius(
        creature.tileX(), creature.tileY(), detectionRange,
        EcoSim::ScentType::FOOD_TRAIL,
        scentX, scentY
    );
    
    // Check if we found a valid scent with meaningful intensity
    if (strongestScent.intensity <= 0.0f) {
        return false;
    }
    
    // Check if scent is at our current position (shouldn't navigate to self)
    if (scentX == creature.tileX() && scentY == creature.tileY()) {
        return false;
    }
    
    // Return the scent coordinates for A* pathfinding
    outX = scentX;
    outY = scentY;
    return true;
}

} // namespace CreatureScent
