/**
 * @file CreatureTaxonomy.cpp
 * @brief Implementation of the CreatureTaxonomy stateless classification utility
 *
 * Provides archetype classification, biome adaptation classification, and
 * scientific naming based on raw genetic values.
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
    namespace CT = ClassificationThresholds;
    
    // Apex Predator: Large, aggressive carnivore
    if (meatDig > CT::CARNIVORE_HIGH && size > CT::SIZE_LARGE && aggression > CT::AGGRESSION_HIGH) {
        return ArchetypeIdentity::ApexPredator();
    }
    
    // Pack Hunter: Coordinated carnivore, smaller
    if (meatDig > CT::CARNIVORE_MIN && packCoord > CT::PACK_COORD_MIN && size < CT::SIZE_SMALL_MAX) {
        return ArchetypeIdentity::PackHunter();
    }
    
    // Ambush Predator: Stealthy carnivore
    if (meatDig > CT::CARNIVORE_HIGH && scentMasking > CT::SCENT_MASKING_MIN && locomotion < CT::LOCOMOTION_SLOW_MAX) {
        return ArchetypeIdentity::AmbushPredator();
    }
    
    // Pursuit Hunter: Fast carnivore
    if (meatDig > CT::CARNIVORE_MIN && locomotion > CT::LOCOMOTION_FAST) {
        return ArchetypeIdentity::PursuitHunter();
    }
    
    // Scavenger: Carrion-eater (check before herbivores)
    if (meatDig > CT::SCAVENGER_MEAT_MIN && huntInstinct < CT::HUNT_INSTINCT_LOW_MAX && toxinTol > CT::TOXIN_TOLERANCE_MIN) {
        return ArchetypeIdentity::Scavenger();
    }
    
    // Tank Herbivore: Large, armored with horns
    if (plantDig > CT::HERBIVORE_MIN && hide > CT::HIDE_MIN && size > CT::SIZE_LARGE && hornLen > CT::HORN_LENGTH_MIN) {
        return ArchetypeIdentity::TankHerbivore();
    }
    
    // Armored Grazer: Scaled herbivore
    if (plantDig > CT::HERBIVORE_MIN && scales > CT::SCALES_MIN) {
        return ArchetypeIdentity::ArmoredGrazer();
    }
    
    // Spiky Defender: Spine defense
    if ((bodySpines > CT::SPINES_MIN || tailSpines > CT::SPINES_MIN) && plantDig > CT::SPIKY_PLANT_MIN) {
        return ArchetypeIdentity::SpikyDefender();
    }
    
    // Canopy Forager: Fruit specialist with excellent sensory abilities
    float sweetPref = getRawGeneValue(genome, UniversalGenes::SWEETNESS_PREFERENCE);
    float colorVision = getRawGeneValue(genome, UniversalGenes::COLOR_VISION);
    if (sweetPref > CT::SWEET_PREFERENCE_MIN && colorVision > CT::COLOR_VISION_MIN && meatDig < CT::LOW_DIET_MAX) {
        return ArchetypeIdentity::CanopyForager();
    }
    
    // Fleet Runner: Fast herbivore
    if (plantDig > CT::FLEET_PLANT_MIN && locomotion > CT::LOCOMOTION_FAST && retreat > CT::RETREAT_HIGH) {
        return ArchetypeIdentity::FleetRunner();
    }
    
    // Omnivore Generalist: Both diets
    if (plantDig > CT::OMNIVORE_MIN && meatDig > CT::OMNIVORE_MIN) {
        return ArchetypeIdentity::OmnivoreGeneralist();
    }
    
    // Default: Omnivore Generalist (catch-all)
    return ArchetypeIdentity::OmnivoreGeneralist();
}

// ============================================================================
// Biome Adaptation Classification
// ============================================================================

const BiomeAdaptation* CreatureTaxonomy::classifyBiomeAdaptation(const Genome& genome) {
    // Get RAW thermal adaptation gene values
    float furDensity = getRawGeneValue(genome, UniversalGenes::FUR_DENSITY, 0.5f);
    float fatLayer = getRawGeneValue(genome, UniversalGenes::FAT_LAYER_THICKNESS, 0.3f);
    
    // Try to get temperature tolerance genes (may not exist in all genomes)
    // Default to moderate values if genes don't exist
    float tempTolLow = getRawGeneValue(genome, "temp_tolerance_low", 0.5f);
    float tempTolHigh = getRawGeneValue(genome, "temp_tolerance_high", 0.5f);
    
    namespace CT = ClassificationThresholds;
    
    // Tundra: High insulation (fur + fat) = cold adaptation
    // Real-world analogs: Arctic Wolf, Woolly Mammoth, Polar Bear
    if (furDensity > CT::TUNDRA_FUR_MIN && fatLayer > CT::TUNDRA_FAT_MIN) {
        return BiomeAdaptation::Tundra();
    }
    
    // Desert: Low fur + high heat tolerance = arid adaptation
    // Real-world analogs: Fennec Fox, Camel, Desert Tortoise
    if (furDensity < CT::DESERT_FUR_MAX && tempTolHigh > CT::DESERT_HEAT_TOL_MIN) {
        return BiomeAdaptation::Desert();
    }
    
    // Tropical: Low fur + moderate cold tolerance = humid heat adaptation
    // Real-world analogs: Jaguar, Elephant, Tapir
    if (furDensity < CT::TROPICAL_FUR_MAX && tempTolLow > CT::TROPICAL_TEMP_LOW_MIN) {
        return BiomeAdaptation::Tropical();
    }
    
    // Taiga: Moderate-high fur but not extreme = cold forest adaptation
    if (furDensity > 0.5f && furDensity <= CT::TUNDRA_FUR_MIN) {
        return BiomeAdaptation::Taiga();
    }
    
    // Default: Temperate (baseline, no special adaptations)
    return BiomeAdaptation::Temperate();
}

// ============================================================================
// Scientific Name Generation
// ============================================================================

std::string CreatureTaxonomy::generateScientificName(const Genome& genome,
                                                      const BiomeAdaptation* biomeAdaptation) {
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
    namespace CT = ClassificationThresholds;
    std::string dietPrefix;
    std::string behaviorSuffix;
    
    // Diet prefix
    bool isCarnivore = meatDig > CT::CARNIVORE_MIN && plantDig < CT::LOW_DIET_MAX;
    bool isHerbivore = plantDig > CT::HERBIVORE_MIN && meatDig < CT::LOW_DIET_MAX;
    bool isOmnivore = plantDig > CT::OMNIVORE_MIN && meatDig > CT::OMNIVORE_MIN;
    bool isScavenger = meatDig > CT::SCAVENGER_MEAT_MIN && huntInstinct < CT::HUNT_INSTINCT_LOW_MAX && toxinTol > CT::TOXIN_TOLERANCE_MIN;
    
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
    } else if (aggression > CT::AGGRESSION_VERY_HIGH && size > CT::SIZE_LARGE) {
        behaviorSuffix = "rex";
    } else if (aggression > CT::AGGRESSION_HIGH && meatDig > CT::CARNIVORE_MIN) {
        behaviorSuffix = "predax";
    } else if (scentMasking > CT::SCENT_MASKING_MIN && meatDig > CT::CARNIVORE_MIN) {
        behaviorSuffix = "latens";
    } else if (packCoord > CT::PACK_COORD_MIN) {
        behaviorSuffix = "socialis";
    } else if (locomotion > CT::LOCOMOTION_FAST) {
        behaviorSuffix = "cursor";
    } else if (scales > CT::SCALES_MIN || hide > CT::HIDE_MIN) {
        behaviorSuffix = "scutum";
    } else if (scentMasking > CT::SCENT_MASKING_MIN) {
        behaviorSuffix = "insidia";
    } else if (plantDig > CT::HERBIVORE_MIN && aggression < CT::AGGRESSION_LOW_MAX) {
        behaviorSuffix = "grazer";
    } else {
        behaviorSuffix = "flexus";
    }
    
    std::string genus = dietPrefix + behaviorSuffix;
    
    // === SPECIES (Size) ===
    std::string species;
    if (size < CT::SIZE_MINIMUS_MAX) {
        species = "minimus";
    } else if (size < CT::SIZE_MINOR_MAX) {
        species = "minor";
    } else if (size < CT::SIZE_MEDIOCRIS_MAX) {
        species = "mediocris";
    } else if (size < CT::SIZE_MAJOR_MAX) {
        species = "major";
    } else if (size < CT::SIZE_GRANDIS_MAX) {
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
        
        // Special - normalized using threshold constants
        {regen / CT::REGEN_MAX, "regenerans"},
        {olfactory, "olfactans"},
        {sightRange / CT::SIGHT_RANGE_MAX, "vigilans"},
        {locomotion / CT::LOCOMOTION_MAX, "velocis"}
    };
    
    // Combined armor check
    if (hide > CT::COMBINED_ARMOR_MIN && scales > CT::COMBINED_ARMOR_MIN) {
        traits.push_back({(hide + scales) / 2.0f + CT::COMBINED_ARMOR_BONUS, "armatus"});
    }
    
    // Defensive posture
    if (retreat > CT::RETREAT_VERY_HIGH) {
        traits.push_back({retreat, "timidus"});
    }
    
    // Find highest scoring trait above threshold
    std::string epithet = "vulgaris"; // Default
    float bestScore = CT::TRAIT_THRESHOLD;
    
    for (const auto& t : traits) {
        if (t.score > bestScore) {
            bestScore = t.score;
            epithet = t.epithet;
        }
    }
    
    // === BIOME MODIFIER (for non-temperate variants) ===
    // If no biome adaptation provided, auto-classify from genome
    const BiomeAdaptation* biome = biomeAdaptation;
    if (!biome) {
        biome = classifyBiomeAdaptation(genome);
    }
    
    // Build the trinomial/quadrinomial name
    std::string baseName = genus + " " + species + " " + epithet;
    
    // Add biome subspecies modifier for non-temperate creatures
    // Examples: "borealis" (tundra/taiga), "tropicus" (tropical), "deserti" (desert)
    if (biome && !biome->isBaseline() && !biome->getLatinModifier().empty()) {
        return baseName + " " + biome->getLatinModifier();
    }
    
    return baseName;
}

} // namespace Genetics
} // namespace EcoSim
