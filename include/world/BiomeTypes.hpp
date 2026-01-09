#pragma once

/**
 * @file BiomeTypes.hpp
 * @brief Biome type definitions and utilities
 * 
 * This header provides access to the Biome enum and related utilities
 * without requiring the full ClimateWorldGenerator header. This helps
 * reduce coupling for headers that only need biome type information.
 * 
 * The Biome enum is defined in ClimateWorldGenerator.hpp and re-exported here.
 */

#include "world/ClimateWorldGenerator.hpp"

#include <string>

namespace EcoSim {

/**
 * @brief Convert a Biome enum value to its string name
 * @param biome The biome to convert
 * @return String representation of the biome (e.g., "Temperate Forest")
 */
inline std::string biomeToString(Biome biome) {
    return ClimateWorldGenerator::getBiomeName(biome);
}

} // namespace EcoSim
