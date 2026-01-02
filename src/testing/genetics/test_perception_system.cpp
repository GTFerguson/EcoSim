/**
 * @file test_perception_system.cpp
 * @brief Comprehensive tests for the organism-agnostic PerceptionSystem
 *
 * Tests scent signature generation, scent deposition, edibility checking,
 * range calculations, and direction detection for both creatures and plants.
 *
 * The PerceptionSystem works through Organism interface using phenotype
 * traits only - no type-specific code.
 */

#include "test_framework.hpp"
#include "genetics/systems/PerceptionSystem.hpp"
#include "genetics/organisms/Organism.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "world/ScentLayer.hpp"

#include <iostream>
#include <memory>
#include <cmath>
#include <array>
#include <unordered_map>
#include <algorithm>

using namespace EcoSim;
using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//================================================================================
//  MockOrganism: Test Implementation of Organism
//================================================================================

/**
 * @brief Mock organism for testing PerceptionSystem
 *
 * Implements Organism interface with configurable traits.
 * Traits can be set directly for precise test control.
 */
class MockOrganism : public Organism {
public:
    MockOrganism()
        : Organism(0, 0, Genome(), registry_)
    {
        initializeRegistry();
        setOptimalState();
    }
    
    ~MockOrganism() noexcept override = default;

    // IPositionable - world coordinates
    float getWorldX() const override { return static_cast<float>(getX()); }
    float getWorldY() const override { return static_cast<float>(getY()); }
    void setWorldPosition(float, float) override {}
    
    // ILifecycle
    unsigned int getMaxLifespan() const override { return 10000; }
    void grow() override {}
    
    // IReproducible
    bool canReproduce() const override { return false; }
    float getReproductiveUrge() const override { return 0.0f; }
    float getReproductionEnergyCost() const override { return 10.0f; }
    ReproductionMode getReproductionMode() const override { return ReproductionMode::SEXUAL; }
    bool isCompatibleWith(const Organism&) const override { return false; }
    std::unique_ptr<Organism> reproduce(const Organism* = nullptr) override { return nullptr; }
    
    // Organism abstract methods
    float getMaxSize() const override { return 1.0f; }

    /**
     * @brief Set a trait directly on the genome for testing
     */
    void setTrait(const std::string& name, float value) {
        ChromosomeType chromType = ChromosomeType::Metabolism;
        
        if (name == "sight_range" || name == "color_vision" || name == "scent_detection" || name == "scent_production") {
            chromType = ChromosomeType::Sensory;
        } else if (name == "hardiness" || name == "size_gene" || name == "color_hue") {
            chromType = ChromosomeType::Morphology;
        } else if (name == "toxicity" || name == "toxin_resistance") {
            chromType = ChromosomeType::Environmental;
        }
        
        Genome& genome = getGenomeMutable();
        if (genome.hasGene(name)) {
            Gene& existingGene = genome.getGeneMutable(name);
            existingGene.setAlleleValues(value);
        } else {
            GeneValue geneVal = value;
            Gene gene(name, geneVal);
            genome.addGene(gene, chromType);
        }
        
        if (!registry_.hasGene(name)) {
            float maxVal = (name == "nutrient_value" || name == "sight_range") ? 100.0f : 1.0f;
            GeneLimits limits(0.0f, maxVal, 0.05f);
            GeneDefinition def(name, chromType, limits, DominanceType::Incomplete);
            registry_.registerGene(def);
        }
        
        Organism::updatePhenotype();
    }

    void configureAsPlant() {
        setTrait("nutrient_value", 50.0f);
        setTrait("fruit_appeal", 0.7f);
        setTrait("toxicity", 0.1f);
        setTrait("hardiness", 0.3f);
        setTrait("scent_production", 0.6f);
        setTrait("color_hue", 0.3f);
        setTrait("size_gene", 0.0f);
        setTrait("plant_digestion", 0.0f);
        setTrait("meat_digestion", 0.0f);
    }

    void configureAsHerbivore() {
        setTrait("nutrient_value", 30.0f);
        setTrait("fruit_appeal", 0.0f);
        setTrait("toxicity", 0.0f);
        setTrait("hardiness", 0.5f);
        setTrait("scent_production", 0.4f);
        setTrait("color_hue", 0.5f);
        setTrait("size_gene", 0.5f);
        setTrait("plant_digestion", 0.8f);
        setTrait("meat_digestion", 0.1f);
        setTrait("toxin_resistance", 0.3f);
        setTrait("sight_range", 50.0f);
        setTrait("color_vision", 0.5f);
        setTrait("scent_detection", 0.5f);
    }

    void configureAsCarnivore() {
        setTrait("nutrient_value", 60.0f);
        setTrait("fruit_appeal", 0.0f);
        setTrait("toxicity", 0.0f);
        setTrait("hardiness", 0.6f);
        setTrait("scent_production", 0.3f);
        setTrait("color_hue", 0.4f);
        setTrait("size_gene", 0.7f);
        setTrait("plant_digestion", 0.05f);
        setTrait("meat_digestion", 0.9f);
        setTrait("toxin_resistance", 0.4f);
        setTrait("sight_range", 60.0f);
        setTrait("color_vision", 0.3f);
        setTrait("scent_detection", 0.7f);
    }

    void setOptimalState() {
        EnvironmentState env;
        env.temperature = 22.0f;
        env.humidity = 0.5f;
        env.time_of_day = 0.5f;
        
        OrganismState org;
        org.age_normalized = 0.5f;
        org.health = 1.0f;
        org.energy_level = 1.0f;
        org.is_sleeping = false;
        org.is_pregnant = false;
        
        const_cast<Phenotype&>(getPhenotype()).updateContext(env, org);
    }

private:
    static inline GeneRegistry registry_;

    void initializeRegistry() {
        registerGeneIfNeeded("nutrient_value", ChromosomeType::Metabolism, 0.0f, 100.0f);
        registerGeneIfNeeded("fruit_appeal", ChromosomeType::Metabolism, 0.0f, 1.0f);
        registerGeneIfNeeded("toxicity", ChromosomeType::Environmental, 0.0f, 1.0f);
        registerGeneIfNeeded("hardiness", ChromosomeType::Morphology, 0.0f, 1.0f);
        registerGeneIfNeeded("scent_production", ChromosomeType::Sensory, 0.0f, 1.0f);
        registerGeneIfNeeded("color_hue", ChromosomeType::Morphology, 0.0f, 1.0f);
        registerGeneIfNeeded("size_gene", ChromosomeType::Morphology, 0.0f, 1.0f);
        registerGeneIfNeeded("plant_digestion", ChromosomeType::Metabolism, 0.0f, 1.0f);
        registerGeneIfNeeded("meat_digestion", ChromosomeType::Metabolism, 0.0f, 1.0f);
        registerGeneIfNeeded("toxin_resistance", ChromosomeType::Environmental, 0.0f, 1.0f);
        registerGeneIfNeeded("sight_range", ChromosomeType::Sensory, 0.0f, 100.0f);
        registerGeneIfNeeded("color_vision", ChromosomeType::Sensory, 0.0f, 1.0f);
        registerGeneIfNeeded("scent_detection", ChromosomeType::Sensory, 0.0f, 1.0f);
    }

    void registerGeneIfNeeded(const std::string& name, ChromosomeType chrom,
                               float minVal, float maxVal) {
        if (!registry_.hasGene(name)) {
            GeneLimits limits(minVal, maxVal, 0.05f);
            GeneDefinition def(name, chrom, limits, DominanceType::Incomplete);
            registry_.registerGene(def);
        }
    }
};

//================================================================================
//  Test: buildScentSignature()
//================================================================================

void test_buildScentSignature_plant_with_high_fruit_appeal() {
    PerceptionSystem perception;
    MockOrganism plant;
    
    // Configure as a plant with high fruit appeal
    // With optimal state (adult, full health/energy), modulation = 100%
    plant.setTrait("nutrient_value", 60.0f);
    plant.setTrait("fruit_appeal", 0.9f);
    plant.setTrait("toxicity", 0.2f);
    plant.setTrait("hardiness", 0.3f);
    plant.setTrait("scent_production", 0.8f);
    plant.setTrait("color_hue", 0.4f);
    plant.setTrait("size_gene", 0.0f);
    
    auto signature = perception.buildScentSignature(plant);
    
    // Verify signature structure (8 elements, all in valid range)
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_GE(signature[i], 0.0f);
        TEST_ASSERT_LE(signature[i], 1.0f);
    }
    
    // Signature index mapping (verify actual trait-to-index mapping):
    // [0] nutrition = nutrient_value / 100 = 0.6
    // [1] fruit_appeal = 0.9
    // [2] toxicity = 0.2
    // [3] hardiness = 0.3
    // [4] scent_production = 0.8
    // [5] color_hue = 0.4
    // [6] size_gene = 0.0
    // [7] reserved = 0.0
    
    TEST_ASSERT_NEAR(0.6f, signature[0], 0.05f);   // nutrient_value/100 = 60/100
    TEST_ASSERT_NEAR(0.9f, signature[1], 0.05f);   // fruit_appeal
    TEST_ASSERT_NEAR(0.2f, signature[2], 0.05f);   // toxicity
    TEST_ASSERT_NEAR(0.3f, signature[3], 0.05f);   // hardiness
    TEST_ASSERT_NEAR(0.8f, signature[4], 0.05f);   // scent_production
    TEST_ASSERT_NEAR(0.4f, signature[5], 0.05f);   // color_hue
    TEST_ASSERT_NEAR(0.0f, signature[6], 0.05f);   // size_gene
    TEST_ASSERT_NEAR(0.0f, signature[7], 0.01f);   // reserved always 0
}

void test_buildScentSignature_creature_no_plant_traits() {
    PerceptionSystem perception;
    MockOrganism creature;
    
    // Configure as creature with no fruit_appeal (creatures aren't food plants)
    creature.setTrait("nutrient_value", 40.0f);
    creature.setTrait("fruit_appeal", 0.0f);  // Creatures don't have fruit appeal
    creature.setTrait("toxicity", 0.0f);
    creature.setTrait("hardiness", 0.6f);
    creature.setTrait("scent_production", 0.4f);
    creature.setTrait("color_hue", 0.5f);
    creature.setTrait("size_gene", 0.7f);
    
    auto signature = perception.buildScentSignature(creature);
    
    // Verify signature structure is valid
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_GE(signature[i], 0.0f);
        TEST_ASSERT_LE(signature[i], 1.0f);
    }
    
    // Fruit appeal should be low/zero for creature
    TEST_ASSERT_LT(signature[1], 0.3f);
    
    // Reserved is always 0
    TEST_ASSERT_NEAR(0.0f, signature[7], 0.01f);
}

void test_buildScentSignature_mixed_organism_traits() {
    PerceptionSystem perception;
    MockOrganism organism;
    
    // Mixed organism - partial traits (like a semi-mobile plant or hybrid)
    organism.setTrait("nutrient_value", 25.0f);
    organism.setTrait("fruit_appeal", 0.3f);
    organism.setTrait("toxicity", 0.5f);
    organism.setTrait("scent_production", 0.1f);
    // hardiness, color_hue, size_gene not set - should use defaults
    
    auto signature = perception.buildScentSignature(organism);
    
    // Verify all values are valid (0-1 range)
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_GE(signature[i], 0.0f);
        TEST_ASSERT_LE(signature[i], 1.0f);
    }
    
    // Reserved always 0
    TEST_ASSERT_NEAR(0.0f, signature[7], 0.01f);
}

void test_buildScentSignature_values_clamped() {
    PerceptionSystem perception;
    MockOrganism organism;
    
    // Set extreme values that should be clamped
    organism.setTrait("nutrient_value", 200.0f);  // Should clamp to 1.0 after /100
    organism.setTrait("fruit_appeal", 1.5f);  // Should clamp to 1.0
    
    auto signature = perception.buildScentSignature(organism);
    
    // Values should be clamped to [0, 1]
    TEST_ASSERT_LE(signature[0], 1.0f);
    TEST_ASSERT_LE(signature[1], 1.0f);
    TEST_ASSERT_GE(signature[0], 0.0f);
    TEST_ASSERT_GE(signature[1], 0.0f);
}

//================================================================================
//  Test: depositScent()
//================================================================================

void test_depositScent_plant_deposits_food_trail() {
    PerceptionSystem perception;
    MockOrganism plant;
    plant.configureAsPlant();
    
    ScentLayer scentLayer(100, 100);
    
    // Deposit plant scent
    perception.depositScent(plant, 50, 50, -1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Verify scent was deposited
    auto scents = scentLayer.getScentsAt(50, 50);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT(scents[0].type == ScentType::FOOD_TRAIL);
    TEST_ASSERT_GT(scents[0].intensity, 0.0f);
    
    // Verify signature matches what buildScentSignature would produce
    auto expectedSig = perception.buildScentSignature(plant);
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_NEAR(expectedSig[i], scents[0].signature[i], 0.01f);
    }
}

void test_depositScent_creature_deposits_mate_seeking() {
    PerceptionSystem perception;
    MockOrganism creature;
    creature.configureAsHerbivore();
    
    ScentLayer scentLayer(100, 100);
    
    // Deposit mate-seeking scent
    perception.depositScent(creature, 25, 75, 42, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Verify scent was deposited
    auto scents = scentLayer.getScentsAt(25, 75);
    TEST_ASSERT_EQ(1, scents.size());
    TEST_ASSERT(scents[0].type == ScentType::MATE_SEEKING);
    TEST_ASSERT_EQ(42, scents[0].creatureId);
    TEST_ASSERT_GT(scents[0].intensity, 0.0f);
}

void test_depositScent_low_production_deposits_nothing() {
    PerceptionSystem perception;
    MockOrganism organism;
    
    // Set scent production below threshold (0.05)
    organism.setTrait("scent_production", 0.02f);
    
    ScentLayer scentLayer(100, 100);
    
    // Try to deposit scent
    perception.depositScent(organism, 50, 50, 1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Verify no scent was deposited
    auto scents = scentLayer.getScentsAt(50, 50);
    TEST_ASSERT(scents.empty());
}

void test_depositScent_intensity_scaled_by_appeal() {
    PerceptionSystem perception;
    ScentLayer scentLayer(100, 100);
    
    // Organism with high fruit appeal
    MockOrganism highAppeal;
    highAppeal.setTrait("scent_production", 0.5f);
    highAppeal.setTrait("fruit_appeal", 0.9f);
    
    // Organism with low fruit appeal
    MockOrganism lowAppeal;
    lowAppeal.setTrait("scent_production", 0.5f);
    lowAppeal.setTrait("fruit_appeal", 0.1f);
    
    perception.depositScent(highAppeal, 10, 10, 1, scentLayer, ScentType::FOOD_TRAIL, 0);
    perception.depositScent(lowAppeal, 20, 20, 2, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    auto highScents = scentLayer.getScentsAt(10, 10);
    auto lowScents = scentLayer.getScentsAt(20, 20);
    
    // Higher fruit appeal should produce stronger scent intensity
    TEST_ASSERT_GT(highScents[0].intensity, lowScents[0].intensity);
}

//================================================================================
//  Test: isEdibleScent()
//================================================================================

void test_isEdibleScent_herbivore_can_eat_nontoxic_plant() {
    PerceptionSystem perception;
    MockOrganism herbivore;
    herbivore.configureAsHerbivore();
    
    // Plant signature: nutritious, non-toxic, soft
    std::array<float, 8> plantSig = {
        0.5f,   // [0] nutrition
        0.7f,   // [1] fruit_appeal
        0.1f,   // [2] toxicity (low)
        0.2f,   // [3] hardiness (soft)
        0.6f,   // [4] scent_production
        0.3f,   // [5] color_hue
        0.0f,   // [6] size_gene
        0.0f    // [7] reserved
    };
    
    bool canEat = perception.isEdibleScent(plantSig, herbivore);
    TEST_ASSERT(canEat);
}

void test_isEdibleScent_carnivore_rejects_plant() {
    PerceptionSystem perception;
    MockOrganism carnivore;
    carnivore.configureAsCarnivore();
    
    // Same plant signature
    std::array<float, 8> plantSig = {
        0.5f, 0.7f, 0.1f, 0.2f, 0.6f, 0.3f, 0.0f, 0.0f
    };
    
    bool canEat = perception.isEdibleScent(plantSig, carnivore);
    
    // Carnivore has plant_digestion < 0.1, should reject
    TEST_ASSERT(!canEat);
}

void test_isEdibleScent_rejects_toxic_beyond_resistance() {
    PerceptionSystem perception;
    MockOrganism herbivore;
    herbivore.configureAsHerbivore();
    // Herbivore has toxin_resistance = 0.3
    
    // Highly toxic plant: toxicity = 0.6 > 0.3 * 1.5 = 0.45
    std::array<float, 8> toxicPlantSig = {
        0.5f,   // nutrition
        0.7f,   // fruit_appeal
        0.6f,   // toxicity (HIGH - beyond resistance)
        0.2f,   // hardiness
        0.6f,   // scent_production
        0.3f,   // color_hue
        0.0f,   // size_gene
        0.0f    // reserved
    };
    
    bool canEat = perception.isEdibleScent(toxicPlantSig, herbivore);
    TEST_ASSERT(!canEat);
}

void test_isEdibleScent_accepts_toxic_within_resistance() {
    PerceptionSystem perception;
    MockOrganism herbivore;
    herbivore.setTrait("plant_digestion", 0.8f);
    herbivore.setTrait("toxin_resistance", 0.8f);  // High resistance
    herbivore.setTrait("hardiness", 0.7f);  // Good hardiness to eat tough plants
    
    // Moderately toxic plant: toxicity = 0.4 <= 0.8 * 1.5 = 1.2 (well within)
    // Plant hardiness = 0.2 <= 0.7 + 0.5 = 1.2 (well within)
    std::array<float, 8> modToxicSig = {
        0.5f,   // nutrition - sufficient
        0.7f,   // fruit_appeal
        0.4f,   // toxicity (within resistance: 0.4 <= 0.8 * 1.5)
        0.2f,   // hardiness (within capability: 0.2 <= 0.7 + 0.5)
        0.6f,   // scent_production
        0.3f,   // color_hue
        0.0f,   // size_gene
        0.0f    // reserved
    };
    
    bool canEat = perception.isEdibleScent(modToxicSig, herbivore);
    TEST_ASSERT(canEat);
}

void test_isEdibleScent_rejects_plant_too_hard() {
    PerceptionSystem perception;
    MockOrganism herbivore;
    herbivore.setTrait("plant_digestion", 0.8f);
    herbivore.setTrait("toxin_resistance", 0.5f);
    herbivore.setTrait("hardiness", 0.2f);  // Low hardiness (weak jaw/teeth)
    
    // Very hard plant: hardiness = 0.8 > 0.2 + 0.5 = 0.7
    std::array<float, 8> hardPlantSig = {
        0.5f,   // nutrition
        0.7f,   // fruit_appeal
        0.1f,   // toxicity (low)
        0.8f,   // hardiness (VERY HARD)
        0.6f,   // scent_production
        0.3f,   // color_hue
        0.0f,   // size_gene
        0.0f    // reserved
    };
    
    bool canEat = perception.isEdibleScent(hardPlantSig, herbivore);
    TEST_ASSERT(!canEat);
}

void test_isEdibleScent_rejects_low_nutrition() {
    PerceptionSystem perception;
    MockOrganism herbivore;
    herbivore.configureAsHerbivore();
    
    // Low nutrition plant: nutrition = 0.02 < 0.05
    std::array<float, 8> lowNutritionSig = {
        0.02f,  // nutrition (too low)
        0.7f,   // fruit_appeal
        0.1f,   // toxicity
        0.2f,   // hardiness
        0.6f,   // scent_production
        0.3f,   // color_hue
        0.0f,   // size_gene
        0.0f    // reserved
    };
    
    bool canEat = perception.isEdibleScent(lowNutritionSig, herbivore);
    TEST_ASSERT(!canEat);
}

//================================================================================
//  Test: Range Calculations
//================================================================================

void test_calculateVisualRange_color_vision_boosts_colorful_targets() {
    PerceptionSystem perception;
    MockOrganism seeker;
    seeker.setTrait("sight_range", 50.0f);
    seeker.setTrait("color_vision", 0.8f);
    
    float colorfulTarget = 0.9f;
    float dullTarget = 0.0f;
    
    float rangeColorful = perception.calculateVisualRange(seeker, colorfulTarget);
    float rangeDull = perception.calculateVisualRange(seeker, dullTarget);
    
    // Formula: sightRange + (colorVision * targetColorfulness * COLOR_VISION_RANGE_MULTIPLIER)
    // COLOR_VISION_RANGE_MULTIPLIER = 100 (from PerceptionSystem.hpp)
    // For colorful: 50 + (0.8 * 0.9 * 100) = 50 + 72 = 122
    // For dull: 50 + (0.8 * 0.0 * 100) = 50 + 0 = 50
    TEST_ASSERT_NEAR(122.0f, rangeColorful, 1.0f);
    TEST_ASSERT_NEAR(50.0f, rangeDull, 1.0f);
    
    // Verify colorful > dull
    TEST_ASSERT_GT(rangeColorful, rangeDull);
}

void test_calculateVisualRange_low_color_vision_minimal_boost() {
    PerceptionSystem perception;
    
    MockOrganism goodColorVision;
    goodColorVision.setTrait("sight_range", 50.0f);
    goodColorVision.setTrait("color_vision", 0.9f);
    
    MockOrganism poorColorVision;
    poorColorVision.setTrait("sight_range", 50.0f);
    poorColorVision.setTrait("color_vision", 0.1f);
    
    float targetColorfulness = 0.9f;
    
    float rangeGood = perception.calculateVisualRange(goodColorVision, targetColorfulness);
    float rangePoor = perception.calculateVisualRange(poorColorVision, targetColorfulness);
    
    // Better color vision = better range for colorful targets
    TEST_ASSERT_GT(rangeGood, rangePoor);
}

void test_calculateVisualRange_no_color_no_boost() {
    PerceptionSystem perception;
    MockOrganism seeker;
    seeker.setTrait("sight_range", 50.0f);
    seeker.setTrait("color_vision", 0.5f);
    
    float dullTarget = 0.0f;
    float colorfulTarget = 1.0f;
    
    float rangeDull = perception.calculateVisualRange(seeker, dullTarget);
    float rangeColorful = perception.calculateVisualRange(seeker, colorfulTarget);
    
    // Dull target = minimal/no color bonus
    // Range for dull should be less than for colorful
    TEST_ASSERT_LT(rangeDull, rangeColorful);
}

void test_calculateScentRange_based_on_detection_trait() {
    PerceptionSystem perception;
    
    MockOrganism midDetection;
    midDetection.setTrait("scent_detection", 0.5f);
    
    MockOrganism highDetection;
    highDetection.setTrait("scent_detection", 1.0f);
    
    float rangeMid = perception.calculateScentRange(midDetection);
    float rangeHigh = perception.calculateScentRange(highDetection);
    
    // Higher scent detection = longer range
    TEST_ASSERT_GT(rangeHigh, rangeMid);
    TEST_ASSERT_GT(rangeMid, 0.0f);
}

void test_calculateScentRange_full_detection() {
    PerceptionSystem perception;
    MockOrganism seeker;
    seeker.setTrait("scent_detection", 1.0f);
    
    float range = perception.calculateScentRange(seeker);
    
    // Formula: scent_detection * SCENT_RANGE_MULTIPLIER
    // SCENT_RANGE_MULTIPLIER = 100 (from PerceptionSystem.hpp)
    // Expected: 1.0 * 100 = 100 tiles
    TEST_ASSERT_NEAR(100.0f, range, 1.0f);
}

void test_calculateScentRange_no_detection() {
    PerceptionSystem perception;
    MockOrganism seeker;
    seeker.setTrait("scent_detection", 0.0f);
    
    float range = perception.calculateScentRange(seeker);
    
    // No detection = zero range
    TEST_ASSERT_NEAR(0.0f, range, 0.1f);
}

void test_calculateEffectiveRange_returns_max_of_visual_and_scent() {
    PerceptionSystem perception;
    MockOrganism seeker;
    
    // Setup where scent should dominate
    seeker.setTrait("sight_range", 10.0f);  // Low sight
    seeker.setTrait("color_vision", 0.0f);  // No color vision boost
    seeker.setTrait("scent_detection", 1.0f);  // Maximum scent detection
    
    float targetColorfulness = 0.0f;  // Dull target
    
    float visualRange = perception.calculateVisualRange(seeker, targetColorfulness);
    float scentRange = perception.calculateScentRange(seeker);
    float effectiveRange = perception.calculateEffectiveRange(seeker, targetColorfulness);
    
    // Effective should be max of the two
    float expectedMax = std::max(visualRange, scentRange);
    TEST_ASSERT_NEAR(expectedMax, effectiveRange, 0.1f);
    
    // Scent should be the dominant one here
    TEST_ASSERT_NEAR(scentRange, effectiveRange, 0.1f);
}

void test_calculateEffectiveRange_visual_dominates() {
    PerceptionSystem perception;
    MockOrganism seeker;
    
    // Setup where visual should dominate
    seeker.setTrait("sight_range", 100.0f);  // High sight
    seeker.setTrait("color_vision", 1.0f);  // Maximum color vision
    seeker.setTrait("scent_detection", 0.0f);  // No scent detection
    
    float targetColorfulness = 1.0f;  // Very colorful target
    
    float visualRange = perception.calculateVisualRange(seeker, targetColorfulness);
    float scentRange = perception.calculateScentRange(seeker);
    float effectiveRange = perception.calculateEffectiveRange(seeker, targetColorfulness);
    
    // Effective should be max of the two
    float expectedMax = std::max(visualRange, scentRange);
    TEST_ASSERT_NEAR(expectedMax, effectiveRange, 0.1f);
    
    // Visual should be the dominant one here
    TEST_ASSERT_NEAR(visualRange, effectiveRange, 0.1f);
}

//================================================================================
//  Test: Direction Detection
//================================================================================

void test_detectFoodDirection_finds_plant_scent_in_range() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Setup herbivore seeker with full detection
    MockOrganism seeker;
    seeker.setTrait("plant_digestion", 0.8f);
    seeker.setTrait("meat_digestion", 0.1f);
    seeker.setTrait("toxin_resistance", 0.5f);
    seeker.setTrait("hardiness", 0.5f);
    seeker.setTrait("scent_detection", 1.0f);  // Max detection = 100 tile range
    
    // Verify we have expected range
    float scentRange = perception.calculateScentRange(seeker);
    TEST_ASSERT_NEAR(100.0f, scentRange, 5.0f);  // Should be ~100 tiles
    
    // Setup plant at known position (10 tiles away, well within range)
    const int PLANT_X = 10;
    const int PLANT_Y = 50;
    
    MockOrganism plant;
    plant.setTrait("nutrient_value", 50.0f);
    plant.setTrait("fruit_appeal", 0.7f);
    plant.setTrait("toxicity", 0.1f);  // Low toxicity (within resistance)
    plant.setTrait("hardiness", 0.2f);  // Soft plant (within capability)
    plant.setTrait("scent_production", 0.6f);
    
    perception.depositScent(plant, PLANT_X, PLANT_Y, -1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Verify scent was deposited
    auto scentsAtPlant = scentLayer.getScentsAt(PLANT_X, PLANT_Y);
    TEST_ASSERT(!scentsAtPlant.empty());
    
    // Seeker at (0, 50) should find food at (10, 50)
    auto result = perception.detectFoodDirection(seeker, 0, PLANT_Y, scentLayer);
    
    // MUST find the food
    TEST_ASSERT(result.has_value());
    TEST_ASSERT_EQ(PLANT_X, result->first);
    TEST_ASSERT_EQ(PLANT_Y, result->second);
}

void test_detectFoodDirection_rejects_inedible_scent() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Setup carnivore seeker (can't eat plants) - set traits directly
    MockOrganism carnivore;
    carnivore.setTrait("plant_digestion", 0.05f);  // Can't digest plants
    carnivore.setTrait("meat_digestion", 0.9f);
    carnivore.setTrait("scent_detection", 1.0f);
    
    // Setup plant scent
    MockOrganism plant;
    plant.setTrait("nutrient_value", 50.0f);
    plant.setTrait("fruit_appeal", 0.7f);
    plant.setTrait("toxicity", 0.1f);
    plant.setTrait("hardiness", 0.3f);
    plant.setTrait("scent_production", 0.6f);
    perception.depositScent(plant, 50, 50, -1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Carnivore should NOT find this food (can't digest plants)
    auto result = perception.detectFoodDirection(carnivore, 0, 50, scentLayer);
    
    TEST_ASSERT(!result.has_value());
}

void test_detectFoodDirection_out_of_range() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Setup seeker with limited range - set traits directly
    MockOrganism seeker;
    seeker.setTrait("plant_digestion", 0.8f);
    seeker.setTrait("toxin_resistance", 0.3f);
    seeker.setTrait("hardiness", 0.5f);
    seeker.setTrait("scent_detection", 0.2f);  // Only 20 tile range
    
    // Setup plant far away (at 50 tiles distance)
    MockOrganism plant;
    plant.setTrait("nutrient_value", 50.0f);
    plant.setTrait("fruit_appeal", 0.7f);
    plant.setTrait("toxicity", 0.1f);
    plant.setTrait("hardiness", 0.3f);
    plant.setTrait("scent_production", 0.6f);
    perception.depositScent(plant, 50, 0, -1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Seeker at (0, 0) should NOT find food at (50, 0)
    auto result = perception.detectFoodDirection(seeker, 0, 0, scentLayer);
    
    TEST_ASSERT(!result.has_value());
}

void test_detectFoodDirection_rejects_toxic_plant() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Setup herbivore with low toxin resistance - set traits directly
    MockOrganism seeker;
    seeker.setTrait("plant_digestion", 0.8f);
    seeker.setTrait("toxin_resistance", 0.1f);  // Low resistance
    seeker.setTrait("hardiness", 0.5f);
    seeker.setTrait("scent_detection", 1.0f);
    
    // Setup highly toxic plant
    MockOrganism toxicPlant;
    toxicPlant.setTrait("nutrient_value", 50.0f);
    toxicPlant.setTrait("fruit_appeal", 0.7f);
    toxicPlant.setTrait("toxicity", 0.8f);  // Highly toxic
    toxicPlant.setTrait("hardiness", 0.2f);
    toxicPlant.setTrait("scent_production", 0.6f);
    
    perception.depositScent(toxicPlant, 50, 50, -1, scentLayer, ScentType::FOOD_TRAIL, 0);
    
    // Seeker should reject this toxic plant
    auto result = perception.detectFoodDirection(seeker, 0, 50, scentLayer);
    
    TEST_ASSERT(!result.has_value());
}

void test_detectMateDirection_finds_mate_ignores_own() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Seeker is creature ID 5
    const int SEEKER_ID = 5;
    const int OTHER_ID = 7;
    
    // Setup seeker
    MockOrganism seeker;
    seeker.setTrait("scent_production", 0.8f);  // High production
    seeker.setTrait("scent_detection", 1.0f);   // Full detection
    
    // Verify we have range
    float scentRange = perception.calculateScentRange(seeker);
    TEST_ASSERT_GT(scentRange, 10.0f);  // Must have at least 10 tile range for this test
    
    // Setup: OTHER creature's scent is STRONGER (so it will be found first)
    // This tests that the system finds other creatures' scents
    MockOrganism otherCreature;
    otherCreature.setTrait("scent_production", 0.9f);  // Stronger scent
    perception.depositScent(otherCreature, 5, 0, OTHER_ID, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Deposit seeker's OWN scent - WEAKER (current implementation limitation)
    // NOTE: If own scent were stronger, the current implementation would
    // return nothing instead of skipping to next strongest.
    MockOrganism seekerAsSource;
    seekerAsSource.setTrait("scent_production", 0.5f);  // Weaker scent
    perception.depositScent(seekerAsSource, 2, 0, SEEKER_ID, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Verify both scents were deposited
    auto ownScents = scentLayer.getScentsAt(2, 0);
    auto otherScents = scentLayer.getScentsAt(5, 0);
    TEST_ASSERT(!ownScents.empty());
    TEST_ASSERT(!otherScents.empty());
    
    // Verify creature IDs
    TEST_ASSERT_EQ(SEEKER_ID, ownScents[0].creatureId);
    TEST_ASSERT_EQ(OTHER_ID, otherScents[0].creatureId);
    
    // Seeker at (0, 0) should find OTHER creature's scent (the stronger one)
    auto result = perception.detectMateDirection(seeker, 0, 0, SEEKER_ID, scentLayer);
    
    // Should find the other creature's scent
    TEST_ASSERT(result.has_value());
    TEST_ASSERT_EQ(5, result->first);
    TEST_ASSERT_EQ(0, result->second);
}

/**
 * @brief Test that documents a limitation: own scent stronger = no result
 *
 * The current implementation uses getStrongestScentInRadius then rejects
 * if that strongest scent is own scent. It does NOT iterate to find
 * the next strongest scent. This is a known limitation.
 */
void test_detectMateDirection_limitation_own_scent_stronger_returns_nothing() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    const int SEEKER_ID = 5;
    const int OTHER_ID = 7;
    
    MockOrganism seeker;
    seeker.setTrait("scent_detection", 1.0f);
    
    // Make OWN scent STRONGER
    MockOrganism seekerAsSource;
    seekerAsSource.setTrait("scent_production", 0.9f);  // Stronger
    perception.depositScent(seekerAsSource, 2, 0, SEEKER_ID, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Other creature's scent is weaker
    MockOrganism otherCreature;
    otherCreature.setTrait("scent_production", 0.5f);  // Weaker
    perception.depositScent(otherCreature, 5, 0, OTHER_ID, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Current behavior: returns nothing because strongest scent is own scent
    // FUTURE IMPROVEMENT: Should skip own scent and return (5, 0)
    auto result = perception.detectMateDirection(seeker, 0, 0, SEEKER_ID, scentLayer);
    
    // Document current behavior (not ideal but accurate)
    TEST_ASSERT(!result.has_value());  // Returns nothing currently
}

void test_detectMateDirection_no_scent_range() {
    PerceptionSystem perception;
    ScentLayer scentLayer(200, 200);
    
    // Setup seeker with no scent detection
    MockOrganism seeker;
    seeker.setTrait("scent_detection", 0.0f);
    
    // Deposit mate scent nearby
    MockOrganism other;
    other.configureAsHerbivore();
    perception.depositScent(other, 10, 10, 7, scentLayer, ScentType::MATE_SEEKING, 0);
    
    // Seeker with no scent detection shouldn't find anything
    auto result = perception.detectMateDirection(seeker, 0, 0, 5, scentLayer);
    
    TEST_ASSERT(!result.has_value());
}

//================================================================================
//  Test: Utility Methods
//================================================================================

void test_calculateDistance() {
    // Test basic distance calculation
    float d1 = PerceptionSystem::calculateDistance(0, 0, 3, 4);
    TEST_ASSERT_NEAR(5.0f, d1, 0.01f);  // 3-4-5 triangle
    
    float d2 = PerceptionSystem::calculateDistance(10, 10, 10, 10);
    TEST_ASSERT_NEAR(0.0f, d2, 0.01f);  // Same point
    
    float d3 = PerceptionSystem::calculateDistance(0, 0, 10, 0);
    TEST_ASSERT_NEAR(10.0f, d3, 0.01f);  // Horizontal
}

void test_isWithinRange() {
    TEST_ASSERT(PerceptionSystem::isWithinRange(0, 0, 3, 4, 5.0f));   // Exactly at range
    TEST_ASSERT(PerceptionSystem::isWithinRange(0, 0, 3, 4, 6.0f));   // Within range
    TEST_ASSERT(!PerceptionSystem::isWithinRange(0, 0, 3, 4, 4.0f));  // Outside range
    TEST_ASSERT(PerceptionSystem::isWithinRange(0, 0, 0, 0, 0.0f));   // Same point, 0 range
}

void test_calculateSignatureSimilarity() {
    PerceptionSystem perception;
    
    // Identical signatures
    std::array<float, 8> sig1 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f};
    std::array<float, 8> sig2 = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f};
    
    float similarity = perception.calculateSignatureSimilarity(sig1, sig2);
    TEST_ASSERT_NEAR(1.0f, similarity, 0.01f);
    
    // Completely different signatures
    std::array<float, 8> sig3 = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 8> sig4 = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};
    
    float diff = perception.calculateSignatureSimilarity(sig3, sig4);
    TEST_ASSERT_LT(diff, 0.5f);  // Should be low similarity
}

//================================================================================
//  Test: Organism-Agnostic Behavior
//================================================================================

void test_plant_and_creature_produce_valid_signatures() {
    PerceptionSystem perception;
    
    MockOrganism plant;
    plant.configureAsPlant();
    
    MockOrganism creature;
    creature.configureAsHerbivore();
    
    auto plantSig = perception.buildScentSignature(plant);
    auto creatureSig = perception.buildScentSignature(creature);
    
    // Both should produce valid 8-element signatures
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_GE(plantSig[i], 0.0f);
        TEST_ASSERT_LE(plantSig[i], 1.0f);
        TEST_ASSERT_GE(creatureSig[i], 0.0f);
        TEST_ASSERT_LE(creatureSig[i], 1.0f);
    }
    
    // Plant should have higher fruit_appeal
    TEST_ASSERT_GT(plantSig[1], creatureSig[1]);
    
    // Creature should have higher size_gene
    TEST_ASSERT_GT(creatureSig[6], plantSig[6]);
}

void test_diet_checking_uses_phenotype_not_type() {
    PerceptionSystem perception;
    
    // Create two organisms with same traits but conceptually different "types"
    // The perception system shouldn't care about type - only phenotype traits
    MockOrganism organism1;
    organism1.setTrait("plant_digestion", 0.8f);
    organism1.setTrait("toxin_resistance", 0.3f);
    organism1.setTrait("hardiness", 0.5f);
    
    MockOrganism organism2;
    organism2.setTrait("plant_digestion", 0.8f);
    organism2.setTrait("toxin_resistance", 0.3f);
    organism2.setTrait("hardiness", 0.5f);
    
    // Same plant signature
    std::array<float, 8> plantSig = {
        0.5f, 0.7f, 0.1f, 0.2f, 0.6f, 0.3f, 0.0f, 0.0f
    };
    
    // Both should have same edibility result since same traits
    bool canEat1 = perception.isEdibleScent(plantSig, organism1);
    bool canEat2 = perception.isEdibleScent(plantSig, organism2);
    
    TEST_ASSERT_EQ(canEat1, canEat2);
    TEST_ASSERT(canEat1);  // Both should be able to eat it
}

//================================================================================
//  Main Entry Point
//================================================================================

void runPerceptionSystemTests() {
    BEGIN_TEST_GROUP("buildScentSignature Tests");
    RUN_TEST(test_buildScentSignature_plant_with_high_fruit_appeal);
    RUN_TEST(test_buildScentSignature_creature_no_plant_traits);
    RUN_TEST(test_buildScentSignature_mixed_organism_traits);
    RUN_TEST(test_buildScentSignature_values_clamped);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("depositScent Tests");
    RUN_TEST(test_depositScent_plant_deposits_food_trail);
    RUN_TEST(test_depositScent_creature_deposits_mate_seeking);
    RUN_TEST(test_depositScent_low_production_deposits_nothing);
    RUN_TEST(test_depositScent_intensity_scaled_by_appeal);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("isEdibleScent Tests");
    RUN_TEST(test_isEdibleScent_herbivore_can_eat_nontoxic_plant);
    RUN_TEST(test_isEdibleScent_carnivore_rejects_plant);
    RUN_TEST(test_isEdibleScent_rejects_toxic_beyond_resistance);
    RUN_TEST(test_isEdibleScent_accepts_toxic_within_resistance);
    RUN_TEST(test_isEdibleScent_rejects_plant_too_hard);
    RUN_TEST(test_isEdibleScent_rejects_low_nutrition);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Range Calculation Tests");
    RUN_TEST(test_calculateVisualRange_color_vision_boosts_colorful_targets);
    RUN_TEST(test_calculateVisualRange_low_color_vision_minimal_boost);
    RUN_TEST(test_calculateVisualRange_no_color_no_boost);
    RUN_TEST(test_calculateScentRange_based_on_detection_trait);
    RUN_TEST(test_calculateScentRange_full_detection);
    RUN_TEST(test_calculateScentRange_no_detection);
    RUN_TEST(test_calculateEffectiveRange_returns_max_of_visual_and_scent);
    RUN_TEST(test_calculateEffectiveRange_visual_dominates);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Direction Detection Tests");
    RUN_TEST(test_detectFoodDirection_finds_plant_scent_in_range);
    RUN_TEST(test_detectFoodDirection_rejects_inedible_scent);
    RUN_TEST(test_detectFoodDirection_out_of_range);
    RUN_TEST(test_detectFoodDirection_rejects_toxic_plant);
    RUN_TEST(test_detectMateDirection_finds_mate_ignores_own);
    RUN_TEST(test_detectMateDirection_limitation_own_scent_stronger_returns_nothing);
    RUN_TEST(test_detectMateDirection_no_scent_range);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Utility Method Tests");
    RUN_TEST(test_calculateDistance);
    RUN_TEST(test_isWithinRange);
    RUN_TEST(test_calculateSignatureSimilarity);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Organism-Agnostic Behavior Tests");
    RUN_TEST(test_plant_and_creature_produce_valid_signatures);
    RUN_TEST(test_diet_checking_uses_phenotype_not_type);
    END_TEST_GROUP();
}

// Standalone main for running just PerceptionSystem tests
#ifdef PERCEPTION_SYSTEM_TEST_MAIN
int main() {
    std::cout << "=== EcoSim PerceptionSystem Tests ===" << std::endl;
    
    runPerceptionSystemTests();
    
    TestSuite::instance().printSummary();
    
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
