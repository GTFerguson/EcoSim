/**
 * @file WorldGenerator.cpp
 * @brief Implementation of terrain generation using Simplex noise
 * 
 * Extracted from World class as part of Phase 1.2 decomposition.
 */

#include "../../include/world/WorldGenerator.hpp"
#include "../../include/colorPairs.hpp"

#include <cmath>
#include <sstream>

namespace EcoSim {

WorldGenerator::WorldGenerator() {
    initializeDefaultTerrainRules();
}

WorldGenerator::WorldGenerator(const MapGen& mapGen, const OctaveGen& octaveGen)
    : _mapGen(mapGen)
    , _octaveGen(octaveGen) {
    initializeDefaultTerrainRules();
}

void WorldGenerator::initializeDefaultTerrainRules() {
    _tileGen.clear();
    
    // Set Terrain Prefabs (with TerrainType for renderer-agnostic rendering)
    _tileGen.push_back(TileGenRule{
        90,   Tile(100, '~',  D_WATER_PAIR,    false, true,  TerrainType::DEEP_WATER),
        TerrainType::DEEP_WATER});
    _tileGen.push_back(TileGenRule{
        110,  Tile(100, '~',  WATER_PAIR,      false, true,  TerrainType::WATER),
        TerrainType::WATER});
    _tileGen.push_back(TileGenRule{
        120,  Tile(100, '~',  S_WATER_PAIR,    true,  true,  TerrainType::SHALLOW_WATER),
        TerrainType::SHALLOW_WATER});
    _tileGen.push_back(TileGenRule{
        130,  Tile(100, '~',  S_WATER_2_PAIR,  true,  true,  TerrainType::SHALLOW_WATER_2),
        TerrainType::SHALLOW_WATER_2});
    _tileGen.push_back(TileGenRule{
        135,  Tile(100, '.',  SAND_PAIR,       true,  false, TerrainType::SAND),
        TerrainType::SAND});
    _tileGen.push_back(TileGenRule{
        138,  Tile(100, '.',  D_SAND_PAIR,     true,  false, TerrainType::DESERT_SAND),
        TerrainType::DESERT_SAND});
    _tileGen.push_back(TileGenRule{
        155,  Tile(100, '.',  PLAINS_PAIR,     true,  false, TerrainType::PLAINS),
        TerrainType::PLAINS});
    _tileGen.push_back(TileGenRule{
        160,  Tile(100, '.',  SAVANNA_PAIR,    true,  false, TerrainType::SAVANNA),
        TerrainType::SAVANNA});
    _tileGen.push_back(TileGenRule{
        165,  Tile(100, ',',  GRASS_PAIR,      true,  false, TerrainType::SHORT_GRASS),
        TerrainType::SHORT_GRASS});
    _tileGen.push_back(TileGenRule{
        170,  Tile(100, '\'', GRASS_PAIR,      true,  false, TerrainType::SHORT_GRASS),
        TerrainType::SHORT_GRASS});  // Medium Grass (uses SHORT_GRASS type)
    _tileGen.push_back(TileGenRule{
        180,  Tile(100, '"',  L_GRASS_PAIR,    true,  false, TerrainType::LONG_GRASS),
        TerrainType::LONG_GRASS});
    _tileGen.push_back(TileGenRule{
        200,  Tile(100, '"',  FOREST_PAIR,     true,  false, TerrainType::FOREST),
        TerrainType::FOREST});
    _tileGen.push_back(TileGenRule{
        205,  Tile(100, '.',  MOUNTAIN_PAIR,   true,  false, TerrainType::MOUNTAIN),
        TerrainType::MOUNTAIN});
    _tileGen.push_back(TileGenRule{
        210,  Tile(100, '.',  MOUNTAIN_2_PAIR, true,  false, TerrainType::MOUNTAIN_2),
        TerrainType::MOUNTAIN_2});
    _tileGen.push_back(TileGenRule{
        220,  Tile(100, '.',  MOUNTAIN_3_PAIR, true,  false, TerrainType::MOUNTAIN_3),
        TerrainType::MOUNTAIN_3});
    _tileGen.push_back(TileGenRule{
        235,  Tile(100, '.',  SNOW_PAIR,       true,  false, TerrainType::SNOW),
        TerrainType::SNOW});
    _tileGen.push_back(TileGenRule{
        255,  Tile(100, '^',  PEAKS_PAIR,      false, false, TerrainType::PEAKS),
        TerrainType::PEAKS});
}

//=============================================================================
// Configuration
//=============================================================================

void WorldGenerator::setMapGen(const MapGen& mg) {
    _mapGen = mg;
}

MapGen WorldGenerator::getMapGen() const {
    return _mapGen;
}

void WorldGenerator::setOctaveGen(const OctaveGen& og) {
    _octaveGen = og;
}

OctaveGen WorldGenerator::getOctaveGen() const {
    return _octaveGen;
}

void WorldGenerator::setTerrainLevel(unsigned int level, double newValue) {
    if (level < _tileGen.size()) {
        _tileGen[level].maxElevation = newValue;
    }
}

double WorldGenerator::getTerrainLevel(unsigned int level) const {
    if (level < _tileGen.size()) {
        return _tileGen[level].maxElevation;
    }
    return 0.0;
}

size_t WorldGenerator::getTerrainRuleCount() const {
    return _tileGen.size();
}

const std::vector<TileGenRule>& WorldGenerator::getTerrainRules() const {
    return _tileGen;
}

//=============================================================================
// Generation
//=============================================================================

void WorldGenerator::generate(WorldGrid& grid) {
    generate(grid, _mapGen.seed);
}

void WorldGenerator::generate(WorldGrid& grid, double seed) {
    _mapGen.seed = seed;
    
    double yinc = _mapGen.scale;
    double xinc = yinc / 2;

    // Precompute constants for island mode
    const double invCols = 1.0 / static_cast<double>(_mapGen.cols);
    const double invRows = 1.0 / static_cast<double>(_mapGen.rows);
    const double invSqrtHalf = 1.0 / 0.7071067811865476;  // 1/sqrt(0.5)
    const double invTerraces = 1.0 / _mapGen.terraces;

    double ny = _mapGen.seed;
    for (unsigned y = 0; y < _mapGen.rows; y++) {
        double nx = _mapGen.seed;
        // Precompute dy for this row (island mode)
        const double dy = y * invRows - 0.5;
        const double dy2 = dy * dy;
        
        for (unsigned x = 0; x < _mapGen.cols; x++) {
            double noise = SimplexNoise::noise(_mapGen.freq * nx, _mapGen.freq * ny);
            addOctaves(noise, nx, ny);

            if (_mapGen.isIsland) {
                double dx = x * invCols - 0.5;
                // Average the diagonal and euclidean distance to edge of map
                double distance = 2 * std::max(std::abs(dx), std::abs(dy));
                distance += std::sqrt(dx * dx + dy2) * invSqrtHalf;
                distance *= 0.5;
                distance = distance * distance;
                noise = (1 + noise - distance) * 0.5;
            }

            // Redistribution - optimize pow for common exponent of 1.0
            if (_mapGen.exponent != 1.0) {
                noise = std::pow(noise, _mapGen.exponent);
            }
            noise = std::round(noise * _mapGen.terraces) * invTerraces;
            noise = noise * 255;

            grid(x, y) = assignTerrain(noise);
            grid(x, y).setElevation(noise);

            nx += xinc;
        }

        ny += yinc;
    }
}

void WorldGenerator::addOctaves(double& noise, double nx, double ny) const {
    // The jump in weight each loop
    double weight = _octaveGen.maxWeight;
    double octaveFreq = _mapGen.freq;
    // Starts at one due to weight of base noise map
    double totalWeight = 1;

    double xFreq, yFreq;
    for (unsigned i = 0; i < _octaveGen.quantity; i++) {
        octaveFreq += _octaveGen.freqInterval;
        xFreq = octaveFreq * nx;
        yFreq = octaveFreq * ny;
        noise += weight * SimplexNoise::noise(xFreq, yFreq);

        totalWeight += weight;
        weight -= _octaveGen.weightInterval();
    }

    // Redistribute the noise back within the original range
    noise = ((noise / totalWeight) + 1) / 2;
}

Tile WorldGenerator::assignTerrain(double height) const {
    for (const TileGenRule& layer : _tileGen) {
        if (height < layer.maxElevation) {
            return layer.prefab;
        }
    }
    return _tileGen.back().prefab;
}

//=============================================================================
// Serialization
//=============================================================================

std::string WorldGenerator::serializeConfig() const {
    std::ostringstream ss;
    ss << _mapGen.seed << ","
       << _mapGen.scale << ","
       << _mapGen.freq << ","
       << _mapGen.exponent << ","
       << _mapGen.terraces << ","
       << _mapGen.rows << ","
       << _mapGen.cols << ","
       << _mapGen.isIsland << ","
       << _octaveGen.quantity << ","
       << _octaveGen.minWeight << ","
       << _octaveGen.maxWeight << ","
       << _octaveGen.freqInterval;
    return ss.str();
}

} // namespace EcoSim
