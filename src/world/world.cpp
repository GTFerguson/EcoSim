/**
 * @file world.cpp
 * @brief Implementation of the World class - central coordinator for world subsystems
 * @author Gary Ferguson
 */

#include "../../include/world/world.hpp"

#include <sstream>

using namespace std;

//================================================================================
// Construction
//================================================================================

World::World(const MapGen& mapGen, const OctaveGen& octaveGen)
    : _scentLayer(mapGen.cols, mapGen.rows)
    , _currentTick(0)
    , _corpseManager(std::make_unique<EcoSim::CorpseManager>())
    , _seasonManager(std::make_unique<EcoSim::SeasonManager>()) {
    
    // Create the terrain generator with the provided configuration
    _generator = std::make_unique<EcoSim::WorldGenerator>(mapGen, octaveGen);
    
    // Initialize grid and generate terrain
    set2Dgrid();
    simplexGen();
    
    // Create and generate climate data
    EcoSim::ClimateGeneratorConfig climateConfig;
    climateConfig.width = mapGen.cols;
    climateConfig.height = mapGen.rows;
    climateConfig.seed = static_cast<unsigned int>(mapGen.seed);
    climateConfig.isIsland = mapGen.isIsland;
    _climateGenerator = std::make_unique<EcoSim::ClimateWorldGenerator>(climateConfig);
    _climateGenerator->generate(_grid);  // Generates climate data for the grid
    
    // Initialize the environment system (must happen after grid is ready)
    _environmentSystem = std::make_unique<EcoSim::EnvironmentSystem>(*_seasonManager, _grid);
    
    // Connect climate data to environment system for per-tile queries
    _environmentSystem->setClimateMap(&_climateGenerator->getClimateMap());
    
    // Initialize the plant manager and connect to environment system
    _plantManager = std::make_unique<EcoSim::PlantManager>(_grid, _scentLayer);
    _plantManager->setEnvironmentSystem(_environmentSystem.get());
}

//================================================================================
// Core Subsystem Accessors
//================================================================================

EcoSim::WorldGrid& World::grid() {
    return _grid;
}

const EcoSim::WorldGrid& World::grid() const {
    return _grid;
}

EcoSim::ScentLayer& World::scentLayer() {
    return _scentLayer;
}

const EcoSim::ScentLayer& World::scentLayer() const {
    return _scentLayer;
}

EcoSim::CorpseManager& World::corpses() {
    return *_corpseManager;
}

const EcoSim::CorpseManager& World::corpses() const {
    return *_corpseManager;
}

EcoSim::SeasonManager& World::seasons() {
    return *_seasonManager;
}

const EcoSim::SeasonManager& World::seasons() const {
    return *_seasonManager;
}

EcoSim::EnvironmentSystem& World::environment() {
    return *_environmentSystem;
}

const EcoSim::EnvironmentSystem& World::environment() const {
    return *_environmentSystem;
}

EcoSim::PlantManager& World::plants() {
    return *_plantManager;
}

const EcoSim::PlantManager& World::plants() const {
    return *_plantManager;
}

//================================================================================
// Spatial Indexing
//================================================================================

void World::initializeCreatureIndex() {
    const MapGen& mapGen = _generator->getMapGen();
    _creatureIndex = std::make_unique<EcoSim::SpatialIndex>(
        static_cast<int>(mapGen.cols),
        static_cast<int>(mapGen.rows)
    );
}

EcoSim::SpatialIndex* World::getCreatureIndex() {
    return _creatureIndex.get();
}

const EcoSim::SpatialIndex* World::getCreatureIndex() const {
    return _creatureIndex.get();
}

void World::rebuildCreatureIndex(std::vector<Creature>& creatures) {
    if (!_creatureIndex) {
        initializeCreatureIndex();
    }
    _creatureIndex->rebuild(creatures);
}

//================================================================================
// Terrain Generation Configuration
//================================================================================

double    World::getSeed()      const { return _generator->getMapGen().seed;     }
double    World::getScale()     const { return _generator->getMapGen().scale;    }
double    World::getFreq()      const { return _generator->getMapGen().freq;     }
double    World::getExponent()  const { return _generator->getMapGen().exponent; }
unsigned  World::getTerraces()  const { return _generator->getMapGen().terraces; }
unsigned  World::getRows()      const { return _generator->getMapGen().rows;     }
unsigned  World::getCols()      const { return _generator->getMapGen().cols;     }
MapGen    World::getMapGen()    const { return _generator->getMapGen();          }
OctaveGen World::getOctaveGen() const { return _generator->getOctaveGen();       }

double World::getTerrainLevel(const unsigned int& level) const {
    return _generator->getTerrainLevel(level);
}

void World::setSeed(const double& seed) {
    MapGen mg = _generator->getMapGen();
    mg.seed = seed;
    _generator->setMapGen(mg);
}

void World::setScale(const double& scale) {
    MapGen mg = _generator->getMapGen();
    mg.scale = scale;
    _generator->setMapGen(mg);
}

void World::setFreq(const double& freq) {
    MapGen mg = _generator->getMapGen();
    mg.freq = freq;
    _generator->setMapGen(mg);
}

void World::setExponent(const double& exponent) {
    MapGen mg = _generator->getMapGen();
    mg.exponent = exponent;
    _generator->setMapGen(mg);
}

void World::setTerraces(const unsigned int& terraces) {
    MapGen mg = _generator->getMapGen();
    mg.terraces = terraces;
    _generator->setMapGen(mg);
}

void World::setRows(const unsigned int& rows) {
    MapGen mg = _generator->getMapGen();
    mg.rows = rows;
    _generator->setMapGen(mg);
}

void World::setCols(const unsigned int& cols) {
    MapGen mg = _generator->getMapGen();
    mg.cols = cols;
    _generator->setMapGen(mg);
}

void World::setTerrainLevel(const unsigned int& level, const unsigned int& newValue) {
    _generator->setTerrainLevel(level, static_cast<double>(newValue));
}

void World::setMapGen(const MapGen& mg) {
    _generator->setMapGen(mg);
}

void World::setOctaveGen(const OctaveGen& og) {
    _generator->setOctaveGen(og);
}

void World::simplexGen() {
    _generator->generate(_grid);
}

void World::regenerateClimate() {
    if (_climateGenerator) {
        _climateGenerator->generate(_grid);
        
        // Update environment system with new climate data
        if (_environmentSystem) {
            _environmentSystem->setClimateMap(&_climateGenerator->getClimateMap());
        }
    }
}

void World::regenerateClimate(unsigned int seed) {
    if (_climateGenerator) {
        EcoSim::ClimateGeneratorConfig& config = _climateGenerator->getConfig();
        config.seed = seed;
        _climateGenerator->setConfig(config);
        regenerateClimate();
    }
}

EcoSim::ClimateWorldGenerator& World::climateGenerator() {
    return *_climateGenerator;
}

const EcoSim::ClimateWorldGenerator& World::climateGenerator() const {
    return *_climateGenerator;
}

//================================================================================
// Simulation Update
//================================================================================

void World::updateAllObjects() {
    if (_plantManager && _plantManager->isInitialized()) {
        _plantManager->tick(_currentTick);
    }
}

void World::updateScentLayer() {
    ++_currentTick;
    _scentLayer.update(_currentTick);
}

unsigned int World::getCurrentTick() const {
    return _currentTick;
}

//================================================================================
// Corpse Convenience Methods
//================================================================================

void World::addCorpse(float x, float y, float size, const std::string& speciesName, float bodyCondition) {
    _corpseManager->addCorpse(x, y, size, speciesName, bodyCondition);
}

void World::tickCorpses() {
    _corpseManager->tick();
}

const std::vector<std::unique_ptr<world::Corpse>>& World::getCorpses() const {
    return _corpseManager->getAll();
}

world::Corpse* World::findNearestCorpse(float x, float y, float maxRange) {
    return _corpseManager->findNearest(x, y, maxRange);
}

void World::removeCorpse(world::Corpse* corpse) {
    _corpseManager->removeCorpse(corpse);
}

//================================================================================
// Serialization
//================================================================================

string World::toString() const {
    const MapGen& mapGen = _generator->getMapGen();
    const OctaveGen& octaveGen = _generator->getOctaveGen();
    
    ostringstream ss;
    ss << mapGen.seed << ","
       << mapGen.scale << ","
       << mapGen.freq << ","
       << mapGen.exponent << ","
       << mapGen.terraces << ","
       << mapGen.rows << ","
       << mapGen.cols << ","
       << mapGen.isIsland << ","
       << octaveGen.quantity << ","
       << octaveGen.minWeight << ","
       << octaveGen.maxWeight << ","
       << octaveGen.freqInterval;

    // Output plant data for tiles that have plants
    const Tile* tile;
    for (unsigned x = 0; x < mapGen.cols; x++) {
        for (unsigned y = 0; y < mapGen.rows; y++) {
            tile = &_grid.at(x, y);
            if (!tile->getPlants().empty()) {
                ss << endl << x << "," << y
                   << endl << tile->contentToString();
            }
        }
    }
    return ss.str();
}

//================================================================================
// Legacy Interface
//================================================================================

vector<vector<Tile>>& World::getGrid() {
    return _grid.raw();
}

EcoSim::ScentLayer& World::getScentLayer() {
    return _scentLayer;
}

const EcoSim::ScentLayer& World::getScentLayer() const {
    return _scentLayer;
}

//================================================================================
// Private Methods
//================================================================================

void World::set2Dgrid() {
    const MapGen& mapGen = _generator->getMapGen();
    const auto& rules = _generator->getTerrainRules();
    
    // Use WorldGrid::resize with default tile from first terrain type
    if (!rules.empty()) {
        _grid.resize(mapGen.cols, mapGen.rows, rules.front().prefab);
    } else {
        _grid.resize(mapGen.cols, mapGen.rows);
    }
}
