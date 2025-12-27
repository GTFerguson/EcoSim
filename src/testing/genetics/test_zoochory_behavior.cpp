/**
 * @file test_zoochory_behavior.cpp
 * @brief Tests for ZoochoryBehavior component
 *
 * Tests animal-mediated seed dispersal (zoochory):
 * - Endozoochory: seed gut passage, digestion timing
 * - Epizoochory: burr attachment/detachment mechanics
 *
 * The ZoochoryBehavior manages creature-side seed dispersal, working
 * with the SeedDispersal interaction class for calculations.
 */

#include "test_framework.hpp"
#include "genetics/behaviors/ZoochoryBehavior.hpp"
#include "genetics/behaviors/BehaviorContext.hpp"
#include "genetics/interactions/SeedDispersal.hpp"
#include "genetics/interfaces/IGeneticOrganism.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/core/Genome.hpp"
#include "genetics/core/Chromosome.hpp"
#include "genetics/core/Gene.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneticTypes.hpp"
#include "genetics/defaults/UniversalGenes.hpp"

#include <iostream>
#include <memory>
#include <cmath>

using namespace EcoSim;
using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//================================================================================
//  MockOrganism: Test Implementation of IGeneticOrganism
//================================================================================

/**
 * @brief Mock organism for testing ZoochoryBehavior
 *
 * Implements IGeneticOrganism interface with configurable traits.
 * Provides stable organism ID for testing.
 */
class ZoochoryMockOrganism : public IGeneticOrganism {
public:
    ZoochoryMockOrganism() : genome_(), phenotype_(&genome_, &registry_) {
        initializeRegistry();
        setOptimalState();
    }

    const Genome& getGenome() const override {
        return genome_;
    }

    Genome& getGenomeMutable() override {
        return genome_;
    }

    const Phenotype& getPhenotype() const override {
        return phenotype_;
    }

    void updatePhenotype() override {
        phenotype_.invalidateCache();
    }
    
    // Position and ID methods (required by IGeneticOrganism)
    int getX() const override { return 0; }
    int getY() const override { return 0; }
    int getId() const override { return 0; }

    /**
     * @brief Set a trait directly on the genome for testing
     */
    void setTrait(const std::string& name, float value) {
        ChromosomeType chromType = ChromosomeType::Metabolism;
        
        if (name == UniversalGenes::HARDINESS || name == UniversalGenes::MAX_SIZE) {
            chromType = ChromosomeType::Morphology;
        } else if (name == UniversalGenes::LOCOMOTION) {
            chromType = ChromosomeType::Morphology;
        }
        
        if (genome_.hasGene(name)) {
            Gene& existingGene = genome_.getGeneMutable(name);
            existingGene.setAlleleValues(value);
        } else {
            GeneValue geneVal = value;
            Gene gene(name, geneVal);
            genome_.addGene(gene, chromType);
        }
        
        if (!registry_.hasGene(name)) {
            float maxVal = (name == UniversalGenes::MAX_SIZE) ? 10.0f : 1.0f;
            GeneLimits limits(0.0f, maxVal, 0.05f);
            GeneDefinition def(name, chromType, limits, DominanceType::Incomplete);
            registry_.registerGene(def);
        }
        
        phenotype_.invalidateCache();
    }

    /**
     * @brief Set optimal organism state for predictable modulation
     */
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
        
        phenotype_.updateContext(env, org);
    }

private:
    Genome genome_;
    mutable GeneRegistry registry_;
    mutable Phenotype phenotype_;

    void initializeRegistry() {
        registerGeneIfNeeded(UniversalGenes::HARDINESS, ChromosomeType::Morphology, 0.0f, 1.0f);
        registerGeneIfNeeded(UniversalGenes::MAX_SIZE, ChromosomeType::Morphology, 0.0f, 10.0f);
        registerGeneIfNeeded(UniversalGenes::LOCOMOTION, ChromosomeType::Morphology, 0.0f, 2.0f);
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

/**
 * @brief Mock BehaviorContext for testing
 */
class MockZoochoryBehaviorContext : public BehaviorContext {
public:
    MockZoochoryBehaviorContext() = default;
};

//================================================================================
//  Test: Epizoochory - Burr Attachment
//================================================================================

void test_attachBurr_storesBurr() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 12345;
    int plantX = 10;
    int plantY = 20;
    int strategy = static_cast<int>(DispersalStrategy::ANIMAL_BURR);
    
    behavior.attachBurr(organismId, plantX, plantY, strategy);
    
    TEST_ASSERT(behavior.hasBurrs(organismId));
}

void test_hasBurrs_trueWhenAttached() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 111;
    behavior.attachBurr(organismId, 5, 5, 0);
    
    TEST_ASSERT(behavior.hasBurrs(organismId));
}

void test_hasBurrs_falseWhenNone() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 222;
    
    TEST_ASSERT(!behavior.hasBurrs(organismId));
}

void test_burrDetachment_probabilistic() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 666;
    
    int totalBurrs = 100;
    for (int i = 0; i < totalBurrs; ++i) {
        behavior.attachBurr(organismId, i, i, 0);
    }
    
    auto events = behavior.processOrganismSeeds(organismId, 50, 50, 1);
    int detachedFirstTick = static_cast<int>(events.size());
    
    TEST_ASSERT_GT(detachedFirstTick, 0);
    
    TEST_ASSERT_LT(detachedFirstTick, totalBurrs);
}

//================================================================================
//  Test: Endozoochory - Gut Seed Passage
//================================================================================

void test_consumeSeeds_storesInGut() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 333;
    int plantX = 15;
    int plantY = 25;
    int count = 3;
    float viability = 0.9f;
    
    behavior.consumeSeeds(organismId, plantX, plantY, count, viability);
    
    auto events = behavior.processOrganismSeeds(organismId, 100, 100, 1);
    
    TEST_ASSERT(events.empty());
}

void test_processGutSeeds_decrementsTicks() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 444;
    behavior.consumeSeeds(organismId, 10, 10, 1, 0.8f);
    
    auto events1 = behavior.processOrganismSeeds(organismId, 50, 50, 1);
    TEST_ASSERT(events1.empty());
    
    auto events2 = behavior.processOrganismSeeds(organismId, 50, 50, 1);
    TEST_ASSERT(events2.empty());
}

void test_processGutSeeds_createsDispersalEvent() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 555;
    int plantX = 20;
    int plantY = 30;
    float viability = 0.85f;
    
    behavior.consumeSeeds(organismId, plantX, plantY, 1, viability);
    
    std::vector<DispersalEvent> events;
    for (int tick = 0; tick < 550; ++tick) {
        events = behavior.processOrganismSeeds(organismId, 100, 100, 1);
        if (!events.empty()) break;
    }
    
    TEST_ASSERT(!events.empty());
    
    if (!events.empty()) {
        const auto& event = events[0];
        TEST_ASSERT(event.originX == plantX);
        TEST_ASSERT(event.originY == plantY);
        TEST_ASSERT(event.targetX == 100);
        TEST_ASSERT(event.targetY == 100);
        TEST_ASSERT(event.method == DispersalStrategy::ANIMAL_FRUIT);
        TEST_ASSERT_NEAR(viability, event.seedViability, 0.01f);
    }
}

//================================================================================
//  Test: Behavior Interface
//================================================================================

void test_isApplicable_alwaysTrue() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    ZoochoryMockOrganism organism;
    MockZoochoryBehaviorContext ctx;
    
    TEST_ASSERT(behavior.isApplicable(organism, ctx));
}

void test_priority_isIdle() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    ZoochoryMockOrganism organism;
    
    float priority = behavior.getPriority(organism);
    
    TEST_ASSERT_NEAR(static_cast<float>(BehaviorPriority::IDLE), priority, 0.01f);
}

void test_execute_reportsStatus() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    ZoochoryMockOrganism organism;
    MockZoochoryBehaviorContext ctx;
    
    auto result = behavior.execute(organism, ctx);
    
    TEST_ASSERT(result.executed);
    TEST_ASSERT(result.completed);
    TEST_ASSERT_NEAR(0.0f, result.energyCost, 0.01f);
    TEST_ASSERT(!result.debugInfo.empty());
}

void test_getEnergyCost_isZero() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    ZoochoryMockOrganism organism;
    
    float cost = behavior.getEnergyCost(organism);
    
    TEST_ASSERT_NEAR(0.0f, cost, 0.01f);
}

void test_getId_returnsZoochory() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    std::string id = behavior.getId();
    
    TEST_ASSERT(id == "zoochory");
}

//================================================================================
//  Test: State Management
//================================================================================

void test_clearOrganismData_removesBurrs() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 777;
    behavior.attachBurr(organismId, 5, 5, 0);
    TEST_ASSERT(behavior.hasBurrs(organismId));
    
    behavior.clearOrganismData(organismId);
    
    TEST_ASSERT(!behavior.hasBurrs(organismId));
}

void test_clearOrganismData_removesGutSeeds() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int organismId = 888;
    behavior.consumeSeeds(organismId, 10, 10, 5, 0.9f);
    
    behavior.clearOrganismData(organismId);
    
    for (int tick = 0; tick < 600; ++tick) {
        auto events = behavior.processOrganismSeeds(organismId, 50, 50, 1);
        TEST_ASSERT(events.empty());
    }
}

void test_multipleOrganisms_independentState() {
    SeedDispersal dispersal;
    ZoochoryBehavior behavior(dispersal);
    
    unsigned int org1 = 1001;
    unsigned int org2 = 1002;
    
    behavior.attachBurr(org1, 10, 10, 0);
    behavior.consumeSeeds(org2, 20, 20, 2, 0.8f);
    
    TEST_ASSERT(behavior.hasBurrs(org1));
    TEST_ASSERT(!behavior.hasBurrs(org2));
    
    behavior.clearOrganismData(org1);
    
    TEST_ASSERT(!behavior.hasBurrs(org1));
    
    for (int tick = 0; tick < 550; ++tick) {
        auto events = behavior.processOrganismSeeds(org2, 50, 50, 1);
        if (!events.empty()) {
            TEST_ASSERT(events[0].originX == 20);
            break;
        }
    }
}

//================================================================================
//  Main Entry Point
//================================================================================

void runZoochoryBehaviorTests() {
    BEGIN_TEST_GROUP("Epizoochory (Burr Attachment) Tests");
    RUN_TEST(test_attachBurr_storesBurr);
    RUN_TEST(test_hasBurrs_trueWhenAttached);
    RUN_TEST(test_hasBurrs_falseWhenNone);
    RUN_TEST(test_burrDetachment_probabilistic);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Endozoochory (Gut Passage) Tests");
    RUN_TEST(test_consumeSeeds_storesInGut);
    RUN_TEST(test_processGutSeeds_decrementsTicks);
    RUN_TEST(test_processGutSeeds_createsDispersalEvent);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("Behavior Interface Tests");
    RUN_TEST(test_isApplicable_alwaysTrue);
    RUN_TEST(test_priority_isIdle);
    RUN_TEST(test_execute_reportsStatus);
    RUN_TEST(test_getEnergyCost_isZero);
    RUN_TEST(test_getId_returnsZoochory);
    END_TEST_GROUP();

    BEGIN_TEST_GROUP("State Management Tests");
    RUN_TEST(test_clearOrganismData_removesBurrs);
    RUN_TEST(test_clearOrganismData_removesGutSeeds);
    RUN_TEST(test_multipleOrganisms_independentState);
    END_TEST_GROUP();
}

#ifdef ZOOCHORY_BEHAVIOR_TEST_MAIN
int main() {
    std::cout << "=== EcoSim ZoochoryBehavior Tests ===" << std::endl;
    
    runZoochoryBehaviorTests();
    
    TestSuite::instance().printSummary();
    
    return TestSuite::instance().allPassed() ? 0 : 1;
}
#endif
