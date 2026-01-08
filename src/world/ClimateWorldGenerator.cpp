/**
 * @file ClimateWorldGenerator.cpp
 * @brief Implementation of climate-based world generation
 */

#include "../../include/world/ClimateWorldGenerator.hpp"
#include "../../include/colorPairs.hpp"

#include <cmath>
#include <algorithm>
#include <limits>

namespace EcoSim {

//=============================================================================
// Static Biome Properties
//=============================================================================

static const BiomeProperties BIOME_PROPERTIES[] = {
    // OCEAN_DEEP
    {"Deep Ocean", 0.0f, 0.0f, 0.0f, 2.0f, false, false, TerrainType::DEEP_WATER, '~'},
    // OCEAN_SHALLOW
    {"Shallow Ocean", 0.0f, 0.0f, 0.1f, 1.5f, false, false, TerrainType::WATER, '~'},
    // OCEAN_COAST
    {"Coastal Waters", 0.0f, 0.0f, 0.2f, 1.3f, false, false, TerrainType::SHALLOW_WATER, '~'},
    // FRESHWATER
    {"Freshwater", 0.0f, 0.0f, 0.3f, 1.5f, false, false, TerrainType::WATER, '~'},
    
    // ICE_SHEET
    {"Ice Sheet", -15.0f, 0.1f, 0.0f, 2.0f, false, false, TerrainType::SNOW, '#'},
    // TUNDRA
    {"Tundra", -8.0f, 0.5f, 0.2f, 1.4f, true, false, TerrainType::SNOW, ','},
    // TAIGA
    {"Taiga", -3.0f, 0.4f, 0.5f, 1.3f, true, true, TerrainType::FOREST, '"'},
    
    // BOREAL_FOREST
    {"Boreal Forest", -2.0f, 0.4f, 0.6f, 1.3f, true, true, TerrainType::FOREST, '"'},
    // TEMPERATE_RAINFOREST
    {"Temperate Rainforest", 2.0f, 0.3f, 0.9f, 1.5f, true, true, TerrainType::FOREST, '"'},
    // TEMPERATE_FOREST
    {"Temperate Forest", 0.0f, 0.5f, 0.7f, 1.2f, true, true, TerrainType::FOREST, '"'},
    // TEMPERATE_GRASSLAND
    {"Temperate Grassland", 1.0f, 0.7f, 0.5f, 1.0f, true, false, TerrainType::PLAINS, '.'},
    
    // TROPICAL_RAINFOREST
    {"Tropical Rainforest", 2.0f, 0.3f, 1.0f, 1.5f, true, true, TerrainType::FOREST, '"'},
    // TROPICAL_SEASONAL_FOREST
    {"Tropical Seasonal Forest", 2.0f, 0.5f, 0.8f, 1.3f, true, true, TerrainType::FOREST, '"'},
    // SAVANNA
    {"Savanna", 3.0f, 0.8f, 0.4f, 1.0f, true, false, TerrainType::SAVANNA, '.'},
    
    // DESERT_HOT
    {"Hot Desert", 8.0f, 2.0f, 0.05f, 1.2f, false, false, TerrainType::DESERT_SAND, '.'},
    // DESERT_COLD
    {"Cold Desert", -5.0f, 1.5f, 0.05f, 1.2f, false, false, TerrainType::DESERT_SAND, '.'},
    // STEPPE
    {"Steppe", 2.0f, 1.0f, 0.2f, 1.0f, true, false, TerrainType::PLAINS, ','},
    // SHRUBLAND
    {"Shrubland", 1.0f, 0.9f, 0.3f, 1.1f, true, false, TerrainType::SHORT_GRASS, ','},
    
    // ALPINE_MEADOW
    {"Alpine Meadow", -2.0f, 0.6f, 0.4f, 1.3f, true, false, TerrainType::SHORT_GRASS, ','},
    // ALPINE_TUNDRA
    {"Alpine Tundra", -8.0f, 0.5f, 0.2f, 1.5f, true, false, TerrainType::MOUNTAIN, '.'},
    // MOUNTAIN_BARE
    {"Bare Mountain", -5.0f, 0.8f, 0.0f, 1.8f, false, false, TerrainType::MOUNTAIN_2, '.'},
    // GLACIER
    {"Glacier", -20.0f, 0.1f, 0.0f, 2.5f, false, false, TerrainType::PEAKS, '^'}
};

//=============================================================================
// BiomeBlend Implementation
//=============================================================================

void BiomeBlend::addContribution(Biome biome, float weight) {
    if (weight <= 0.0f) return;
    
    // Check if this biome is already in the blend
    for (int i = 0; i < count; ++i) {
        if (contributions[i].biome == biome) {
            contributions[i].weight += weight;
            return;
        }
    }
    
    // Add new contribution if we have space
    if (count < MAX_BIOME_BLEND) {
        contributions[count] = {biome, weight};
        ++count;
    } else {
        // Replace smallest contribution if new one is larger
        int minIdx = 0;
        float minWeight = contributions[0].weight;
        for (int i = 1; i < MAX_BIOME_BLEND; ++i) {
            if (contributions[i].weight < minWeight) {
                minWeight = contributions[i].weight;
                minIdx = i;
            }
        }
        if (weight > minWeight) {
            contributions[minIdx] = {biome, weight};
        }
    }
}

void BiomeBlend::normalize() {
    float total = 0.0f;
    for (int i = 0; i < count; ++i) {
        total += contributions[i].weight;
    }
    
    if (total > 0.0f) {
        for (int i = 0; i < count; ++i) {
            contributions[i].weight /= total;
        }
    }
    
    // Sort by weight (highest first) so primary() returns dominant biome
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (contributions[j].weight > contributions[i].weight) {
                std::swap(contributions[i], contributions[j]);
            }
        }
    }
}

float BiomeBlend::getBlendedVegetationDensity() const {
    float result = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        result += props.vegetationDensity * contributions[i].weight;
    }
    return result;
}

float BiomeBlend::getBlendedMovementCost() const {
    float result = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        result += props.movementCost * contributions[i].weight;
    }
    return result;
}

float BiomeBlend::getBlendedEvaporationRate() const {
    float result = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        result += props.evaporationRate * contributions[i].weight;
    }
    return result;
}

float BiomeBlend::getBlendedTemperatureModifier() const {
    float result = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        result += props.temperatureModifier * contributions[i].weight;
    }
    return result;
}

bool BiomeBlend::canSupportPlants() const {
    // Can support plants if dominant biome supports them, or if significant secondary biomes do
    float plantSupport = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        if (props.supportsPlants) {
            plantSupport += contributions[i].weight;
        }
    }
    return plantSupport >= 0.3f;  // At least 30% plant-supporting biomes
}

bool BiomeBlend::canSupportTrees() const {
    // Can support trees if dominant biome supports them, or if significant secondary biomes do
    float treeSupport = 0.0f;
    for (int i = 0; i < count; ++i) {
        const BiomeProperties& props = ClimateWorldGenerator::getBiomeProperties(contributions[i].biome);
        if (props.supportsTrees) {
            treeSupport += contributions[i].weight;
        }
    }
    return treeSupport >= 0.4f;  // At least 40% tree-supporting biomes
}

// Whittaker-style biome lookup table
// Rows: Temperature zones (FROZEN, COLD, COOL, TEMPERATE, WARM, HOT)
// Columns: Moisture zones (ARID, DRY, MODERATE, WET, SATURATED)
static const Biome BIOME_TABLE[6][5] = {
    // FROZEN (-20 to -10°C)
    {Biome::ICE_SHEET, Biome::ICE_SHEET, Biome::TUNDRA, Biome::TUNDRA, Biome::GLACIER},
    
    // COLD (-10 to 0°C)
    {Biome::DESERT_COLD, Biome::TUNDRA, Biome::TAIGA, Biome::TAIGA, Biome::BOREAL_FOREST},
    
    // COOL (0 to 10°C)
    {Biome::STEPPE, Biome::SHRUBLAND, Biome::TEMPERATE_FOREST, Biome::TEMPERATE_FOREST, Biome::TEMPERATE_RAINFOREST},
    
    // TEMPERATE (10 to 20°C)
    {Biome::STEPPE, Biome::TEMPERATE_GRASSLAND, Biome::TEMPERATE_FOREST, Biome::TEMPERATE_FOREST, Biome::TEMPERATE_RAINFOREST},
    
    // WARM (20 to 30°C)
    {Biome::DESERT_HOT, Biome::SAVANNA, Biome::SAVANNA, Biome::TROPICAL_SEASONAL_FOREST, Biome::TROPICAL_RAINFOREST},
    
    // HOT (30°C+)
    {Biome::DESERT_HOT, Biome::DESERT_HOT, Biome::SAVANNA, Biome::TROPICAL_SEASONAL_FOREST, Biome::TROPICAL_RAINFOREST}
};

//=============================================================================
// Constructor
//=============================================================================

ClimateWorldGenerator::ClimateWorldGenerator() {
    _config = ClimateGeneratorConfig{};
}

ClimateWorldGenerator::ClimateWorldGenerator(const ClimateGeneratorConfig& config)
    : _config(config) {
}

void ClimateWorldGenerator::setConfig(const ClimateGeneratorConfig& config) {
    _config = config;
}

//=============================================================================
// Static Utilities
//=============================================================================

const BiomeProperties& ClimateWorldGenerator::getBiomeProperties(Biome biome) {
    return BIOME_PROPERTIES[static_cast<int>(biome)];
}

TerrainType ClimateWorldGenerator::biomeToTerrainType(Biome biome) {
    return BIOME_PROPERTIES[static_cast<int>(biome)].terrainType;
}

const char* ClimateWorldGenerator::getBiomeName(Biome biome) {
    return BIOME_PROPERTIES[static_cast<int>(biome)].name;
}

//=============================================================================
// Main Generation
//=============================================================================

void ClimateWorldGenerator::generate(WorldGrid& grid) {
    generate(grid, _config.seed);
}

void ClimateWorldGenerator::generate(WorldGrid& grid, unsigned int seed) {
    _config.seed = seed;
    _rng.seed(seed);
    
    // Resize grid if needed
    grid.resize(_config.width, _config.height);
    
    // Run generation pipeline
    initializeMaps();
    generatePlateRidges();          // Generate tectonic plate boundaries first
    calculateRidgeDistanceMap();    // Precompute distance to ridges
    generateContinentMap();
    generateElevationMap();
    
    // Remove unrealistic inland seas before climate simulation
    if (_config.removeInlandSeas) {
        removeInlandSeas();
    }
    
    calculateTemperature();
    calculateWaterDistance();
    calculateMoisture();
    determineBiomes();
    
    if (_config.generateRivers) {
        generateRivers();
    }
    
    applyToGrid(grid);
}

void ClimateWorldGenerator::initializeMaps() {
    unsigned int w = _config.width;
    unsigned int h = _config.height;
    
    _continentMap.assign(w, std::vector<float>(h, 0.0f));
    _elevationMap.assign(w, std::vector<float>(h, 0.0f));
    _temperatureMap.assign(w, std::vector<float>(h, 0.0f));
    _moistureMap.assign(w, std::vector<float>(h, 0.0f));
    _waterDistanceMap.assign(w, std::vector<float>(h, 0.0f));
    _climateMap.assign(w, std::vector<TileClimate>(h));
}

//=============================================================================
// Phase 1: Continental Shape with Fractal Coastlines
//=============================================================================

void ClimateWorldGenerator::generateContinentMap() {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            float noise = continentNoise(x, y);
            
            // Apply island mode falloff
            if (_config.isIsland) {
                float edgeDist = distanceToEdge(x, y);
                // Use a blend approach: boost center, sink edges
                float centerBoost = smoothstep(0.0f, _config.islandFalloff, edgeDist);
                float edgeSink = 1.0f - smoothstep(_config.islandFalloff * 0.5f, 0.0f, edgeDist);
                
                // Center has noise boosted, edges sink below sea level
                noise = noise * 0.7f + centerBoost * 0.5f - edgeSink * 0.3f;
            }
            
            // Add fractal coastline detail
            // This creates bays, peninsulas, and jagged edges
            float coastlineNoise = coastlineDetailNoise(x, y);
            
            // Apply coastline detail more strongly near the land/sea boundary
            float distFromSeaLevel = std::abs(noise - _config.seaLevel);
            float coastlineInfluence = 1.0f - clampValue(distFromSeaLevel * 5.0f, 0.0f, 1.0f);
            
            // Perturb the elevation near coastlines
            noise += coastlineNoise * 0.15f * coastlineInfluence;
            
            _continentMap[x][y] = clampValue(noise, 0.0f, 1.0f);
        }
    }
}

float ClimateWorldGenerator::continentNoise(int x, int y) const {
    float freq = _config.continentFrequency;
    float seed = static_cast<float>(_config.seed);
    
    // Base continent noise
    float noise = SimplexNoise::noise(
        x * freq + seed,
        y * freq + seed
    );
    
    // Add octaves for coastline detail
    float amplitude = 0.5f;
    float totalAmp = 1.0f;
    
    for (int i = 1; i < _config.continentOctaves; ++i) {
        freq *= 2.0f;
        noise += amplitude * SimplexNoise::noise(
            x * freq + seed,
            y * freq + seed
        );
        totalAmp += amplitude;
        amplitude *= 0.5f;
    }
    
    // Normalize to 0-1 range
    noise = (noise / totalAmp + 1.0f) * 0.5f;
    return noise;
}

float ClimateWorldGenerator::coastlineDetailNoise(int x, int y) const {
    // High-frequency fractal noise for coastline detail
    // Creates bays, peninsulas, capes, and irregular shorelines
    
    float seed = static_cast<float>(_config.seed) * 5.0f;
    float noise = 0.0f;
    float amplitude = 1.0f;
    float totalAmp = 0.0f;
    
    // Use higher frequencies than continent noise for fine coastal detail
    float freq = _config.continentFrequency * 8.0f;
    
    // Multiple octaves of noise for multi-scale coastline features
    // Large bays/peninsulas + medium irregularities + fine jagged edges
    for (int octave = 0; octave < 4; ++octave) {
        noise += amplitude * SimplexNoise::noise(
            x * freq + seed,
            y * freq + seed + octave * 100.0f
        );
        totalAmp += amplitude;
        freq *= 2.2f;        // Slightly more than 2 for irregular patterns
        amplitude *= 0.45f;  // Slower falloff keeps high-freq detail visible
    }
    
    // Normalize to roughly -1 to 1
    noise = noise / totalAmp;
    
    // Add some warping for more organic coastlines
    float warpX = SimplexNoise::noise(x * 0.008f + seed * 2.0f, y * 0.008f) * 8.0f;
    float warpY = SimplexNoise::noise(x * 0.008f, y * 0.008f + seed * 2.0f) * 8.0f;
    
    float warpedNoise = SimplexNoise::noise(
        (x + warpX) * _config.continentFrequency * 6.0f + seed,
        (y + warpY) * _config.continentFrequency * 6.0f + seed
    );
    
    // Combine regular and warped noise
    return noise * 0.7f + warpedNoise * 0.3f;
}

//=============================================================================
// Tectonic Plate Ridge Generation
//=============================================================================

void ClimateWorldGenerator::generatePlateRidges() {
    _plateRidges.clear();
    
    // Generate curved plate boundaries that span the map
    // These represent subduction zones and continental collision zones
    std::uniform_real_distribution<float> posDist(0.15f, 0.85f);
    std::uniform_real_distribution<float> curveDist(-0.3f, 0.3f);
    std::uniform_real_distribution<float> strengthDist(0.6f, 1.0f);
    std::uniform_int_distribution<int> orientDist(0, 1);
    
    for (int i = 0; i < _config.numPlateRidges; ++i) {
        PlateRidge ridge;
        
        // Alternate between roughly horizontal and vertical ridges
        // with randomized positions and curvature
        if (orientDist(_rng) == 0 || i < 2) {
            // Roughly vertical ridge (N-S mountain range like Rockies/Andes)
            ridge.startX = posDist(_rng);
            ridge.startY = 0.05f + curveDist(_rng) * 0.3f;
            ridge.endX = ridge.startX + curveDist(_rng) * 0.4f;
            ridge.endY = 0.95f + curveDist(_rng) * 0.3f;
            
            // Control point creates the curve
            ridge.controlX = (ridge.startX + ridge.endX) / 2.0f + curveDist(_rng);
            ridge.controlY = posDist(_rng);
        } else {
            // Roughly horizontal ridge (E-W mountain range like Alps/Himalayas)
            ridge.startX = 0.05f + curveDist(_rng) * 0.3f;
            ridge.startY = posDist(_rng);
            ridge.endX = 0.95f + curveDist(_rng) * 0.3f;
            ridge.endY = ridge.startY + curveDist(_rng) * 0.4f;
            
            ridge.controlX = posDist(_rng);
            ridge.controlY = (ridge.startY + ridge.endY) / 2.0f + curveDist(_rng);
        }
        
        // Clamp positions to valid range
        ridge.startX = clampValue(ridge.startX, 0.0f, 1.0f);
        ridge.startY = clampValue(ridge.startY, 0.0f, 1.0f);
        ridge.endX = clampValue(ridge.endX, 0.0f, 1.0f);
        ridge.endY = clampValue(ridge.endY, 0.0f, 1.0f);
        ridge.controlX = clampValue(ridge.controlX, 0.0f, 1.0f);
        ridge.controlY = clampValue(ridge.controlY, 0.0f, 1.0f);
        
        ridge.strength = strengthDist(_rng);
        
        _plateRidges.push_back(ridge);
    }
}

void ClimateWorldGenerator::calculateRidgeDistanceMap() {
    _ridgeDistanceMap.assign(_config.width, std::vector<float>(_config.height,
        std::numeric_limits<float>::max()));
    
    // For each tile, find minimum distance to any ridge
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            float px = static_cast<float>(x) / _config.width;
            float py = static_cast<float>(y) / _config.height;
            
            float minDist = std::numeric_limits<float>::max();
            
            for (const auto& ridge : _plateRidges) {
                float dist = distanceToRidge(px, py, ridge);
                // Weight by ridge strength
                dist = dist / ridge.strength;
                minDist = std::min(minDist, dist);
            }
            
            // Convert back to pixel units for easier threshold comparison
            _ridgeDistanceMap[x][y] = minDist * std::max(_config.width, _config.height);
        }
    }
}

float ClimateWorldGenerator::distanceToRidge(float px, float py, const PlateRidge& ridge) const {
    // Find closest point on quadratic Bezier curve
    // Sample the curve at multiple points and find minimum distance
    float minDist = std::numeric_limits<float>::max();
    
    const int samples = 50;
    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i) / samples;
        
        // Quadratic Bezier: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
        float bx = bezierPoint(t, ridge.startX, ridge.controlX, ridge.endX);
        float by = bezierPoint(t, ridge.startY, ridge.controlY, ridge.endY);
        
        float dx = px - bx;
        float dy = py - by;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        minDist = std::min(minDist, dist);
    }
    
    return minDist;
}

float ClimateWorldGenerator::bezierPoint(float t, float p0, float p1, float p2) const {
    float oneMinusT = 1.0f - t;
    return oneMinusT * oneMinusT * p0 + 2.0f * oneMinusT * t * p1 + t * t * p2;
}

float ClimateWorldGenerator::ridgeInfluence(int x, int y) const {
    float dist = _ridgeDistanceMap[x][y];
    
    // Core mountain zone - steep mountains
    if (dist < _config.ridgeWidth) {
        float t = dist / _config.ridgeWidth;
        // Sharp falloff near ridge core
        return 1.0f - t * t;
    }
    
    // Foothills zone - gradual elevation boost
    if (dist < _config.ridgeWidth + _config.foothillsWidth) {
        float t = (dist - _config.ridgeWidth) / _config.foothillsWidth;
        // Smooth falloff for foothills
        return (1.0f - t) * (1.0f - t) * 0.4f;
    }
    
    return 0.0f;
}

float ClimateWorldGenerator::ridgedMultifractal(float x, float y, int octaves,
                                                  float lacunarity, float gain) const {
    float seed = static_cast<float>(_config.seed) * 3.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;
    float result = 0.0f;
    
    // Previous octave's value for weighting (heterogeneous terrain)
    float weight = 1.0f;
    
    for (int i = 0; i < octaves; ++i) {
        // Sample noise and create ridge effect
        float noise = SimplexNoise::noise(x * frequency + seed + i * 13.7f,
                                          y * frequency + seed + i * 7.3f);
        
        // Ridge formula: 1 - abs(noise), squared for sharper ridges
        float ridge = 1.0f - std::abs(noise);
        ridge = ridge * ridge;
        
        // Weight by previous octave for terrain variation
        ridge *= weight;
        weight = clampValue(ridge * 2.0f, 0.0f, 1.0f);
        
        result += ridge * amplitude;
        totalAmplitude += amplitude;
        
        frequency *= lacunarity;
        amplitude *= gain;
    }
    
    return result / totalAmplitude;
}

//=============================================================================
// Phase 2: Elevation Detail with Tectonic Mountain Ranges
//=============================================================================

void ClimateWorldGenerator::generateElevationMap() {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            float continent = _continentMap[x][y];
            
            if (continent < _config.seaLevel) {
                // Ocean floor - gentle variation with oceanic ridges
                float oceanFloor = continent * 0.3f;
                
                // Oceanic ridges (mid-ocean spreading centers)
                float oceanRidge = ridgeInfluence(x, y) * 0.15f;
                _elevationMap[x][y] = oceanFloor + oceanRidge;
            } else {
                // Land elevation with detail
                float elevation = continent;
                
                // === Tectonic Mountain Ranges ===
                // Mountains concentrate along plate boundaries
                float tectonicInfluence = ridgeInfluence(x, y);
                
                // Multi-octave ridged noise for mountain detail
                float ridgeNoise = ridgedMultifractal(
                    x * _config.ridgeFrequency,
                    y * _config.ridgeFrequency,
                    _config.ridgeOctaves,
                    _config.ridgeLacunarity,
                    _config.ridgeGain
                );
                
                // Combine tectonic influence with ridge noise
                // Near plate boundaries: strong mountains
                // Far from boundaries: gentler terrain
                float mountainBoost = tectonicInfluence * _config.ridgeStrength;
                
                // Scale mountain formation by how far above sea level (coastal mountains)
                float landHeight = (continent - _config.seaLevel) / (1.0f - _config.seaLevel);
                
                // Mountain clustering noise - creates distinct peaks within ranges
                float clusterNoise = SimplexNoise::noise(
                    x * _config.mountainClusterFreq + _config.seed * 4.0f,
                    y * _config.mountainClusterFreq + _config.seed * 4.0f
                );
                clusterNoise = clusterNoise * 0.5f + 0.5f;  // 0 to 1
                
                // Combine: tectonic position + ridge shape + clustering
                float mountainFactor = mountainBoost * (0.5f + 0.5f * ridgeNoise) * (0.7f + 0.3f * clusterNoise);
                elevation += mountainFactor * landHeight;
                
                // Add random ridged noise for mountains outside tectonic zones
                // (volcanic mountains, erosion remnants, etc.)
                float randomRidges = ridgeNoise * 0.25f * landHeight * (1.0f - tectonicInfluence * 0.7f);
                elevation += randomRidges;
                
                // Local terrain variation (small hills and valleys)
                float detail = SimplexNoise::noise(
                    x * _config.elevationFrequency + _config.seed * 2.0f,
                    y * _config.elevationFrequency + _config.seed * 2.0f
                );
                elevation += detail * 0.08f;
                
                // Higher-frequency detail for rugged terrain near mountains
                if (mountainFactor > 0.1f) {
                    float rugged = SimplexNoise::noise(
                        x * _config.elevationFrequency * 3.0f + _config.seed * 5.0f,
                        y * _config.elevationFrequency * 3.0f + _config.seed * 5.0f
                    );
                    elevation += rugged * 0.05f * mountainFactor;
                }
                
                _elevationMap[x][y] = clampValue(elevation, 0.0f, 1.0f);
            }
        }
    }
}

//=============================================================================
// Inland Sea Removal
//=============================================================================

void ClimateWorldGenerator::removeInlandSeas() {
    // Step 1: Create ocean mask - mark all water tiles connected to map edges
    std::vector<std::vector<bool>> oceanMask(_config.width,
        std::vector<bool>(_config.height, false));
    
    floodFillOcean(oceanMask);
    
    // Step 2: Find all water tiles NOT connected to ocean (inland seas)
    // Group them into connected components and measure their size
    std::vector<std::vector<int>> componentMap(_config.width,
        std::vector<int>(_config.height, -1));
    int numComponents = 0;
    std::vector<int> componentSizes;
    
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            // If this is water but not ocean, it's an inland sea
            if (_elevationMap[x][y] < _config.seaLevel && !oceanMask[x][y]) {
                if (componentMap[x][y] == -1) {
                    // Start a new component - BFS to find all connected water
                    std::queue<std::pair<int, int>> frontier;
                    frontier.push({x, y});
                    componentMap[x][y] = numComponents;
                    int size = 0;
                    
                    while (!frontier.empty()) {
                        auto [cx, cy] = frontier.front();
                        frontier.pop();
                        ++size;
                        
                        // Check all 8 neighbors
                        for (auto [dx, dy] : NEIGHBORS_8) {
                            int nx = cx + dx;
                            int ny = cy + dy;
                            if (!inBounds(nx, ny)) continue;
                            
                            // If neighbor is water, not ocean, and not yet visited
                            if (_elevationMap[nx][ny] < _config.seaLevel &&
                                !oceanMask[nx][ny] &&
                                componentMap[nx][ny] == -1) {
                                componentMap[nx][ny] = numComponents;
                                frontier.push({nx, ny});
                            }
                        }
                    }
                    
                    componentSizes.push_back(size);
                    ++numComponents;
                }
            }
        }
    }
    
    // Step 3: Fill in inland seas that are too small to be realistic
    // Large ones become lakes, small ones become land
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            int comp = componentMap[x][y];
            if (comp >= 0) {
                int size = componentSizes[comp];
                
                if (size < _config.minInlandSeaSize) {
                    // Small inland sea - fill with land
                    // Use noise to create natural-looking fill elevation
                    float baseElev = _config.inlandSeaFillElevation;
                    float noise = SimplexNoise::noise(
                        x * 0.02f + _config.seed * 8.0f,
                        y * 0.02f + _config.seed * 8.0f
                    ) * 0.05f;
                    
                    _elevationMap[x][y] = baseElev + noise;
                    
                    // Also update continent map for consistency
                    _continentMap[x][y] = baseElev + noise;
                }
                // Larger inland seas are kept as lakes (freshwater bodies)
            }
        }
    }
}

void ClimateWorldGenerator::floodFillOcean(std::vector<std::vector<bool>>& oceanMask) {
    std::queue<std::pair<int, int>> frontier;
    
    // Start flood fill from all water tiles on map edges
    // Top and bottom edges
    for (unsigned int x = 0; x < _config.width; ++x) {
        if (_elevationMap[x][0] < _config.seaLevel) {
            oceanMask[x][0] = true;
            frontier.push({x, 0});
        }
        unsigned int lastY = _config.height - 1;
        if (_elevationMap[x][lastY] < _config.seaLevel) {
            oceanMask[x][lastY] = true;
            frontier.push({x, static_cast<int>(lastY)});
        }
    }
    
    // Left and right edges
    for (unsigned int y = 0; y < _config.height; ++y) {
        if (_elevationMap[0][y] < _config.seaLevel) {
            oceanMask[0][y] = true;
            frontier.push({0, static_cast<int>(y)});
        }
        unsigned int lastX = _config.width - 1;
        if (_elevationMap[lastX][y] < _config.seaLevel) {
            oceanMask[lastX][y] = true;
            frontier.push({static_cast<int>(lastX), static_cast<int>(y)});
        }
    }
    
    // BFS to fill all connected ocean
    while (!frontier.empty()) {
        auto [cx, cy] = frontier.front();
        frontier.pop();
        
        for (auto [dx, dy] : NEIGHBORS_8) {
            int nx = cx + dx;
            int ny = cy + dy;
            if (!inBounds(nx, ny)) continue;
            
            // If neighbor is water and not yet marked as ocean
            if (_elevationMap[nx][ny] < _config.seaLevel && !oceanMask[nx][ny]) {
                oceanMask[nx][ny] = true;
                frontier.push({nx, ny});
            }
        }
    }
}

//=============================================================================
// Elevation Noise Functions
//=============================================================================

float ClimateWorldGenerator::elevationNoise(int x, int y) const {
    float freq = _config.elevationFrequency;
    float seed = static_cast<float>(_config.seed) * 1.5f;
    
    float noise = SimplexNoise::noise(x * freq + seed, y * freq + seed);
    
    float amplitude = 0.5f;
    float totalAmp = 1.0f;
    
    for (int i = 1; i < _config.elevationOctaves; ++i) {
        freq *= 2.0f;
        noise += amplitude * SimplexNoise::noise(x * freq + seed, y * freq + seed);
        totalAmp += amplitude;
        amplitude *= 0.5f;
    }
    
    return (noise / totalAmp + 1.0f) * 0.5f;
}

float ClimateWorldGenerator::ridgedNoise(float x, float y) const {
    float seed = static_cast<float>(_config.seed) * 3.0f;
    float noise = SimplexNoise::noise(x + seed, y + seed);
    
    // Invert absolute value for ridge effect
    noise = 1.0f - std::abs(noise);
    // Square for sharper ridges
    noise = noise * noise;
    
    return noise;
}

//=============================================================================
// Phase 3: Temperature Calculation
//=============================================================================

void ClimateWorldGenerator::calculateTemperature() {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            _temperatureMap[x][y] = calculateTileTemperature(x, y);
        }
    }
}

float ClimateWorldGenerator::calculateTileTemperature(int x, int y) const {
    // 1. Base temperature from latitude
    // y=0 is one pole, y=height is the other
    float latitude = static_cast<float>(y) / _config.height;
    
    // Distance from equator (0 at equator, 1 at poles)
    float distFromEquator = std::abs(latitude - _config.equatorPosition) * 2.0f;
    distFromEquator = std::min(distFromEquator, 1.0f);
    
    // Cosine distribution for temperature
    float latitudeFactor = std::cos(distFromEquator * M_PI * 0.5f);
    
    // Base temperature: hot at equator, cold at poles
    float baseTemp = _config.baseTemperature + 
                     (_config.temperatureRange * 0.5f) * (latitudeFactor * 2.0f - 1.0f);
    
    // 2. Elevation cooling (lapse rate: typically 6.5°C per 1000m)
    float elevation = _elevationMap[x][y];
    float elevationMeters = elevation * _config.maxElevationMeters;
    float elevationCooling = -(_config.lapseRate / 1000.0f) * elevationMeters;
    
    // 3. Local climate variation noise
    float tempNoise = SimplexNoise::noise(
        x * _config.temperatureNoiseScale + _config.seed * 2.0f,
        y * _config.temperatureNoiseScale + _config.seed * 2.0f
    ) * 5.0f;  // ±5°C variation
    
    return baseTemp + elevationCooling + tempNoise;
}

//=============================================================================
// Phase 4: Water Distance Calculation
//=============================================================================

void ClimateWorldGenerator::calculateWaterDistance() {
    // BFS from all water tiles to calculate distance
    std::queue<std::pair<int, int>> frontier;
    
    // Initialize: water tiles have distance 0, land starts at max
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            if (_elevationMap[x][y] < _config.seaLevel) {
                _waterDistanceMap[x][y] = 0.0f;
                frontier.push({x, y});
            } else {
                _waterDistanceMap[x][y] = std::numeric_limits<float>::max();
            }
        }
    }
    
    // BFS to propagate distances
    while (!frontier.empty()) {
        auto [cx, cy] = frontier.front();
        frontier.pop();
        
        float currentDist = _waterDistanceMap[cx][cy];
        
        for (auto [dx, dy] : NEIGHBORS_8) {
            int nx = cx + dx;
            int ny = cy + dy;
            
            if (!inBounds(nx, ny)) continue;
            
            // Distance to diagonal neighbors is sqrt(2), orthogonal is 1
            float neighborDist = (dx != 0 && dy != 0) ? 1.414f : 1.0f;
            float newDist = currentDist + neighborDist;
            
            if (newDist < _waterDistanceMap[nx][ny]) {
                _waterDistanceMap[nx][ny] = newDist;
                frontier.push({nx, ny});
            }
        }
    }
}

//=============================================================================
// Phase 5: Moisture Calculation
//=============================================================================

void ClimateWorldGenerator::calculateMoisture() {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            _moistureMap[x][y] = calculateTileMoisture(x, y);
        }
    }
}

float ClimateWorldGenerator::calculateTileMoisture(int x, int y) const {
    // Water tiles have maximum moisture
    if (_elevationMap[x][y] < _config.seaLevel) {
        return 1.0f;
    }
    
    // 1. Base moisture from distance to water (exponential decay)
    float coastDist = _waterDistanceMap[x][y];
    float baseMoisture = std::exp(-coastDist / _config.coastalMoistureDecay);
    
    // 2. Prevailing wind patterns based on latitude
    // Real wind patterns: Trade winds (east), Westerlies (west), Polar easterlies
    float windMoisture = calculateWindMoisture(x, y);
    
    // 3. Rain shadow effect
    float rainShadow = calculateRainShadow(x, y);
    
    // 4. Temperature effect - hot air can hold more moisture, but also loses more
    float temp = _temperatureMap[x][y];
    float tempFactor = 1.0f;
    if (temp > 20.0f) {
        tempFactor = 1.0f + 0.02f * (temp - 20.0f);  // Boost for tropics
    } else if (temp < 0.0f) {
        tempFactor = 0.7f;  // Cold air holds less moisture
    }
    
    // 5. Local moisture variation noise
    float moistNoise = SimplexNoise::noise(
        x * _config.moistureNoiseScale + _config.seed * 3.0f,
        y * _config.moistureNoiseScale + _config.seed * 3.0f
    ) * 0.15f;  // ±0.15 variation
    
    float moisture = baseMoisture * windMoisture * rainShadow * tempFactor * _config.moistureScale + moistNoise;
    return clampValue(moisture, 0.0f, 1.0f);
}

float ClimateWorldGenerator::calculateWindMoisture(int x, int y) const {
    // Calculate latitude-based wind patterns
    float latitude = static_cast<float>(y) / _config.height;
    float distFromEquator = std::abs(latitude - _config.equatorPosition);
    
    // Wind direction varies by latitude:
    // 0-30° from equator: Trade winds (from east, so eastern coasts wetter)
    // 30-60° from equator: Westerlies (from west, so western coasts wetter)
    // 60-90° from equator: Polar easterlies (from east)
    
    float xNorm = static_cast<float>(x) / _config.width;
    float windFactor = 1.0f;
    
    if (distFromEquator < 0.3f) {
        // Trade wind zone - eastern coasts get moisture
        windFactor = 0.7f + 0.6f * xNorm;  // East wetter
    } else if (distFromEquator < 0.6f) {
        // Westerly zone - western coasts get moisture
        windFactor = 1.3f - 0.6f * xNorm;  // West wetter
    } else {
        // Polar easterlies - slight eastern boost
        windFactor = 0.8f + 0.4f * xNorm;
    }
    
    return windFactor;
}

float ClimateWorldGenerator::calculateRainShadow(int x, int y) const {
    // Look upwind (west) for mountains that block moisture
    float maxUpwindElevation = 0.0f;
    float currentElev = _elevationMap[x][y];
    
    for (int dx = 1; dx < _config.rainShadowDistance && x - dx >= 0; ++dx) {
        float upwindElev = _elevationMap[x - dx][y];
        maxUpwindElevation = std::max(maxUpwindElevation, upwindElev);
    }
    
    // If there's a significant mountain upwind, reduce moisture
    if (maxUpwindElevation > currentElev + 0.1f) {
        float shadowStrength = (maxUpwindElevation - currentElev) / 0.3f;
        shadowStrength = std::min(shadowStrength, 1.0f);
        return 1.0f - shadowStrength * _config.rainShadowStrength;
    }
    
    return 1.0f;
}

//=============================================================================
// Phase 6: Biome Determination
//=============================================================================

void ClimateWorldGenerator::determineBiomes() {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            TileClimate& climate = _climateMap[x][y];
            
            climate.elevation = _elevationMap[x][y];
            climate.temperature = _temperatureMap[x][y];
            climate.moisture = _moistureMap[x][y];
            climate.biomeBlend = lookupBiomeBlend(climate.temperature, climate.moisture, climate.elevation);
            
            // Assign terrain features based on local terrain analysis
            climate.feature = determineTerrainFeature(x, y, climate);
        }
    }
}

TerrainFeature ClimateWorldGenerator::determineTerrainFeature(
    unsigned int x, unsigned int y, const TileClimate& climate) const
{
    // Water features
    if (climate.elevation < _config.seaLevel) {
        return TerrainFeature::NONE;
    }
    
    // Beach detection - land adjacent to ocean
    if (isCoastalTile(x, y)) {
        // Hot coastal areas can have beaches, cold can have rocky coasts
        if (climate.temperature > 10.0f && climate.moisture > 0.3f) {
            return TerrainFeature::BEACH;
        }
    }
    
    // Marsh detection - low wet areas
    if (climate.elevation < _config.seaLevel + 0.1f && climate.moisture > 0.7f) {
        return TerrainFeature::MARSH;
    }
    
    // Calculate local slope/roughness for terrain classification
    float localSlope = calculateLocalSlope(x, y);
    
    if (climate.elevation > 0.85f || localSlope > 0.15f) {
        return TerrainFeature::MOUNTAINS;
    }
    if (climate.elevation > 0.65f || localSlope > 0.08f) {
        return TerrainFeature::HILLS;
    }
    if (localSlope > 0.03f) {
        return TerrainFeature::ROLLING;
    }
    
    // Check for valleys (local minimum in elevation)
    if (isLocalMinimum(x, y)) {
        return TerrainFeature::VALLEY;
    }
    
    return TerrainFeature::FLAT;
}

float ClimateWorldGenerator::calculateLocalSlope(unsigned int x, unsigned int y) const {
    float centerElev = _elevationMap[x][y];
    float maxDiff = 0.0f;
    
    for (auto [dx, dy] : NEIGHBORS_8) {
        int nx = static_cast<int>(x) + dx;
        int ny = static_cast<int>(y) + dy;
        if (!inBounds(nx, ny)) continue;
        
        float neighborElev = _elevationMap[nx][ny];
        float diff = std::abs(neighborElev - centerElev);
        maxDiff = std::max(maxDiff, diff);
    }
    
    return maxDiff;
}

bool ClimateWorldGenerator::isCoastalTile(unsigned int x, unsigned int y) const {
    // Check if any neighbor is water
    for (auto [dx, dy] : NEIGHBORS_8) {
        int nx = static_cast<int>(x) + dx;
        int ny = static_cast<int>(y) + dy;
        if (!inBounds(nx, ny)) continue;
        
        if (_elevationMap[nx][ny] < _config.seaLevel) {
            return true;
        }
    }
    return false;
}

bool ClimateWorldGenerator::isLocalMinimum(unsigned int x, unsigned int y) const {
    float centerElev = _elevationMap[x][y];
    
    for (auto [dx, dy] : NEIGHBORS_8) {
        int nx = static_cast<int>(x) + dx;
        int ny = static_cast<int>(y) + dy;
        if (!inBounds(nx, ny)) continue;
        
        if (_elevationMap[nx][ny] < centerElev) {
            return false;  // Not a local minimum
        }
    }
    return true;
}

Biome ClimateWorldGenerator::lookupBiome(float temperature, float moisture, float elevation) const {
    // Returns primary biome from blend for backward compatibility
    return lookupBiomeBlend(temperature, moisture, elevation).primary();
}

BiomeBlend ClimateWorldGenerator::lookupBiomeBlend(float temperature, float moisture, float elevation) const {
    BiomeBlend blend;
    
    // Ocean biomes - no blending, water is discrete
    if (elevation < _config.seaLevel) {
        float depth = _config.seaLevel - elevation;
        if (depth > 0.2f) {
            blend = BiomeBlend(Biome::OCEAN_DEEP);
        } else if (depth > 0.1f) {
            // Blend between deep and shallow ocean
            float t = (depth - 0.1f) / 0.1f;
            blend = BiomeBlend(Biome::OCEAN_SHALLOW);
            blend.addContribution(Biome::OCEAN_DEEP, t * 0.5f);
            blend.addContribution(Biome::OCEAN_COAST, (1.0f - t) * 0.3f);
            blend.normalize();
        } else {
            // Coastal waters blend with shallow
            float t = depth / 0.1f;
            blend = BiomeBlend(Biome::OCEAN_COAST);
            blend.addContribution(Biome::OCEAN_SHALLOW, t * 0.4f);
            blend.normalize();
        }
        return blend;
    }
    
    // High elevation overrides (mountains) - limited blending
    if (elevation > 0.85f) {
        if (temperature < -10.0f) {
            blend = BiomeBlend(Biome::GLACIER);
            // Blend with alpine tundra near boundary
            float t = (-10.0f - temperature) / 10.0f;
            t = clampValue(t, 0.0f, 1.0f);
            blend.addContribution(Biome::ALPINE_TUNDRA, (1.0f - t) * 0.3f);
        } else if (temperature < 0.0f) {
            blend = BiomeBlend(Biome::ALPINE_TUNDRA);
            // Blend with glacier or bare mountain
            float t = -temperature / 10.0f;
            blend.addContribution(Biome::GLACIER, t * 0.2f);
            blend.addContribution(Biome::MOUNTAIN_BARE, (1.0f - t) * 0.2f);
        } else {
            blend = BiomeBlend(Biome::MOUNTAIN_BARE);
            blend.addContribution(Biome::ALPINE_TUNDRA, 0.15f);
        }
        blend.normalize();
        return blend;
    }
    
    if (elevation > 0.7f && temperature < 10.0f) {
        blend = BiomeBlend(Biome::ALPINE_MEADOW);
        // Blend with surrounding biomes based on elevation and temp
        float elevBlend = (elevation - 0.7f) / 0.15f;
        float tempBlend = (10.0f - temperature) / 10.0f;
        blend.addContribution(Biome::ALPINE_TUNDRA, elevBlend * 0.3f);
        blend.addContribution(Biome::TEMPERATE_GRASSLAND, (1.0f - elevBlend) * 0.2f);
        blend.normalize();
        return blend;
    }
    
    // For land biomes, calculate "soft" position in temperature-moisture space
    // and blend between neighboring biomes in the Whittaker diagram
    
    // Temperature zones with boundaries
    // FROZEN: < -10, COLD: -10 to 0, COOL: 0 to 10, TEMPERATE: 10 to 20, WARM: 20 to 30, HOT: > 30
    const float tempBoundaries[] = {-10.0f, 0.0f, 10.0f, 20.0f, 30.0f};
    
    // Calculate continuous position in temperature space (0.0 to 5.0)
    float tempPos;
    if (temperature <= -10.0f) {
        tempPos = 0.0f;
    } else if (temperature >= 30.0f) {
        tempPos = 5.0f;
    } else {
        // Find which zone we're in and calculate fractional position
        for (int i = 0; i < 5; ++i) {
            if (temperature < tempBoundaries[i]) {
                float prevBound = (i == 0) ? -20.0f : tempBoundaries[i - 1];
                tempPos = static_cast<float>(i) + (temperature - prevBound) / (tempBoundaries[i] - prevBound);
                break;
            }
        }
        if (temperature >= 30.0f) {
            tempPos = 5.0f;
        } else if (temperature >= tempBoundaries[4]) {
            tempPos = 4.0f + (temperature - tempBoundaries[4]) / 10.0f;
        }
    }
    
    // Calculate continuous position in moisture space (0.0 to 4.0)
    float moistPos = moisture * 4.0f;
    moistPos = clampValue(moistPos, 0.0f, 4.0f);
    
    // Get the four surrounding biomes in the grid
    int tempLow = static_cast<int>(std::floor(tempPos));
    int tempHigh = static_cast<int>(std::ceil(tempPos));
    int moistLow = static_cast<int>(std::floor(moistPos));
    int moistHigh = static_cast<int>(std::ceil(moistPos));
    
    tempLow = clampValue(tempLow, 0, 5);
    tempHigh = clampValue(tempHigh, 0, 5);
    moistLow = clampValue(moistLow, 0, 4);
    moistHigh = clampValue(moistHigh, 0, 4);
    
    // Fractional positions within the cell
    float tempFrac = tempPos - std::floor(tempPos);
    float moistFrac = moistPos - std::floor(moistPos);
    
    // Bilinear interpolation weights for the four corners
    float w00 = (1.0f - tempFrac) * (1.0f - moistFrac);  // low temp, low moist
    float w01 = (1.0f - tempFrac) * moistFrac;           // low temp, high moist
    float w10 = tempFrac * (1.0f - moistFrac);           // high temp, low moist
    float w11 = tempFrac * moistFrac;                     // high temp, high moist
    
    // Get biomes at the four corners
    Biome b00 = BIOME_TABLE[tempLow][moistLow];
    Biome b01 = BIOME_TABLE[tempLow][moistHigh];
    Biome b10 = BIOME_TABLE[tempHigh][moistLow];
    Biome b11 = BIOME_TABLE[tempHigh][moistHigh];
    
    // Build the blend - start with the largest weight
    float maxWeight = std::max({w00, w01, w10, w11});
    
    if (w00 == maxWeight) {
        blend = BiomeBlend(b00);
        blend.contributions[0].weight = w00;
    } else if (w01 == maxWeight) {
        blend = BiomeBlend(b01);
        blend.contributions[0].weight = w01;
    } else if (w10 == maxWeight) {
        blend = BiomeBlend(b10);
        blend.contributions[0].weight = w10;
    } else {
        blend = BiomeBlend(b11);
        blend.contributions[0].weight = w11;
    }
    
    // Add the other contributions
    if (w00 > 0.01f && b00 != blend.primary()) blend.addContribution(b00, w00);
    if (w01 > 0.01f && b01 != blend.primary()) blend.addContribution(b01, w01);
    if (w10 > 0.01f && b10 != blend.primary()) blend.addContribution(b10, w10);
    if (w11 > 0.01f && b11 != blend.primary()) blend.addContribution(b11, w11);
    
    blend.normalize();
    return blend;
}

//=============================================================================
// Phase 7: River Generation (Flow Accumulation Method with Meandering)
//=============================================================================

void ClimateWorldGenerator::generateRivers() {
    // Step 1: Calculate flow direction with meandering for every cell
    std::vector<std::vector<float>> flowAccum(_config.width,
        std::vector<float>(_config.height, 0.0f));
    std::vector<std::vector<std::pair<int,int>>> flowDir(_config.width,
        std::vector<std::pair<int,int>>(_config.height, {0, 0}));
    
    // Calculate flow direction with meandering bias for each land cell
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            if (_elevationMap[x][y] < _config.seaLevel) continue;
            
            float currentElev = _elevationMap[x][y];
            
            // Find all valid downhill neighbors with their gradients
            struct FlowOption {
                int dx, dy;
                float gradient;
            };
            std::vector<FlowOption> options;
            
            for (auto [dx, dy] : NEIGHBORS_8) {
                int nx = static_cast<int>(x) + dx;
                int ny = static_cast<int>(y) + dy;
                if (!inBounds(nx, ny)) continue;
                
                float neighborElev = _elevationMap[nx][ny];
                if (neighborElev < currentElev) {
                    // Calculate gradient (steeper = higher value)
                    float dist = (dx != 0 && dy != 0) ? 1.414f : 1.0f;
                    float gradient = (currentElev - neighborElev) / dist;
                    options.push_back({dx, dy, gradient});
                }
            }
            
            if (options.empty()) {
                flowDir[x][y] = {0, 0};  // Local minimum
                continue;
            }
            
            // Sort by gradient (steepest first)
            std::sort(options.begin(), options.end(),
                [](const FlowOption& a, const FlowOption& b) {
                    return a.gradient > b.gradient;
                });
            
            // Meandering: Use noise to sometimes pick a non-steepest path
            // Only meander on gentle slopes (where multiple paths are similar)
            float steepestGradient = options[0].gradient;
            int selectedIdx = 0;
            
            if (options.size() > 1 && steepestGradient < 0.05f) {
                // On gentle terrain, use noise to bias toward meandering
                float meander = SimplexNoise::noise(
                    x * 0.05f + _config.seed * 7.0f,
                    y * 0.05f + _config.seed * 7.0f
                );
                
                // Find options within 50% of steepest gradient
                std::vector<int> validOptions;
                for (size_t i = 0; i < options.size(); ++i) {
                    if (options[i].gradient >= steepestGradient * 0.5f) {
                        validOptions.push_back(static_cast<int>(i));
                    }
                }
                
                // Use noise to select among valid options
                if (validOptions.size() > 1) {
                    float t = (meander + 1.0f) * 0.5f;  // 0 to 1
                    selectedIdx = validOptions[static_cast<size_t>(t * (validOptions.size() - 0.01f))];
                }
            }
            
            flowDir[x][y] = {options[selectedIdx].dx, options[selectedIdx].dy};
        }
    }
    
    // Step 2: Sort cells by elevation (highest first) for flow accumulation
    std::vector<std::tuple<float, int, int>> sortedCells;
    sortedCells.reserve(_config.width * _config.height);
    
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            if (_elevationMap[x][y] >= _config.seaLevel) {
                sortedCells.emplace_back(_elevationMap[x][y], x, y);
            }
        }
    }
    
    std::sort(sortedCells.begin(), sortedCells.end(),
        [](const auto& a, const auto& b) {
            return std::get<0>(a) > std::get<0>(b);  // Highest first
        });
    
    // Step 3: Accumulate flow from high to low
    for (const auto& [elev, x, y] : sortedCells) {
        // Each cell contributes its precipitation (moisture) plus accumulated upstream flow
        float cellFlow = _moistureMap[x][y] * 0.5f + flowAccum[x][y];
        
        // Pass flow to downstream neighbor
        auto [dx, dy] = flowDir[x][y];
        if (dx != 0 || dy != 0) {
            int nx = x + dx;
            int ny = y + dy;
            if (inBounds(nx, ny) && _elevationMap[nx][ny] >= _config.seaLevel) {
                flowAccum[nx][ny] += cellFlow;
            }
        }
    }
    
    // Step 4: Determine river threshold based on accumulation distribution
    float maxFlow = 0.0f;
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            maxFlow = std::max(maxFlow, flowAccum[x][y]);
        }
    }
    
    // Rivers form where flow exceeds threshold (top ~5% of flow values)
    float riverThreshold = maxFlow * 0.03f;
    float majorRiverThreshold = maxFlow * 0.15f;
    
    // Step 5: Mark river tiles with width based on flow
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            float flow = flowAccum[x][y];
            
            if (flow > riverThreshold && _elevationMap[x][y] >= _config.seaLevel) {
                // Mark this cell as river
                _climateMap[x][y].feature = TerrainFeature::RIVER;
                _climateMap[x][y].waterLevel = std::min(flow / maxFlow, 1.0f);
                
                // For major rivers, also mark adjacent cells
                if (flow > majorRiverThreshold) {
                    // Expand river width perpendicular to flow direction
                    auto [dx, dy] = flowDir[x][y];
                    
                    // Perpendicular directions
                    std::vector<std::pair<int,int>> perpDirs;
                    if (dx == 0) {
                        perpDirs = {{-1, 0}, {1, 0}};
                    } else if (dy == 0) {
                        perpDirs = {{0, -1}, {0, 1}};
                    } else {
                        // Diagonal flow - perpendicular is the other diagonal
                        perpDirs = {{-dx, dy}, {dx, -dy}};
                    }
                    
                    for (auto [px, py] : perpDirs) {
                        int nx = static_cast<int>(x) + px;
                        int ny = static_cast<int>(y) + py;
                        if (inBounds(nx, ny) &&
                            _elevationMap[nx][ny] >= _config.seaLevel &&
                            _climateMap[nx][ny].feature != TerrainFeature::RIVER) {
                            _climateMap[nx][ny].feature = TerrainFeature::RIVER;
                            _climateMap[nx][ny].waterLevel = _climateMap[x][y].waterLevel * 0.8f;
                        }
                    }
                }
            }
        }
    }
    
    // Step 6: Lake formation in local minima with high accumulated flow
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            if (_elevationMap[x][y] < _config.seaLevel) continue;
            
            auto [dx, dy] = flowDir[x][y];
            // Check for local minimum (no downhill neighbor)
            if (dx == 0 && dy == 0 && flowAccum[x][y] > riverThreshold * 2.0f) {
                // This is a depression with significant flow - create a lake
                formLake(x, y, flowAccum[x][y]);
            }
        }
    }
}

void ClimateWorldGenerator::formLake(int centerX, int centerY, float inflow) {
    // Simple lake: mark center and immediate low neighbors
    _climateMap[centerX][centerY].feature = TerrainFeature::LAKE;
    _climateMap[centerX][centerY].biomeBlend = BiomeBlend(Biome::FRESHWATER);
    _climateMap[centerX][centerY].waterLevel = 0.5f;
    
    float centerElev = _elevationMap[centerX][centerY];
    float lakeThreshold = centerElev + 0.02f;
    
    // Expand lake to nearby cells at similar elevation
    for (auto [dx, dy] : NEIGHBORS_8) {
        int nx = centerX + dx;
        int ny = centerY + dy;
        if (!inBounds(nx, ny)) continue;
        
        if (_elevationMap[nx][ny] < lakeThreshold &&
            _elevationMap[nx][ny] >= _config.seaLevel) {
            _climateMap[nx][ny].feature = TerrainFeature::LAKE;
            _climateMap[nx][ny].biomeBlend = BiomeBlend(Biome::FRESHWATER);
            _climateMap[nx][ny].waterLevel = 0.4f;
        }
    }
}

void ClimateWorldGenerator::findRiverSources(std::vector<RiverSource>& sources) {
    // Not used in flow accumulation method, but kept for API compatibility
    (void)sources;
}

void ClimateWorldGenerator::traceRiver(int x, int y, float flow) {
    // Not used in flow accumulation method, but kept for API compatibility
    (void)x; (void)y; (void)flow;
}

void ClimateWorldGenerator::floodFillLake(int startX, int startY, float inflow) {
    // Not used in flow accumulation method, but kept for API compatibility
    (void)startX; (void)startY; (void)inflow;
}

//=============================================================================
// Apply to WorldGrid
//=============================================================================

void ClimateWorldGenerator::applyToGrid(WorldGrid& grid) {
    for (unsigned int y = 0; y < _config.height; ++y) {
        for (unsigned int x = 0; x < _config.width; ++x) {
            const TileClimate& climate = _climateMap[x][y];
            const BiomeProperties& props = getBiomeProperties(climate.biome());
            
            // Determine passability
            bool passable = true;
            bool isWater = false;
            
            if (climate.biome() == Biome::OCEAN_DEEP ||
                climate.biome() == Biome::OCEAN_SHALLOW ||
                climate.biome() == Biome::OCEAN_COAST) {
                passable = false;
                isWater = true;
            } else if (climate.biome() == Biome::FRESHWATER) {
                passable = true;  // Shallow enough to wade
                isWater = true;
            } else if (climate.feature == TerrainFeature::RIVER ||
                       climate.feature == TerrainFeature::LAKE) {
                isWater = true;
            } else if (climate.biome() == Biome::GLACIER ||
                       climate.biome() == Biome::MOUNTAIN_BARE) {
                passable = false;
            }
            
            // Get color pair based on biome
            int colorPair = getColorPairForBiome(climate.biome(), climate.feature);
            
            // Create tile
            Tile tile(100, props.displayChar, colorPair, passable, isWater, props.terrainType);
            tile.setElevation(static_cast<unsigned int>(climate.elevation * 255));
            
            grid(x, y) = tile;
        }
    }
}

int ClimateWorldGenerator::getColorPairForBiome(Biome biome, TerrainFeature feature) const {
    // Handle water features first
    if (feature == TerrainFeature::RIVER || feature == TerrainFeature::LAKE) {
        return WATER_PAIR;
    }
    
    // Map biomes to existing color pairs
    switch (biome) {
        case Biome::OCEAN_DEEP:
            return D_WATER_PAIR;
        case Biome::OCEAN_SHALLOW:
            return WATER_PAIR;
        case Biome::OCEAN_COAST:
            return S_WATER_PAIR;
        case Biome::FRESHWATER:
            return S_WATER_2_PAIR;
            
        case Biome::ICE_SHEET:
        case Biome::GLACIER:
            return PEAKS_PAIR;
        case Biome::TUNDRA:
        case Biome::ALPINE_TUNDRA:
            return SNOW_PAIR;
        case Biome::TAIGA:
        case Biome::BOREAL_FOREST:
            return FOREST_PAIR;
            
        case Biome::TEMPERATE_RAINFOREST:
        case Biome::TEMPERATE_FOREST:
        case Biome::TROPICAL_RAINFOREST:
        case Biome::TROPICAL_SEASONAL_FOREST:
            return FOREST_PAIR;
        case Biome::TEMPERATE_GRASSLAND:
            return GRASS_PAIR;
            
        case Biome::SAVANNA:
            return SAVANNA_PAIR;
        case Biome::SHRUBLAND:
        case Biome::STEPPE:
            return PLAINS_PAIR;
            
        case Biome::DESERT_HOT:
        case Biome::DESERT_COLD:
            return D_SAND_PAIR;
            
        case Biome::ALPINE_MEADOW:
            return GRASS_PAIR;
        case Biome::MOUNTAIN_BARE:
            return MOUNTAIN_2_PAIR;
            
        default:
            return PLAINS_PAIR;
    }
}

//=============================================================================
// Data Access
//=============================================================================

const TileClimate& ClimateWorldGenerator::getClimate(unsigned int x, unsigned int y) const {
    return _climateMap[x][y];
}

//=============================================================================
// Utility Functions
//=============================================================================

bool ClimateWorldGenerator::inBounds(int x, int y) const {
    return x >= 0 && x < static_cast<int>(_config.width) &&
           y >= 0 && y < static_cast<int>(_config.height);
}

float ClimateWorldGenerator::distanceToEdge(int x, int y) const {
    float dx = static_cast<float>(x) / _config.width - 0.5f;
    float dy = static_cast<float>(y) / _config.height - 0.5f;
    
    // Use maximum of normalized distances (square falloff)
    return 0.5f - std::max(std::abs(dx), std::abs(dy));
}

float ClimateWorldGenerator::smoothstep(float edge0, float edge1, float x) const {
    x = clampValue((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

// Define static constexpr member
constexpr std::pair<int, int> ClimateWorldGenerator::NEIGHBORS_8[8];

} // namespace EcoSim
