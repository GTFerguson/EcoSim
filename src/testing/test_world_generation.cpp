/**
 * @file test_world_generation.cpp
 * @brief Test executable for climate-based world generation with PNG output
 * 
 * Generates multiple worlds with different seeds and exports visualization
 * PNGs for each layer (elevation, temperature, moisture, biomes).
 * 
 * Usage:
 *   ./test_world_generation [seed] [width] [height]
 * 
 * Outputs to: output/worldgen/
 */

#include "../../include/world/ClimateWorldGenerator.hpp"
#include "../../include/world/WorldGenPNG.hpp"
#include "../../include/world/WorldGenerator.hpp"
#include "../../include/world/WorldGrid.hpp"

#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

using namespace EcoSim;

/**
 * @brief Print configuration summary
 */
void printConfig(const ClimateGeneratorConfig& config) {
    std::cout << "=== World Generation Configuration ===" << std::endl;
    std::cout << "  Dimensions: " << config.width << " x " << config.height << std::endl;
    std::cout << "  Seed: " << config.seed << std::endl;
    std::cout << "  Sea Level: " << config.seaLevel << std::endl;
    std::cout << "  Island Mode: " << (config.isIsland ? "Yes" : "No") << std::endl;
    std::cout << "  Equator Position: " << config.equatorPosition << std::endl;
    std::cout << "  Temperature Range: " << config.temperatureRange << "°C" << std::endl;
    std::cout << "  Generate Rivers: " << (config.generateRivers ? "Yes" : "No") << std::endl;
    std::cout << "  Max Rivers: " << config.maxRivers << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Print biome statistics for a generated world
 */
void printBiomeStats(const ClimateWorldGenerator& generator) {
    const auto& climateMap = generator.getClimateMap();
    if (climateMap.empty()) return;
    
    unsigned int width = static_cast<unsigned int>(climateMap.size());
    unsigned int height = static_cast<unsigned int>(climateMap[0].size());
    
    // Count biomes
    int biomeCounts[static_cast<int>(Biome::COUNT)] = {0};
    int riverCount = 0;
    int lakeCount = 0;
    
    float minTemp = 100.0f, maxTemp = -100.0f;
    float minElev = 1.0f, maxElev = 0.0f;
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const TileClimate& climate = climateMap[x][y];
            biomeCounts[static_cast<int>(climate.biome())]++;
            
            if (climate.feature == TerrainFeature::RIVER) riverCount++;
            if (climate.feature == TerrainFeature::LAKE) lakeCount++;
            
            minTemp = std::min(minTemp, climate.temperature);
            maxTemp = std::max(maxTemp, climate.temperature);
            minElev = std::min(minElev, climate.elevation);
            maxElev = std::max(maxElev, climate.elevation);
        }
    }
    
    unsigned int totalTiles = width * height;
    
    std::cout << "=== World Statistics ===" << std::endl;
    std::cout << "  Temperature Range: " << std::fixed << std::setprecision(1) 
              << minTemp << "°C to " << maxTemp << "°C" << std::endl;
    std::cout << "  Elevation Range: " << std::setprecision(3) 
              << minElev << " to " << maxElev << std::endl;
    std::cout << "  River Tiles: " << riverCount << std::endl;
    std::cout << "  Lake Tiles: " << lakeCount << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Biome Distribution ===" << std::endl;
    for (int i = 0; i < static_cast<int>(Biome::COUNT); ++i) {
        if (biomeCounts[i] > 0) {
            float percent = 100.0f * biomeCounts[i] / totalTiles;
            std::cout << "  " << std::setw(25) << std::left 
                      << ClimateWorldGenerator::getBiomeName(static_cast<Biome>(i))
                      << ": " << std::setw(6) << biomeCounts[i] 
                      << " (" << std::fixed << std::setprecision(1) << percent << "%)"
                      << std::endl;
        }
    }
    std::cout << std::endl;
}

/**
 * @brief Generate a single world and export PNGs
 */
bool generateWorld(unsigned int seed, unsigned int width, unsigned int height,
                   const std::string& outputDir) {
    std::cout << "========================================" << std::endl;
    std::cout << "Generating world with seed: " << seed << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Configure generator
    ClimateGeneratorConfig config;
    config.width = width;
    config.height = height;
    config.seed = seed;
    config.isIsland = true;
    config.generateRivers = true;
    config.maxRivers = 30;
    
    printConfig(config);
    
    // Create generator and grid
    ClimateWorldGenerator generator(config);
    WorldGrid grid;
    
    // Time the generation
    auto startTime = std::chrono::high_resolution_clock::now();
    
    generator.generate(grid);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Generation Time: " << duration.count() << " ms" << std::endl;
    std::cout << std::endl;
    
    // Print statistics
    printBiomeStats(generator);
    
    // Export PNGs
    std::stringstream prefix;
    prefix << "world_seed" << seed;
    
    PNGExportConfig exportConfig;
    exportConfig.outputDir = outputDir;
    exportConfig.prefix = prefix.str();
    
    std::cout << "=== Exporting PNGs ===" << std::endl;
    bool success = WorldGenPNG::exportAll(generator, exportConfig);
    std::cout << std::endl;
    
    return success;
}

/**
 * @brief Compare old generator with new climate generator
 */
void compareGenerators(unsigned int seed, unsigned int width, unsigned int height) {
    std::cout << "========================================" << std::endl;
    std::cout << "Comparing Old vs New Generator" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Old generator
    MapGen oldMapGen;
    oldMapGen.seed = static_cast<double>(seed);
    oldMapGen.rows = height;
    oldMapGen.cols = width;
    oldMapGen.isIsland = true;
    
    OctaveGen oldOctaveGen;
    
    WorldGenerator oldGenerator(oldMapGen, oldOctaveGen);
    WorldGrid oldGrid(width, height);
    
    auto startOld = std::chrono::high_resolution_clock::now();
    oldGenerator.generate(oldGrid);
    auto endOld = std::chrono::high_resolution_clock::now();
    auto durationOld = std::chrono::duration_cast<std::chrono::milliseconds>(endOld - startOld);
    
    std::cout << "Old Generator Time: " << durationOld.count() << " ms" << std::endl;
    
    // New climate generator
    ClimateGeneratorConfig newConfig;
    newConfig.width = width;
    newConfig.height = height;
    newConfig.seed = seed;
    newConfig.isIsland = true;
    
    ClimateWorldGenerator newGenerator(newConfig);
    WorldGrid newGrid;
    
    auto startNew = std::chrono::high_resolution_clock::now();
    newGenerator.generate(newGrid);
    auto endNew = std::chrono::high_resolution_clock::now();
    auto durationNew = std::chrono::duration_cast<std::chrono::milliseconds>(endNew - startNew);
    
    std::cout << "New Generator Time: " << durationNew.count() << " ms" << std::endl;
    
    // Compare terrain type distribution
    int oldTerrainCounts[18] = {0};
    int newTerrainCounts[18] = {0};
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int oldType = static_cast<int>(oldGrid(x, y).getTerrainType());
            int newType = static_cast<int>(newGrid(x, y).getTerrainType());
            if (oldType >= 0 && oldType < 18) oldTerrainCounts[oldType]++;
            if (newType >= 0 && newType < 18) newTerrainCounts[newType]++;
        }
    }
    
    std::cout << std::endl << "Terrain Type Comparison:" << std::endl;
    const char* terrainNames[] = {
        "Deep Water", "Water", "Shallow Water", "Shallow Water 2",
        "Sand", "Desert Sand", "Plains", "Savanna",
        "Short Grass", "Long Grass", "Forest",
        "Mountain", "Mountain 2", "Mountain 3",
        "Snow", "Peaks"
    };
    
    unsigned int total = width * height;
    for (int i = 0; i < 16; ++i) {
        float oldPct = 100.0f * oldTerrainCounts[i] / total;
        float newPct = 100.0f * newTerrainCounts[i] / total;
        if (oldTerrainCounts[i] > 0 || newTerrainCounts[i] > 0) {
            std::cout << "  " << std::setw(16) << std::left << terrainNames[i]
                      << ": Old " << std::setw(5) << std::fixed << std::setprecision(1) << oldPct << "%"
                      << " | New " << std::setw(5) << newPct << "%"
                      << std::endl;
        }
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "============================================" << std::endl;
    std::cout << "  EcoSim Climate World Generation Test" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    unsigned int baseSeed = 12345;
    unsigned int width = 500;
    unsigned int height = 500;
    std::string outputDir = "output/worldgen";
    
    if (argc >= 2) {
        baseSeed = static_cast<unsigned int>(std::stoul(argv[1]));
    }
    if (argc >= 3) {
        width = static_cast<unsigned int>(std::stoul(argv[2]));
    }
    if (argc >= 4) {
        height = static_cast<unsigned int>(std::stoul(argv[3]));
    }
    
    std::cout << "Output directory: " << outputDir << std::endl;
    std::cout << std::endl;
    
    // Generate worlds with different seeds
    std::vector<unsigned int> seeds = {baseSeed, baseSeed + 1, baseSeed + 2};
    
    for (unsigned int seed : seeds) {
        if (!generateWorld(seed, width, height, outputDir)) {
            std::cerr << "Failed to generate world with seed " << seed << std::endl;
        }
    }
    
    // Compare old vs new generator
    compareGenerators(baseSeed, width, height);
    
    // Test different configurations
    std::cout << "========================================" << std::endl;
    std::cout << "Testing Configuration Variations" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Continental (not island) world
    {
        ClimateGeneratorConfig config;
        config.width = width;
        config.height = height;
        config.seed = baseSeed + 100;
        config.isIsland = false;  // Continental
        config.generateRivers = true;
        
        ClimateWorldGenerator generator(config);
        WorldGrid grid;
        generator.generate(grid);
        
        PNGExportConfig exportConfig;
        exportConfig.outputDir = outputDir;
        exportConfig.prefix = "world_continental";
        WorldGenPNG::exportAll(generator, exportConfig);
        
        std::cout << "Generated continental world" << std::endl;
    }
    
    // High moisture world (more rainforests)
    try {
        ClimateGeneratorConfig config;
        config.width = width;
        config.height = height;
        config.seed = baseSeed + 200;
        config.isIsland = true;
        config.moistureScale = 1.3f;  // More moisture
        config.generateRivers = true;
        config.maxRivers = 30;
        
        ClimateWorldGenerator generator(config);
        WorldGrid grid;
        generator.generate(grid);
        
        PNGExportConfig exportConfig;
        exportConfig.outputDir = outputDir;
        exportConfig.prefix = "world_wet";
        WorldGenPNG::exportAll(generator, exportConfig);
        
        std::cout << "Generated wet world" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating wet world: " << e.what() << std::endl;
    }
    
    // Low moisture world (more deserts)
    try {
        ClimateGeneratorConfig config;
        config.width = width;
        config.height = height;
        config.seed = baseSeed + 300;
        config.isIsland = true;
        config.moistureScale = 0.6f;  // Less moisture
        config.generateRivers = true;
        
        ClimateWorldGenerator generator(config);
        WorldGrid grid;
        generator.generate(grid);
        
        PNGExportConfig exportConfig;
        exportConfig.outputDir = outputDir;
        exportConfig.prefix = "world_dry";
        WorldGenPNG::exportAll(generator, exportConfig);
        
        std::cout << "Generated dry world" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating dry world: " << e.what() << std::endl;
    }
    
    // Cold world (ice age)
    try {
        ClimateGeneratorConfig config;
        config.width = width;
        config.height = height;
        config.seed = baseSeed + 400;
        config.isIsland = true;
        config.baseTemperature = 0.0f;  // Colder
        config.generateRivers = true;
        
        ClimateWorldGenerator generator(config);
        WorldGrid grid;
        generator.generate(grid);
        
        PNGExportConfig exportConfig;
        exportConfig.outputDir = outputDir;
        exportConfig.prefix = "world_cold";
        WorldGenPNG::exportAll(generator, exportConfig);
        
        std::cout << "Generated cold world" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error generating cold world: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "  Generation Complete!" << std::endl;
    std::cout << "  Check " << outputDir << "/ for PNG outputs" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
