#ifndef ECOSIM_WORLD_GEN_PNG_HPP
#define ECOSIM_WORLD_GEN_PNG_HPP

/**
 * @file WorldGenPNG.hpp
 * @brief PNG export utilities for world generation visualization
 * 
 * Provides functions to export world generation data layers (elevation,
 * temperature, moisture, biomes) to PNG images for debugging and iteration.
 */

#include "ClimateWorldGenerator.hpp"
#include <string>
#include <cstdint>

namespace EcoSim {

/**
 * @brief PNG export configuration
 */
struct PNGExportConfig {
    std::string outputDir = "output/worldgen";
    std::string prefix = "world";
    bool includeElevation = true;
    bool includeTemperature = true;
    bool includeMoisture = true;
    bool includeBiomes = true;
    bool includeComposite = true;
};

/**
 * @brief Color representation for PNG export
 */
struct RGB {
    uint8_t r, g, b;
    
    RGB() : r(0), g(0), b(0) {}
    RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    
    // Interpolate between two colors
    static RGB lerp(const RGB& a, const RGB& b, float t) {
        t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
        return RGB(
            static_cast<uint8_t>(a.r + (b.r - a.r) * t),
            static_cast<uint8_t>(a.g + (b.g - a.g) * t),
            static_cast<uint8_t>(a.b + (b.b - a.b) * t)
        );
    }
};

/**
 * @brief Export world generation data to PNG files
 * 
 * Generates separate PNG files for each data layer:
 * - elevation.png: Height map (black=low, white=high)
 * - temperature.png: Temperature map (blue=cold, red=hot)
 * - moisture.png: Moisture map (yellow=dry, blue=wet)
 * - biomes.png: Biome map (distinct colors per biome)
 * - composite.png: Combined visualization
 */
class WorldGenPNG {
public:
    /**
     * @brief Export all layers for a generated world
     * @param generator The ClimateWorldGenerator with generated data
     * @param config Export configuration
     * @return true if all exports succeeded
     */
    static bool exportAll(const ClimateWorldGenerator& generator, 
                          const PNGExportConfig& config = PNGExportConfig{});
    
    /**
     * @brief Export elevation map to PNG
     * @param generator The ClimateWorldGenerator with generated data
     * @param filename Output filename
     * @return true if export succeeded
     */
    static bool exportElevation(const ClimateWorldGenerator& generator,
                                const std::string& filename);
    
    /**
     * @brief Export temperature map to PNG
     * @param generator The ClimateWorldGenerator with generated data
     * @param filename Output filename
     * @return true if export succeeded
     */
    static bool exportTemperature(const ClimateWorldGenerator& generator,
                                  const std::string& filename);
    
    /**
     * @brief Export moisture map to PNG
     * @param generator The ClimateWorldGenerator with generated data
     * @param filename Output filename
     * @return true if export succeeded
     */
    static bool exportMoisture(const ClimateWorldGenerator& generator,
                               const std::string& filename);
    
    /**
     * @brief Export biome map to PNG
     * @param generator The ClimateWorldGenerator with generated data
     * @param filename Output filename
     * @return true if export succeeded
     */
    static bool exportBiomes(const ClimateWorldGenerator& generator,
                             const std::string& filename);
    
    /**
     * @brief Export composite visualization to PNG
     * @param generator The ClimateWorldGenerator with generated data
     * @param filename Output filename
     * @return true if export succeeded
     */
    static bool exportComposite(const ClimateWorldGenerator& generator,
                                const std::string& filename);
    
    /**
     * @brief Get color for a specific biome type
     * @param biome The biome type
     * @return RGB color for that biome
     */
    static RGB getBiomeColor(Biome biome);
    
    /**
     * @brief Get blended color for a BiomeBlend (ecotone transitions)
     * @param blend The biome blend with weighted contributions
     * @return RGB color interpolated from contributing biome colors
     */
    static RGB getBlendedBiomeColor(const BiomeBlend& blend);
    
    /**
     * @brief Get color for elevation value (grayscale gradient)
     * @param elevation Normalized elevation (0-1)
     * @param seaLevel Sea level threshold
     * @return RGB color
     */
    static RGB getElevationColor(float elevation, float seaLevel = 0.4f);
    
    /**
     * @brief Get color for temperature value (cold blue to hot red)
     * @param temperature Temperature in Celsius
     * @param minTemp Minimum expected temperature (default -40)
     * @param maxTemp Maximum expected temperature (default 50)
     * @return RGB color
     */
    static RGB getTemperatureColor(float temperature, 
                                   float minTemp = -40.0f, 
                                   float maxTemp = 50.0f);
    
    /**
     * @brief Get color for moisture value (dry yellow to wet blue)
     * @param moisture Normalized moisture (0-1)
     * @return RGB color
     */
    static RGB getMoistureColor(float moisture);
    
private:
    /**
     * @brief Write RGB data to PNG file
     * @param filename Output filename
     * @param data RGB pixel data (width * height * 3 bytes)
     * @param width Image width
     * @param height Image height
     * @return true if write succeeded
     */
    static bool writePNG(const std::string& filename,
                         const uint8_t* data,
                         unsigned int width,
                         unsigned int height);
    
    /**
     * @brief Ensure output directory exists
     * @param path Directory path
     * @return true if directory exists or was created
     */
    static bool ensureDirectory(const std::string& path);
};

} // namespace EcoSim

#endif // ECOSIM_WORLD_GEN_PNG_HPP
