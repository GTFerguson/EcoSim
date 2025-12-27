/**
 * @file CreatureTaxonomy.cpp
 * @brief Implementation of the CreatureTaxonomy stateless classification utility
 * 
 * Provides archetype classification and scientific naming based on raw genetic values.
 * Classification logic is ported from CreatureFactory::classifyCreature()
 * Naming logic is ported from CreatureFactory::generateScientificName()
 */

#include "genetics/classification/CreatureTaxonomy.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include <vector>
#include <algorithm>

namespace EcoSim {
namespace Genetics {

// ============================================================================
// Raw Gene Value Access
// ============================================================================

float CreatureTaxonomy::getRawGeneValue(const Genome& genome, const std::string& geneId, float fallback) {
    if (genome.hasGene(geneId)) {
        const Gene& gene = genome.getGene(geneId);
        return gene.getNumericValue(DominanceType::Incomplete);
    }
    return fallback;
}

// ============================================================================
// Archetype Classification
// ============================================================================

const ArchetypeIdentity* CreatureTaxonomy::classifyArchetype(const Genome& genome) {
    // Get RAW gene values - classification is based on genetic potential, not
    // current phenotype expression which is modulated by age/energy/health.
    // A newborn apex predator is still an apex predator genetically!
    float meatDig = getRawGeneValue(genome, UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float plantDig = getRawGeneValue(genome, UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float size = getRawGeneValue(genome, UniversalGenes::MAX_SIZE);
    float locomotion = getRawGeneValue(genome, UniversalGenes::LOCOMOTION);
    float aggression = getRawGeneValue(genome, UniversalGenes::COMBAT_AGGRESSION);
    float packCoord = getRawGeneValue(genome, UniversalGenes::PACK_COORDINATION);
    float scentMasking = getRawGeneValue(genome, UniversalGenes::SCENT_MASKING);
    float huntInstinct = getRawGeneValue(genome, UniversalGenes::HUNT_INSTINCT);
    float toxinTol = getRawGeneValue(genome, UniversalGenes::TOXIN_TOLERANCE);
    float hide = getRawGeneValue(genome, UniversalGenes::HIDE_THICKNESS);
    float scales = getRawGeneValue(genome, UniversalGenes::SCALE_COVERAGE);
    float hornLen = getRawGeneValue(genome, UniversalGenes::HORN_LENGTH);
    float bodySpines = getRawGeneValue(genome, UniversalGenes::BODY_SPINES);
    float tailSpines = getRawGeneValue(genome, UniversalGenes::TAIL_SPINES);
    float retreat = getRawGeneValue(genome, UniversalGenes::RETREAT_THRESHOLD);
    
    // Classification logic - check in priority order
    // (Same thresholds as CreatureFactory::classifyCreature)
    
    // Apex Predator: Large, aggressive carnivore
    if (meatDig > 0.8f && size > 2.0f && aggression > 0.7f) {
        return ArchetypeIdentity::ApexPredator();
    }
    
    // Pack Hunter: Coordinated carnivore, smaller
    if (meatDig > 0.7f && packCoord > 0.7f && size < 1.5f) {
        return ArchetypeIdentity::PackHunter();
    }
    
    // Ambush Predator: Stealthy carnivore
    if (meatDig > 0.8f && scentMasking > 0.6f && locomotion < 0.8f) {
        return ArchetypeIdentity::AmbushPredator();
    }
    
    // Pursuit Hunter: Fast carnivore
    if (meatDig > 0.7f && locomotion > 1.5f) {
        return ArchetypeIdentity::PursuitHunter();
    }
    
    // Scavenger: Carrion-eater (check before herbivores)
    if (meatDig > 0.5f && huntInstinct < 0.3f && toxinTol > 0.7f) {
        return ArchetypeIdentity::Scavenger();
    }
    
    // Tank Herbivore: Large, armored with horns
    if (plantDig > 0.7f && hide > 0.7f && size > 2.0f && hornLen > 0.6f) {
        return ArchetypeIdentity::TankHerbivore();
    }
    
    // Armored Grazer: Scaled herbivore
    if (plantDig > 0.7f && scales > 0.7f) {
        return ArchetypeIdentity::ArmoredGrazer();
    }
    
    // Spiky Defender: Spine defense
    if ((bodySpines > 0.5f || tailSpines > 0.5f) && plantDig > 0.4f) {
        return ArchetypeIdentity::SpikyDefender();
    }
    
    // Canopy Forager: Fruit specialist with excellent sensory abilities
    float sweetPref = getRawGeneValue(genome, UniversalGenes::SWEETNESS_PREFERENCE);
    float colorVision = getRawGeneValue(genome, UniversalGenes::COLOR_VISION);
    if (sweetPref > 0.6f && colorVision > 0.7f && meatDig < 0.3f) {
        return ArchetypeIdentity::CanopyForager();
    }
    
    // Fleet Runner: Fast herbivore
    if (plantDig > 0.5f && locomotion > 1.5f && retreat > 0.7f) {
        return ArchetypeIdentity::FleetRunner();
    }
    
    // Omnivore Generalist: Both diets
    if (plantDig > 0.4f && meatDig > 0.4f) {
        return ArchetypeIdentity::OmnivoreGeneralist();
    }
    
    // Default: Omnivore Generalist (catch-all)
    return ArchetypeIdentity::OmnivoreGeneralist();
}

// ============================================================================
// Scientific Name Generation
// ============================================================================

std::string CreatureTaxonomy::generateScientificName(const Genome& genome) {
    // Get RAW gene values - scientific name is based on genetic potential, not
    // current phenotype expression. A newborn or injured predator should have
    // the same scientific name as a healthy adult of the same species!
    float meatDig = getRawGeneValue(genome, UniversalGenes::MEAT_DIGESTION_EFFICIENCY);
    float plantDig = getRawGeneValue(genome, UniversalGenes::PLANT_DIGESTION_EFFICIENCY);
    float size = getRawGeneValue(genome, UniversalGenes::MAX_SIZE);
    float locomotion = getRawGeneValue(genome, UniversalGenes::LOCOMOTION);
    float aggression = getRawGeneValue(genome, UniversalGenes::COMBAT_AGGRESSION);
    float packCoord = getRawGeneValue(genome, UniversalGenes::PACK_COORDINATION);
    float scentMasking = getRawGeneValue(genome, UniversalGenes::SCENT_MASKING);
    float huntInstinct = getRawGeneValue(genome, UniversalGenes::HUNT_INSTINCT);
    float toxinTol = getRawGeneValue(genome, UniversalGenes::TOXIN_TOLERANCE);
    float hide = getRawGeneValue(genome, UniversalGenes::HIDE_THICKNESS);
    float scales = getRawGeneValue(genome, UniversalGenes::SCALE_COVERAGE);
    float fatLayer = getRawGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS);
    float teethSharp = getRawGeneValue(genome, UniversalGenes::TEETH_SHARPNESS);
    float clawSharp = getRawGeneValue(genome, UniversalGenes::CLAW_SHARPNESS);
    float hornLen = getRawGeneValue(genome, UniversalGenes::HORN_LENGTH);
    float tailMass = getRawGeneValue(genome, UniversalGenes::TAIL_MASS);
    float bodySpines = getRawGeneValue(genome, UniversalGenes::BODY_SPINES);
    float tailSpines = getRawGeneValue(genome, UniversalGenes::TAIL_SPINES);
    float regen = getRawGeneValue(genome, UniversalGenes::REGENERATION_RATE);
    float olfactory = getRawGeneValue(genome, UniversalGenes::OLFACTORY_ACUITY);
    float sightRange = getRawGeneValue(genome, UniversalGenes::SIGHT_RANGE);
    float retreat = getRawGeneValue(genome, UniversalGenes::RETREAT_THRESHOLD);
    
    // === GENUS (Diet + Behavior) ===
    std::string dietPrefix;
    std::string behaviorSuffix;
    
    // Diet prefix
    bool isCarnivore = meatDig > 0.7f && plantDig < 0.3f;
    bool isHerbivore = plantDig > 0.7f && meatDig < 0.3f;
    bool isOmnivore = plantDig > 0.4f && meatDig > 0.4f;
    bool isScavenger = meatDig > 0.5f && huntInstinct < 0.3f && toxinTol > 0.7f;
    
    if (isScavenger) {
        dietPrefix = "Necro";
    } else if (isCarnivore) {
        dietPrefix = "Carno";
    } else if (isHerbivore) {
        dietPrefix = "Herbo";
    } else if (isOmnivore) {
        dietPrefix = "Omni";
    } else {
        // Default based on higher value
        dietPrefix = (meatDig > plantDig) ? "Carno" : "Herbo";
    }
    
    // Behavior suffix (priority order)
    if (isScavenger) {
        behaviorSuffix = "scavus";
    } else if (aggression > 0.8f && size > 2.0f) {
        behaviorSuffix = "rex";
    } else if (aggression > 0.7f && meatDig > 0.7f) {
        behaviorSuffix = "predax";
    } else if (scentMasking > 0.6f && meatDig > 0.7f) {
        behaviorSuffix = "latens";
    } else if (packCoord > 0.7f) {
        behaviorSuffix = "socialis";
    } else if (locomotion > 1.5f) {
        behaviorSuffix = "cursor";
    } else if (scales > 0.7f || hide > 0.7f) {
        behaviorSuffix = "scutum";
    } else if (scentMasking > 0.6f) {
        behaviorSuffix = "insidia";
    } else if (plantDig > 0.7f && aggression < 0.3f) {
        behaviorSuffix = "grazer";
    } else {
        behaviorSuffix = "flexus";
    }
    
    std::string genus = dietPrefix + behaviorSuffix;
    
    // === SPECIES (Size) ===
    std::string species;
    if (size < 0.7f) {
        species = "minimus";
    } else if (size < 1.0f) {
        species = "minor";
    } else if (size < 1.5f) {
        species = "mediocris";
    } else if (size < 2.0f) {
        species = "major";
    } else if (size < 2.5f) {
        species = "grandis";
    } else {
        species = "titan";
    }
    
    // === EPITHET (Primary Trait) ===
    // Find most distinctive trait
    struct TraitScore {
        float score;
        std::string epithet;
    };
    
    std::vector<TraitScore> traits = {
        // Offense
        {teethSharp, "dentatus"},
        {clawSharp, "unguiculatus"},
        {hornLen, "cornuatus"},
        {tailMass, "caudatus"},
        
        // Defense
        {hide, "fortis"},
        {scales, "squamatus"},
        {fatLayer, "crassus"},
        {(bodySpines > tailSpines ? bodySpines : tailSpines), "spinosus"},
        
        // Special
        {regen / 2.0f, "regenerans"}, // Normalize (max is 2.0)
        {olfactory, "olfactans"},
        {sightRange / 150.0f, "vigilans"}, // Normalize (max is ~150)
        {locomotion / 2.0f, "velocis"} // Normalize (max is 2.0)
    };
    
    // Combined armor check
    if (hide > 0.5f && scales > 0.5f) {
        traits.push_back({(hide + scales) / 2.0f + 0.1f, "armatus"});
    }
    
    // Defensive posture
    if (retreat > 0.8f) {
        traits.push_back({retreat, "timidus"});
    }
    
    // Find highest scoring trait above threshold
    std::string epithet = "vulgaris"; // Default
    float bestScore = 0.5f; // Threshold
    
    for (const auto& t : traits) {
        if (t.score > bestScore) {
            bestScore = t.score;
            epithet = t.epithet;
        }
    }
    
    return genus + " " + species + " " + epithet;
}

} // namespace Genetics
} // namespace EcoSim
