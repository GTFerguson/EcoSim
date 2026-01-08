/**
 * @file WorldGenPNG.cpp
 * @brief Implementation of PNG export utilities for world generation
 */

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../external/stb_image_write.h"

#include "../../include/world/WorldGenPNG.hpp"

#include <vector>
#include <sys/stat.h>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

namespace EcoSim {

//=============================================================================
// Biome Color Table
//=============================================================================

static const RGB BIOME_COLORS[] = {
    // OCEAN_DEEP - dark blue
    RGB(20, 40, 100),
    // OCEAN_SHALLOW - medium blue
    RGB(40, 80, 160),
    // OCEAN_COAST - light blue
    RGB(80, 140, 200),
    // FRESHWATER - cyan-blue
    RGB(60, 160, 200),
    
    // ICE_SHEET - white-blue
    RGB(220, 230, 255),
    // TUNDRA - gray-brown
    RGB(160, 160, 140),
    // TAIGA - dark green
    RGB(50, 90, 60),
    
    // BOREAL_FOREST - blue-green
    RGB(40, 80, 70),
    // TEMPERATE_RAINFOREST - dark rich green
    RGB(30, 100, 50),
    // TEMPERATE_FOREST - medium green
    RGB(50, 120, 60),
    // TEMPERATE_GRASSLAND - yellow-green
    RGB(140, 180, 80),
    
    // TROPICAL_RAINFOREST - bright green
    RGB(40, 140, 40),
    // TROPICAL_SEASONAL_FOREST - lime green
    RGB(90, 150, 50),
    // SAVANNA - tan/olive
    RGB(180, 170, 100),
    
    // DESERT_HOT - sandy tan
    RGB(220, 200, 140),
    // DESERT_COLD - gray-tan
    RGB(180, 170, 150),
    // STEPPE - dry yellow
    RGB(200, 190, 120),
    // SHRUBLAND - olive
    RGB(140, 150, 90),
    
    // ALPINE_MEADOW - light green
    RGB(150, 190, 120),
    // ALPINE_TUNDRA - gray
    RGB(140, 140, 140),
    // MOUNTAIN_BARE - dark gray
    RGB(100, 100, 100),
    // GLACIER - bright white
    RGB(240, 250, 255)
};

//=============================================================================
// Color Mapping Functions
//=============================================================================

RGB WorldGenPNG::getBiomeColor(Biome biome) {
    int index = static_cast<int>(biome);
    if (index >= 0 && index < static_cast<int>(Biome::COUNT)) {
        return BIOME_COLORS[index];
    }
    return RGB(128, 128, 128);  // Gray for unknown
}

RGB WorldGenPNG::getBlendedBiomeColor(const BiomeBlend& blend) {
    // Blend colors based on biome weights
    float r = 0.0f, g = 0.0f, b = 0.0f;
    
    for (int i = 0; i < blend.count; ++i) {
        RGB biomeColor = getBiomeColor(blend.contributions[i].biome);
        float weight = blend.contributions[i].weight;
        
        r += biomeColor.r * weight;
        g += biomeColor.g * weight;
        b += biomeColor.b * weight;
    }
    
    return RGB(
        static_cast<uint8_t>(r < 255.0f ? r : 255.0f),
        static_cast<uint8_t>(g < 255.0f ? g : 255.0f),
        static_cast<uint8_t>(b < 255.0f ? b : 255.0f)
    );
}

RGB WorldGenPNG::getElevationColor(float elevation, float seaLevel) {
    if (elevation < seaLevel) {
        // Water: dark blue to light blue based on depth
        float depth = (seaLevel - elevation) / seaLevel;
        return RGB::lerp(
            RGB(80, 140, 200),   // Shallow
            RGB(10, 30, 80),     // Deep
            depth
        );
    } else {
        // Land: green to brown to gray to white
        float landHeight = (elevation - seaLevel) / (1.0f - seaLevel);
        
        if (landHeight < 0.3f) {
            // Low land: green
            return RGB::lerp(
                RGB(80, 140, 80),
                RGB(120, 160, 80),
                landHeight / 0.3f
            );
        } else if (landHeight < 0.6f) {
            // Mid elevation: tan/brown
            return RGB::lerp(
                RGB(140, 140, 100),
                RGB(120, 100, 80),
                (landHeight - 0.3f) / 0.3f
            );
        } else if (landHeight < 0.85f) {
            // High elevation: gray
            return RGB::lerp(
                RGB(100, 100, 100),
                RGB(160, 160, 160),
                (landHeight - 0.6f) / 0.25f
            );
        } else {
            // Mountain peaks: snow
            return RGB::lerp(
                RGB(200, 200, 200),
                RGB(255, 255, 255),
                (landHeight - 0.85f) / 0.15f
            );
        }
    }
}

RGB WorldGenPNG::getTemperatureColor(float temperature, float minTemp, float maxTemp) {
    // Normalize temperature to 0-1 range
    float t = (temperature - minTemp) / (maxTemp - minTemp);
    t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
    
    // Multi-stop gradient: blue -> cyan -> green -> yellow -> orange -> red
    if (t < 0.2f) {
        // Blue to cyan (very cold)
        return RGB::lerp(RGB(0, 0, 180), RGB(0, 180, 220), t / 0.2f);
    } else if (t < 0.4f) {
        // Cyan to green (cold)
        return RGB::lerp(RGB(0, 180, 220), RGB(80, 180, 80), (t - 0.2f) / 0.2f);
    } else if (t < 0.6f) {
        // Green to yellow (mild)
        return RGB::lerp(RGB(80, 180, 80), RGB(220, 220, 60), (t - 0.4f) / 0.2f);
    } else if (t < 0.8f) {
        // Yellow to orange (warm)
        return RGB::lerp(RGB(220, 220, 60), RGB(240, 140, 40), (t - 0.6f) / 0.2f);
    } else {
        // Orange to red (hot)
        return RGB::lerp(RGB(240, 140, 40), RGB(200, 40, 40), (t - 0.8f) / 0.2f);
    }
}

RGB WorldGenPNG::getMoistureColor(float moisture) {
    // Clamp moisture to 0-1
    moisture = (moisture < 0.0f) ? 0.0f : ((moisture > 1.0f) ? 1.0f : moisture);
    
    // Gradient: brown/tan (dry) -> green -> cyan -> blue (wet)
    if (moisture < 0.25f) {
        // Very dry: tan/brown
        return RGB::lerp(RGB(200, 180, 120), RGB(180, 180, 100), moisture / 0.25f);
    } else if (moisture < 0.5f) {
        // Dry: tan to light green
        return RGB::lerp(RGB(180, 180, 100), RGB(120, 180, 100), (moisture - 0.25f) / 0.25f);
    } else if (moisture < 0.75f) {
        // Moderate: green to cyan
        return RGB::lerp(RGB(120, 180, 100), RGB(80, 180, 180), (moisture - 0.5f) / 0.25f);
    } else {
        // Wet: cyan to blue
        return RGB::lerp(RGB(80, 180, 180), RGB(40, 100, 200), (moisture - 0.75f) / 0.25f);
    }
}

//=============================================================================
// File Operations
//=============================================================================

bool WorldGenPNG::ensureDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    
    // Create directory (and parents if needed)
    std::string current;
    for (size_t i = 0; i < path.size(); ++i) {
        current += path[i];
        if (path[i] == '/' || i == path.size() - 1) {
            if (!current.empty() && current != "/") {
                mkdir(current.c_str(), 0755);
            }
        }
    }
    
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool WorldGenPNG::writePNG(const std::string& filename,
                           const uint8_t* data,
                           unsigned int width,
                           unsigned int height) {
    // stbi_write_png expects row stride in bytes (width * 3 for RGB)
    int stride = width * 3;
    int result = stbi_write_png(filename.c_str(), width, height, 3, data, stride);
    return result != 0;
}

//=============================================================================
// Export Functions
//=============================================================================

bool WorldGenPNG::exportAll(const ClimateWorldGenerator& generator,
                            const PNGExportConfig& config) {
    // Ensure output directory exists
    if (!ensureDirectory(config.outputDir)) {
        std::cerr << "Failed to create output directory: " << config.outputDir << std::endl;
        return false;
    }
    
    bool success = true;
    std::string basePath = config.outputDir + "/" + config.prefix;
    
    if (config.includeElevation) {
        std::string filename = basePath + "_elevation.png";
        if (!exportElevation(generator, filename)) {
            std::cerr << "Failed to export elevation: " << filename << std::endl;
            success = false;
        } else {
            std::cout << "Exported: " << filename << std::endl;
        }
    }
    
    if (config.includeTemperature) {
        std::string filename = basePath + "_temperature.png";
        if (!exportTemperature(generator, filename)) {
            std::cerr << "Failed to export temperature: " << filename << std::endl;
            success = false;
        } else {
            std::cout << "Exported: " << filename << std::endl;
        }
    }
    
    if (config.includeMoisture) {
        std::string filename = basePath + "_moisture.png";
        if (!exportMoisture(generator, filename)) {
            std::cerr << "Failed to export moisture: " << filename << std::endl;
            success = false;
        } else {
            std::cout << "Exported: " << filename << std::endl;
        }
    }
    
    if (config.includeBiomes) {
        std::string filename = basePath + "_biomes.png";
        if (!exportBiomes(generator, filename)) {
            std::cerr << "Failed to export biomes: " << filename << std::endl;
            success = false;
        } else {
            std::cout << "Exported: " << filename << std::endl;
        }
    }
    
    if (config.includeComposite) {
        std::string filename = basePath + "_composite.png";
        if (!exportComposite(generator, filename)) {
            std::cerr << "Failed to export composite: " << filename << std::endl;
            success = false;
        } else {
            std::cout << "Exported: " << filename << std::endl;
        }
    }
    
    return success;
}

bool WorldGenPNG::exportElevation(const ClimateWorldGenerator& generator,
                                  const std::string& filename) {
    const auto& elevationMap = generator.getElevationMap();
    if (elevationMap.empty()) return false;
    
    unsigned int width = static_cast<unsigned int>(elevationMap.size());
    unsigned int height = static_cast<unsigned int>(elevationMap[0].size());
    float seaLevel = generator.getConfig().seaLevel;
    
    std::vector<uint8_t> data(width * height * 3);
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            float elev = elevationMap[x][y];
            RGB color = getElevationColor(elev, seaLevel);
            
            size_t idx = (y * width + x) * 3;
            data[idx + 0] = color.r;
            data[idx + 1] = color.g;
            data[idx + 2] = color.b;
        }
    }
    
    return writePNG(filename, data.data(), width, height);
}

bool WorldGenPNG::exportTemperature(const ClimateWorldGenerator& generator,
                                    const std::string& filename) {
    const auto& temperatureMap = generator.getTemperatureMap();
    if (temperatureMap.empty()) return false;
    
    unsigned int width = static_cast<unsigned int>(temperatureMap.size());
    unsigned int height = static_cast<unsigned int>(temperatureMap[0].size());
    
    std::vector<uint8_t> data(width * height * 3);
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            float temp = temperatureMap[x][y];
            RGB color = getTemperatureColor(temp);
            
            size_t idx = (y * width + x) * 3;
            data[idx + 0] = color.r;
            data[idx + 1] = color.g;
            data[idx + 2] = color.b;
        }
    }
    
    return writePNG(filename, data.data(), width, height);
}

bool WorldGenPNG::exportMoisture(const ClimateWorldGenerator& generator,
                                 const std::string& filename) {
    const auto& moistureMap = generator.getMoistureMap();
    if (moistureMap.empty()) return false;
    
    unsigned int width = static_cast<unsigned int>(moistureMap.size());
    unsigned int height = static_cast<unsigned int>(moistureMap[0].size());
    
    std::vector<uint8_t> data(width * height * 3);
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            float moisture = moistureMap[x][y];
            RGB color = getMoistureColor(moisture);
            
            size_t idx = (y * width + x) * 3;
            data[idx + 0] = color.r;
            data[idx + 1] = color.g;
            data[idx + 2] = color.b;
        }
    }
    
    return writePNG(filename, data.data(), width, height);
}

bool WorldGenPNG::exportBiomes(const ClimateWorldGenerator& generator,
                               const std::string& filename) {
    const auto& climateMap = generator.getClimateMap();
    if (climateMap.empty()) return false;
    
    unsigned int width = static_cast<unsigned int>(climateMap.size());
    unsigned int height = static_cast<unsigned int>(climateMap[0].size());
    
    std::vector<uint8_t> data(width * height * 3);
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const TileClimate& climate = climateMap[x][y];
            
            // Use blended biome color for smooth transitions
            RGB color = getBlendedBiomeColor(climate.biomeBlend);
            
            // Make rivers/lakes stand out with bright blue
            if (climate.feature == TerrainFeature::RIVER) {
                // Bright cyan-blue for rivers
                float flow = climate.waterLevel;
                color = RGB::lerp(
                    RGB(80, 160, 220),   // Small stream
                    RGB(30, 100, 200),   // Major river
                    flow
                );
            } else if (climate.feature == TerrainFeature::LAKE) {
                color = RGB(50, 140, 210);  // Lake blue
            }
            
            size_t idx = (y * width + x) * 3;
            data[idx + 0] = color.r;
            data[idx + 1] = color.g;
            data[idx + 2] = color.b;
        }
    }
    
    return writePNG(filename, data.data(), width, height);
}

bool WorldGenPNG::exportComposite(const ClimateWorldGenerator& generator,
                                  const std::string& filename) {
    const auto& climateMap = generator.getClimateMap();
    const auto& elevationMap = generator.getElevationMap();
    if (climateMap.empty() || elevationMap.empty()) return false;
    
    unsigned int width = static_cast<unsigned int>(climateMap.size());
    unsigned int height = static_cast<unsigned int>(climateMap[0].size());
    float seaLevel = generator.getConfig().seaLevel;
    
    std::vector<uint8_t> data(width * height * 3);
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const TileClimate& climate = climateMap[x][y];
            float elev = elevationMap[x][y];
            
            // Base color from blended biome for smooth transitions
            RGB color = getBlendedBiomeColor(climate.biomeBlend);
            
            // Override for terrain features - bright, visible water features
            switch (climate.feature) {
                case TerrainFeature::RIVER:
                    // Bright cyan-blue with flow-based intensity
                    // Larger rivers (higher waterLevel) are more saturated blue
                    {
                        float flow = climate.waterLevel;
                        // Interpolate from light blue to deep river blue based on flow
                        color = RGB::lerp(
                            RGB(100, 180, 220),   // Small stream: lighter cyan
                            RGB(30, 100, 200),    // Major river: saturated blue
                            flow
                        );
                    }
                    break;
                case TerrainFeature::LAKE:
                    color = RGB(50, 140, 210);    // Clear lake blue
                    break;
                case TerrainFeature::BEACH:
                    color = RGB(230, 215, 160);   // Sandy beach
                    break;
                case TerrainFeature::MARSH:
                    color = RGB(90, 130, 100);    // Swampy green
                    break;
                default:
                    break;
            }
            
            // Add river bank shading: darken adjacent land tiles
            if (climate.feature != TerrainFeature::RIVER &&
                climate.feature != TerrainFeature::LAKE &&
                elev >= seaLevel) {
                // Check if adjacent to river
                bool nearRiver = false;
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) continue;
                        unsigned int nx = x + dx;
                        unsigned int ny = y + dy;
                        if (nx < width && ny < height) {
                            if (climateMap[nx][ny].feature == TerrainFeature::RIVER ||
                                climateMap[nx][ny].feature == TerrainFeature::LAKE) {
                                nearRiver = true;
                                break;
                            }
                        }
                    }
                    if (nearRiver) break;
                }
                
                // Darken river bank slightly (creates visual border around rivers)
                if (nearRiver) {
                    color.r = static_cast<uint8_t>(color.r * 0.85f);
                    color.g = static_cast<uint8_t>(color.g * 0.85f);
                    color.b = static_cast<uint8_t>(color.b * 0.90f);
                }
            }
            
            // Apply shading based on elevation (hillshading effect)
            float landHeight = (elev >= seaLevel) ?
                (elev - seaLevel) / (1.0f - seaLevel) : 0.0f;
            
            // Simple hillshading: brighter on one side, darker on other
            float dx = 0.0f, dy = 0.0f;
            if (x > 0 && x < width - 1) {
                dx = elevationMap[x+1][y] - elevationMap[x-1][y];
            }
            if (y > 0 && y < height - 1) {
                dy = elevationMap[x][y+1] - elevationMap[x][y-1];
            }
            
            // Combine x and y slopes for more realistic shading
            float shade = 0.85f + dx * 1.5f - dy * 0.5f;
            shade = (shade < 0.5f) ? 0.5f : ((shade > 1.3f) ? 1.3f : shade);
            
            // Also brighten high elevations slightly
            shade += landHeight * 0.15f;
            
            // Only apply shading to land features (not water or special features)
            if (climate.feature != TerrainFeature::RIVER &&
                climate.feature != TerrainFeature::LAKE &&
                elev >= seaLevel) {
                color.r = static_cast<uint8_t>((color.r * shade < 255) ? color.r * shade : 255);
                color.g = static_cast<uint8_t>((color.g * shade < 255) ? color.g * shade : 255);
                color.b = static_cast<uint8_t>((color.b * shade < 255) ? color.b * shade : 255);
            }
            
            size_t idx = (y * width + x) * 3;
            data[idx + 0] = color.r;
            data[idx + 1] = color.g;
            data[idx + 2] = color.b;
        }
    }
    
    return writePNG(filename, data.data(), width, height);
}

} // namespace EcoSim
