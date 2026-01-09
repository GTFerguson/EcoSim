/**
 * Title    : EcoSim - Pathfinding Sensitivity Tests
 * Author   : Claude / Gary Ferguson
 * Created  : Jan 2026
 * Purpose  : Test the gene-controlled pathfinding system that allows creatures
 *            to make risk-aware movement decisions based on environmental danger.
 *            
 *            Creatures with high ENVIRONMENTAL_SENSITIVITY gene values will 
 *            avoid hostile biomes (temperatures outside their tolerance), while
 *            low-sensitivity creatures prioritize direct paths regardless of danger.
 */

#include "test_framework.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnvironmentalStress.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/core/GeneDefinition.hpp"
#include "genetics/core/Genome.hpp"

#include <cmath>
#include <map>
#include <utility>

using namespace EcoSim::Genetics;
using namespace EcoSim::Testing;

//============================================================================
// Mock EnvironmentSystem for testing
//============================================================================

/**
 * Mock environment system that returns configurable temperatures per tile.
 */
class MockEnvironmentSystem {
public:
    MockEnvironmentSystem() = default;
    
    /// Set temperature for a specific tile
    void setTemperature(int x, int y, float temp) {
        _temperatures[std::make_pair(x, y)] = temp;
    }
    
    /// Set default temperature for tiles not explicitly configured
    void setDefaultTemperature(float temp) {
        _defaultTemp = temp;
    }
    
    /// Get temperature at a position
    float getTemperature(int x, int y) const {
        auto it = _temperatures.find(std::make_pair(x, y));
        if (it != _temperatures.end()) {
            return it->second;
        }
        return _defaultTemp;
    }
    
private:
    std::map<std::pair<int, int>, float> _temperatures;
    float _defaultTemp = 20.0f;
};

//============================================================================
// Test PathfindingContext for testing (mimics real PathfindingContext)
//============================================================================

/**
 * Custom PathfindingContext that uses MockEnvironmentSystem.
 * This lets us test tile cost calculation without a full World.
 */
struct TestPathfindingContext {
    float effectiveTolMin;
    float effectiveTolMax;
    float environmentalSensitivity;
    const MockEnvironmentSystem* mockEnv;
    
    static constexpr float DANGER_WEIGHT_FACTOR = 10.0f;
    
    float calculateTileCost(float baseCost, int x, int y) const {
        if (!mockEnv || environmentalSensitivity < 0.01f) {
            return baseCost;
        }
        
        float temp = mockEnv->getTemperature(x, y);
        
        float degreesOutside = 0.0f;
        if (temp < effectiveTolMin) {
            degreesOutside = effectiveTolMin - temp;
        } else if (temp > effectiveTolMax) {
            degreesOutside = temp - effectiveTolMax;
        }
        
        float environmentalCost = (degreesOutside / 10.0f) * environmentalSensitivity * DANGER_WEIGHT_FACTOR;
        
        return baseCost + environmentalCost;
    }
};

//============================================================================
// Test Cases
//============================================================================

/**
 * Test that ENVIRONMENTAL_SENSITIVITY gene is properly registered.
 */
void test_environmental_sensitivity_gene_registration() {
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Check gene exists
    TEST_ASSERT(registry.hasGene(UniversalGenes::ENVIRONMENTAL_SENSITIVITY));
    
    // Check gene definition
    const GeneDefinition& def = registry.getDefinition(UniversalGenes::ENVIRONMENTAL_SENSITIVITY);
    const GeneLimits& limits = def.getLimits();
    TEST_ASSERT_NEAR(limits.min_value, 0.0f, 0.01f);
    TEST_ASSERT_NEAR(limits.max_value, 2.0f, 0.01f);
    TEST_ASSERT_EQ(static_cast<int>(def.getChromosome()), static_cast<int>(ChromosomeType::Behavior));
}

/**
 * Test tile cost calculation - no danger (comfortable temperature).
 */
void test_pathfinding_comfortable_temperature() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setDefaultTemperature(20.0f);  // Comfortable
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    // Temperature 20C is within tolerance (10-30), so no extra cost
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, baseCost, 0.01f);  // No environmental penalty
}

/**
 * Test tile cost calculation - cold danger.
 */
void test_pathfinding_cold_temperature() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, -10.0f);  // Very cold
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;   // Creature can handle down to 10C
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    // Temperature -10C is 20C below tolerance minimum (10C)
    // Environmental cost = (20 / 10) * 1.0 * 10 = 20
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, 30.0f, 0.01f);  // baseCost (10) + envCost (20)
}

/**
 * Test tile cost calculation - hot danger.
 */
void test_pathfinding_hot_temperature() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, 50.0f);  // Very hot
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;  // Creature can handle up to 30C
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    // Temperature 50C is 20C above tolerance maximum (30C)
    // Environmental cost = (20 / 10) * 1.0 * 10 = 20
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, 30.0f, 0.01f);  // baseCost (10) + envCost (20)
}

/**
 * Test sensitivity scaling - zero sensitivity ignores environmental danger.
 */
void test_pathfinding_zero_sensitivity() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, -50.0f);  // Extremely cold
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 0.0f;  // No sensitivity
    ctx.mockEnv = &mockEnv;
    
    // Even with extreme cold, zero sensitivity means no extra cost
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, baseCost, 0.01f);  // No environmental penalty
}

/**
 * Test sensitivity scaling - high sensitivity strongly avoids danger.
 */
void test_pathfinding_high_sensitivity() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, -10.0f);  // Cold
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 2.0f;  // Maximum sensitivity
    ctx.mockEnv = &mockEnv;
    
    // Temperature -10C is 20C below tolerance minimum
    // Environmental cost = (20 / 10) * 2.0 * 10 = 40
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, 50.0f, 0.01f);  // baseCost (10) + envCost (40)
}

/**
 * Test null environment system returns base cost only (backward compatibility).
 */
void test_pathfinding_null_environment() {
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = nullptr;  // No environment system
    
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, baseCost, 0.01f);  // Backward compatible - no penalty
}

/**
 * Test that sensitivity below threshold (0.01) is treated as zero.
 */
void test_pathfinding_very_low_sensitivity() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, -50.0f);  // Extremely cold
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 0.005f;  // Below threshold
    ctx.mockEnv = &mockEnv;
    
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, baseCost, 0.01f);  // Treated as zero sensitivity
}

/**
 * Test tile cost at boundary temperature.
 */
void test_pathfinding_boundary_temperature() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, 10.0f);  // Exactly at tolerance minimum
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    // Temperature 10C is exactly at tolerance minimum - no penalty
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, baseCost, 0.01f);  // At boundary, no penalty
}

/**
 * Test moderate temperature slightly outside tolerance.
 */
void test_pathfinding_slightly_outside_tolerance() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setTemperature(5, 5, 5.0f);  // 5C below tolerance min
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    // Temperature 5C is 5C below tolerance minimum
    // Environmental cost = (5 / 10) * 1.0 * 10 = 5
    float baseCost = 10.0f;
    float totalCost = ctx.calculateTileCost(baseCost, 5, 5);
    
    TEST_ASSERT_NEAR(totalCost, 15.0f, 0.01f);  // baseCost (10) + envCost (5)
}

/**
 * Test that different tile positions have different costs based on temperature.
 */
void test_pathfinding_variable_tile_temperatures() {
    MockEnvironmentSystem mockEnv;
    mockEnv.setDefaultTemperature(20.0f);  // Safe default
    mockEnv.setTemperature(0, 0, 20.0f);   // Safe
    mockEnv.setTemperature(1, 0, -10.0f);  // Dangerous cold
    mockEnv.setTemperature(2, 0, 50.0f);   // Dangerous hot
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 10.0f;
    ctx.effectiveTolMax = 30.0f;
    ctx.environmentalSensitivity = 1.0f;
    ctx.mockEnv = &mockEnv;
    
    float baseCost = 10.0f;
    
    // Safe tile
    float cost0 = ctx.calculateTileCost(baseCost, 0, 0);
    TEST_ASSERT_NEAR(cost0, 10.0f, 0.01f);
    
    // Cold tile (-10C is 20 degrees below minimum)
    float cost1 = ctx.calculateTileCost(baseCost, 1, 0);
    TEST_ASSERT_NEAR(cost1, 30.0f, 0.01f);  // 10 + (20/10)*1*10
    
    // Hot tile (50C is 20 degrees above maximum)
    float cost2 = ctx.calculateTileCost(baseCost, 2, 0);
    TEST_ASSERT_NEAR(cost2, 30.0f, 0.01f);  // 10 + (20/10)*1*10
}

/**
 * Verify formula: cost = baseCost + (degreesOutside/10) * sensitivity * 10
 */
void test_pathfinding_formula_verification() {
    MockEnvironmentSystem mockEnv;
    
    TestPathfindingContext ctx;
    ctx.effectiveTolMin = 15.0f;
    ctx.effectiveTolMax = 25.0f;
    ctx.mockEnv = &mockEnv;
    
    float baseCost = 10.0f;
    
    // Test case 1: 7C outside tolerance (8C temp, min is 15C)
    mockEnv.setTemperature(0, 0, 8.0f);
    ctx.environmentalSensitivity = 1.0f;
    // Expected: 10 + (7/10) * 1.0 * 10 = 10 + 7 = 17
    float cost1 = ctx.calculateTileCost(baseCost, 0, 0);
    TEST_ASSERT_NEAR(cost1, 17.0f, 0.01f);
    
    // Test case 2: Same temp, sensitivity 0.5
    ctx.environmentalSensitivity = 0.5f;
    // Expected: 10 + (7/10) * 0.5 * 10 = 10 + 3.5 = 13.5
    float cost2 = ctx.calculateTileCost(baseCost, 0, 0);
    TEST_ASSERT_NEAR(cost2, 13.5f, 0.01f);
    
    // Test case 3: Different degrees outside (temp 40C, max 25C = 15 degrees outside)
    mockEnv.setTemperature(1, 1, 40.0f);
    ctx.environmentalSensitivity = 1.0f;
    // Expected: 10 + (15/10) * 1.0 * 10 = 10 + 15 = 25
    float cost3 = ctx.calculateTileCost(baseCost, 1, 1);
    TEST_ASSERT_NEAR(cost3, 25.0f, 0.01f);
}

/**
 * Test gene value access from phenotype.
 */
void test_pathfinding_phenotype_gene_access() {
    // Create a gene registry with all genes
    GeneRegistry registry;
    UniversalGenes::registerDefaults(registry);
    
    // Verify gene is registered
    TEST_ASSERT(registry.hasGene(UniversalGenes::ENVIRONMENTAL_SENSITIVITY));
    
    // Verify gene limits match specification
    const GeneDefinition& def = registry.getDefinition(UniversalGenes::ENVIRONMENTAL_SENSITIVITY);
    const GeneLimits& limits = def.getLimits();
    TEST_ASSERT_NEAR(limits.min_value, 0.0f, 0.01f);
    TEST_ASSERT_NEAR(limits.max_value, 2.0f, 0.01f);
    
    // Create a test genome using registry defaults
    Genome genome = UniversalGenes::createCreatureGenome(registry);
    
    // Check if gene is in genome
    TEST_ASSERT(genome.hasGene(UniversalGenes::ENVIRONMENTAL_SENSITIVITY));
    
    // Create phenotype from genome
    Phenotype phenotype(&genome, &registry);
    
    // Verify gene value is accessible using getTrait()
    // Note: The expressed value may differ from raw value due to phenotype modulation
    // (energy budget, expression levels, etc.) but should be non-zero and within gene limits
    float sensitivity = phenotype.getTrait(UniversalGenes::ENVIRONMENTAL_SENSITIVITY);
    
    // Expressed value should be valid (within gene range limits)
    TEST_ASSERT(sensitivity >= 0.0f);
    TEST_ASSERT(sensitivity <= 2.0f);
    // And should be non-zero (gene is being expressed)
    TEST_ASSERT(sensitivity > 0.0f);
}

//============================================================================
// Test Runner
//============================================================================

void runPathfindingSensitivityTests() {
    BEGIN_TEST_GROUP("Pathfinding Sensitivity System");
    
    // Gene registration tests
    RUN_TEST(test_environmental_sensitivity_gene_registration);
    
    // Basic tile cost calculation tests
    RUN_TEST(test_pathfinding_comfortable_temperature);
    RUN_TEST(test_pathfinding_cold_temperature);
    RUN_TEST(test_pathfinding_hot_temperature);
    
    // Sensitivity scaling tests
    RUN_TEST(test_pathfinding_zero_sensitivity);
    RUN_TEST(test_pathfinding_high_sensitivity);
    RUN_TEST(test_pathfinding_very_low_sensitivity);
    
    // Backward compatibility tests
    RUN_TEST(test_pathfinding_null_environment);
    
    // Edge case tests
    RUN_TEST(test_pathfinding_boundary_temperature);
    RUN_TEST(test_pathfinding_slightly_outside_tolerance);
    
    // Integration tests
    RUN_TEST(test_pathfinding_variable_tile_temperatures);
    RUN_TEST(test_pathfinding_formula_verification);
    RUN_TEST(test_pathfinding_phenotype_gene_access);
    
    END_TEST_GROUP();
}
