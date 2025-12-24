/**
 * @file test_plant_copy.cpp
 * @brief Test to verify Plant copy/move semantics and detect dangling pointers
 *
 * This test verifies the fix for the memory corruption issue where
 * Plant's Phenotype member stores raw pointers that become dangling
 * after copy/move operations without proper Rule of Five implementation.
 *
 * The key bug: When a Plant is copied/moved, its phenotype_.genome_ pointer
 * must be rebound to THIS plant's genome, not the old plant's genome.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <cassert>
#include <cmath>
#include "genetics/organisms/Plant.hpp"
#include "genetics/organisms/PlantFactory.hpp"
#include "genetics/core/GeneRegistry.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/defaults/PlantGenes.hpp"

using namespace EcoSim::Genetics;

// Global registry for tests
static std::shared_ptr<GeneRegistry> g_registry;

void initRegistry() {
    if (!g_registry) {
        g_registry = std::make_shared<GeneRegistry>();
        UniversalGenes::registerDefaults(*g_registry);
    }
}

/**
 * @brief Verify that a Plant's phenotype correctly accesses its genome
 *
 * This checks that trait access works correctly after copy/move.
 * If the phenotype has a dangling pointer, this may crash or return garbage.
 */
bool verifyPhenotypeIntegrity(const Plant& plant, const char* context) {
    std::cout << "  [" << context << "] Genome at: " << &plant.getGenome() << std::endl;
    
    try {
        float growthRate = plant.getGrowthRate();
        float maxSize = plant.getMaxSize();
        float hardiness = plant.getHardiness();
        
        bool valid = (growthRate >= 0.0f && growthRate <= 10.0f) &&
                     (maxSize >= 0.0f && maxSize <= 100.0f) &&
                     (hardiness >= 0.0f && hardiness <= 1.0f);
        
        if (!valid) {
            std::cerr << "  [" << context << "] INVALID: growth=" << growthRate
                      << " maxSize=" << maxSize << " hardiness=" << hardiness << std::endl;
            return false;
        }
        
        std::cout << "  [" << context << "] OK: growth=" << growthRate << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "  [" << context << "] EXCEPTION: " << e.what() << std::endl;
        return false;
    }
}

// Test 1: Copy construction
bool testCopyConstruction() {
    std::cout << "\n=== Test 1: Copy Construction ===" << std::endl;
    initRegistry();
    
    Plant original(10, 20, *g_registry);
    if (!verifyPhenotypeIntegrity(original, "original")) return false;
    
    Plant copied(original);
    if (!verifyPhenotypeIntegrity(original, "original after copy")) return false;
    if (!verifyPhenotypeIntegrity(copied, "copied")) return false;
    
    // Verify different genome addresses
    if (&original.getGenome() == &copied.getGenome()) {
        std::cerr << "  ERROR: Genomes share same address!" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 2: Move construction
bool testMoveConstruction() {
    std::cout << "\n=== Test 2: Move Construction ===" << std::endl;
    initRegistry();
    
    Plant original(10, 20, *g_registry);
    float originalGrowth = original.getGrowthRate();
    
    Plant moved(std::move(original));
    if (!verifyPhenotypeIntegrity(moved, "moved")) return false;
    
    // Verify value preserved
    float movedGrowth = moved.getGrowthRate();
    if (std::abs(movedGrowth - originalGrowth) > 0.001f) {
        std::cerr << "  ERROR: Value changed after move!" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 3: shared_ptr move pattern (world.cpp pattern)
bool testSharedPtrMovePattern() {
    std::cout << "\n=== Test 3: shared_ptr Move Pattern ===" << std::endl;
    initRegistry();
    
    PlantFactory factory(g_registry);
    factory.registerDefaultTemplates();
    
    std::vector<std::shared_ptr<Plant>> plants;
    
    for (int i = 0; i < 10; i++) {
        Plant plant = factory.createFromTemplate("berry_bush", i, i);
        auto sharedPlant = std::make_shared<Plant>(std::move(plant));
        plants.push_back(sharedPlant);
    }
    
    // Force memory churn
    for (int i = 0; i < 100; i++) {
        std::vector<char> garbage(1000, 'X');
    }
    
    // Verify all plants still valid
    for (size_t i = 0; i < plants.size(); i++) {
        std::string ctx = "plant " + std::to_string(i);
        if (!verifyPhenotypeIntegrity(*plants[i], ctx.c_str())) return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 4: Large-scale stress test
bool testLargeScaleStress() {
    std::cout << "\n=== Test 4: Large-Scale Stress ===" << std::endl;
    initRegistry();
    
    PlantFactory factory(g_registry);
    factory.registerDefaultTemplates();
    
    const int NUM_PLANTS = 500;
    std::vector<std::shared_ptr<Plant>> plants;
    plants.reserve(NUM_PLANTS);
    
    std::vector<std::string> species = {"grass", "berry_bush", "oak_tree", "thorn_bush"};
    
    for (int i = 0; i < NUM_PLANTS; i++) {
        Plant plant = factory.createFromTemplate(species[i % 4], i % 100, i / 100);
        plants.push_back(std::make_shared<Plant>(std::move(plant)));
    }
    
    std::cout << "  Created " << plants.size() << " plants" << std::endl;
    
    // Verify sample
    for (int i = 0; i < NUM_PLANTS; i += 50) {
        std::string ctx = "plant " + std::to_string(i);
        if (!verifyPhenotypeIntegrity(*plants[i], ctx.c_str())) return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 5: Copy assignment
bool testCopyAssignment() {
    std::cout << "\n=== Test 5: Copy Assignment ===" << std::endl;
    initRegistry();
    
    Plant plant1(10, 20, *g_registry);
    Plant plant2(30, 40, *g_registry);
    float growth1 = plant1.getGrowthRate();
    
    plant2 = plant1;
    
    if (!verifyPhenotypeIntegrity(plant1, "plant1")) return false;
    if (!verifyPhenotypeIntegrity(plant2, "plant2")) return false;
    
    if (std::abs(plant2.getGrowthRate() - growth1) > 0.001f) {
        std::cerr << "  ERROR: Copy assignment didn't copy values!" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 6: Move assignment
bool testMoveAssignment() {
    std::cout << "\n=== Test 6: Move Assignment ===" << std::endl;
    initRegistry();
    
    Plant plant1(10, 20, *g_registry);
    Plant plant2(30, 40, *g_registry);
    float growth1 = plant1.getGrowthRate();
    
    plant2 = std::move(plant1);
    
    if (!verifyPhenotypeIntegrity(plant2, "plant2 after move")) return false;
    
    if (std::abs(plant2.getGrowthRate() - growth1) > 0.001f) {
        std::cerr << "  ERROR: Move assignment didn't preserve values!" << std::endl;
        return false;
    }
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

// Test 7: Offspring production
bool testOffspringProduction() {
    std::cout << "\n=== Test 7: Offspring Production ===" << std::endl;
    initRegistry();
    
    Plant parent(50, 50, *g_registry);
    if (!verifyPhenotypeIntegrity(parent, "parent")) return false;
    
    PlantFactory factory(g_registry);
    Plant offspring = factory.createOffspring(parent, parent, 55, 55);
    
    if (!verifyPhenotypeIntegrity(offspring, "offspring")) return false;
    
    std::cout << "  PASSED" << std::endl;
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Plant Copy/Move Semantics Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nVerifies phenotype correctly rebinds to" << std::endl;
    std::cout << "its own genome after copy/move operations." << std::endl;
    
    int passed = 0, failed = 0;
    
    try {
        if (testCopyConstruction()) passed++; else failed++;
        if (testMoveConstruction()) passed++; else failed++;
        if (testSharedPtrMovePattern()) passed++; else failed++;
        if (testLargeScaleStress()) passed++; else failed++;
        if (testCopyAssignment()) passed++; else failed++;
        if (testMoveAssignment()) passed++; else failed++;
        if (testOffspringProduction()) passed++; else failed++;
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "RESULTS: " << passed << "/" << (passed+failed) << " passed" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return (failed == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cerr << "\nCRASHED: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "\nCRASHED (likely dangling pointer)" << std::endl;
        return 1;
    }
}
