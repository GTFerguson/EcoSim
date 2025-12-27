/**
 *	Title	  : EcoSim - World
 *	Author	: Gary Ferguson
 *	Date	  : Nov 18th, 2020
 *	Purpose	: Storage and manipulation of	world data.
 */

#include "../../include/world/world.hpp"

using namespace std;
using namespace EcoSim::Genetics;

/**
 *	This is the parametrised constructor for creating a world.
 *
 *	@param seed		   The world seed used with the Simplex Noise.
 *	@param scale	   This is the scale of the world generated.
 *	@param freq		   The frequency of noise samples taken.
 *	@param exponent	 An exponent value used for redistribution.
 *	@param terraces	 The terrace value used for redistribution.
 *	@param rows		   The number of rows in the world grid.
 *	@param cols		   The number of columns in the world grid.
 */
  World::World (const MapGen &mapGen, const OctaveGen &octaveGen)
    : _rng(std::random_device{}())
    , _scentLayer(mapGen.cols, mapGen.rows)
    , _currentTick(0) {
    _mapGen     = mapGen;
    _octaveGen  = octaveGen;

    //  Set Terrain Prefabs (with TerrainType for renderer-agnostic rendering)
    _tileGen.push_back (TileGen {
        90,   Tile (100, '~',  D_WATER_PAIR,    false, true,  TerrainType::DEEP_WATER),
        TerrainType::DEEP_WATER });
    _tileGen.push_back (TileGen {
        110,  Tile (100, '~',  WATER_PAIR,      false, true,  TerrainType::WATER),
        TerrainType::WATER });
    _tileGen.push_back (TileGen {
        120,  Tile (100, '~',  S_WATER_PAIR,    true,  true,  TerrainType::SHALLOW_WATER),
        TerrainType::SHALLOW_WATER });
    _tileGen.push_back (TileGen {
        130,  Tile (100, '~',  S_WATER_2_PAIR,  true,  true,  TerrainType::SHALLOW_WATER_2),
        TerrainType::SHALLOW_WATER_2 });
    _tileGen.push_back (TileGen {
        135,  Tile (100, '.',  SAND_PAIR,       true,  false, TerrainType::SAND),
        TerrainType::SAND });
    _tileGen.push_back (TileGen {
        138,  Tile (100, '.',  D_SAND_PAIR,     true,  false, TerrainType::DESERT_SAND),
        TerrainType::DESERT_SAND });
    _tileGen.push_back (TileGen {
        155,  Tile (100, '.',  PLAINS_PAIR,     true,  false, TerrainType::PLAINS),
        TerrainType::PLAINS });
    _tileGen.push_back (TileGen {
        160,  Tile (100, '.',  SAVANNA_PAIR,    true,  false, TerrainType::SAVANNA),
        TerrainType::SAVANNA });
    _tileGen.push_back (TileGen {
        165,  Tile (100, ',',  GRASS_PAIR,      true,  false, TerrainType::SHORT_GRASS),
        TerrainType::SHORT_GRASS });
    _tileGen.push_back (TileGen {
        170,  Tile (100, '\'', GRASS_PAIR,      true,  false, TerrainType::SHORT_GRASS),
        TerrainType::SHORT_GRASS });  //  Medium Grass (uses SHORT_GRASS type)
    _tileGen.push_back (TileGen {
        180,  Tile (100, '"',  L_GRASS_PAIR,    true,  false, TerrainType::LONG_GRASS),
        TerrainType::LONG_GRASS });
    _tileGen.push_back (TileGen {
        200,  Tile (100, '"',  FOREST_PAIR,     true,  false, TerrainType::FOREST),
        TerrainType::FOREST });
    _tileGen.push_back (TileGen {
        205,  Tile (100, '.',  MOUNTAIN_PAIR,   true,  false, TerrainType::MOUNTAIN),
        TerrainType::MOUNTAIN });
    _tileGen.push_back (TileGen {
        210,  Tile (100, '.',  MOUNTAIN_2_PAIR, true,  false, TerrainType::MOUNTAIN_2),
        TerrainType::MOUNTAIN_2 });
    _tileGen.push_back (TileGen {
        220,  Tile (100, '.',  MOUNTAIN_3_PAIR, true,  false, TerrainType::MOUNTAIN_3),
        TerrainType::MOUNTAIN_3 });
    _tileGen.push_back (TileGen {
        235,  Tile (100, '.',  SNOW_PAIR,       true,  false, TerrainType::SNOW),
        TerrainType::SNOW });
    _tileGen.push_back (TileGen {
        255,  Tile (100, '^',  PEAKS_PAIR,      false, false, TerrainType::PEAKS),
        TerrainType::PEAKS });

  set2Dgrid   ();
  simplexGen  ();
}

//================================================================================
//  Getters
//================================================================================
vector<vector<Tile>> &World::getGrid () {
  return _grid;
}

double World::getTerrainLevel (const unsigned &level) const {
  return _tileGen.at(level).elevation;
}

// NOTE: MapGen getters are simple pass-throughs.
// Consider adding validation or conversion logic if needed in future.
double    World::getSeed        () const { return _mapGen.seed;     }
double    World::getScale       () const { return _mapGen.scale;    }
double    World::getFreq        () const { return _mapGen.freq;     }
double    World::getExponent    () const { return _mapGen.exponent; }
unsigned  World::getTerraces    () const { return _mapGen.terraces; }
unsigned  World::getRows        () const { return _mapGen.rows;     }
unsigned  World::getCols        () const { return _mapGen.cols;     }
MapGen    World::getMapGen      () const { return _mapGen;          }
OctaveGen World::getOctaveGen   () const { return _octaveGen;       }

//================================================================================
//  Setters
//================================================================================
void World::setSeed     (const double &seed)        { _mapGen.seed     = seed;     }
void World::setScale    (const double &scale)       { _mapGen.scale    = scale;    }
void World::setFreq     (const double &freq)        { _mapGen.freq     = freq;     }
void World::setExponent (const double &exponent)    { _mapGen.exponent = exponent; }
void World::setTerraces (const unsigned &terraces)  { _mapGen.terraces = terraces; }
void World::setRows     (const unsigned &rows)      { _mapGen.rows     = rows;     }
void World::setCols     (const unsigned &cols)      { _mapGen.cols     = cols;     }
void World::setTerrainLevel (const unsigned &level, const unsigned &newValue) { 
  _tileGen.at(level).elevation = newValue;
}

void World::setMapGen     (const MapGen &mg)    { _mapGen     = mg; }
void World::setOctaveGen  (const OctaveGen &og) { _octaveGen  = og; }

//================================================================================
//  Set Up Methods
//================================================================================
/**
 *  Assigns the relevant Tile prefab to the terrain based
 *  on the generated elevation.
 *
 *  @param height The elevation of the tile being assigned.
 *  @return       The correct tile prefab for the given height.
 */
Tile World::assignTerrain (const double &height) {
  for (TileGen & layer : _tileGen) {
    if (height < layer.elevation) return layer.prefab;
  }
  return _tileGen.back().prefab;
}

/**
 *  This method takes in a base noise value and adds octaves to it.
 *
 *  @param noise  The base noise value.
 *  @param nx     The x-coordinate within the noise field.
 *  @param ny     The y-coordinate within the noise field.
 */
void World::addOctaves (double &noise, const double &nx, const double &ny) {
  //  The jump in weight each loop
  double weight         = _octaveGen.maxWeight;
  double octaveFreq     = _mapGen.freq;
  //  Starts at one due to weight of base noise map
  double totalWeight    = 1;

  double xFreq, yFreq;
  for (unsigned i = 0; i < _octaveGen.quantity; i++) {
    octaveFreq  += _octaveGen.freqInterval;
    xFreq        = (octaveFreq) * nx;
    yFreq        = (octaveFreq) * ny;
    noise       += weight * SimplexNoise::noise(xFreq, yFreq);

    totalWeight += weight;
    weight      -= _octaveGen.weightInterval ();
  }

  //  Redistribute the noise back within the original range
  noise = ((noise/totalWeight) + 1) / 2;
}

/**
 *  Generate a map using octaves of simplex noise and various
 *  methods of redistribution, such as using an exponent, rounding
 *  noise to terraces.
 */
void World::simplexGen () {
  double yinc = _mapGen.scale;
  double xinc = yinc / 2;

  // Precompute constants for island mode
  const double invCols = 1.0 / static_cast<double>(_mapGen.cols);
  const double invRows = 1.0 / static_cast<double>(_mapGen.rows);
  const double invSqrtHalf = 1.0 / 0.7071067811865476;  // 1/sqrt(0.5)
  const double invTerraces = 1.0 / _mapGen.terraces;

  double ny   = _mapGen.seed;
  for (unsigned y = 0; y < _mapGen.rows; y++) {
    double nx = _mapGen.seed;
    // Precompute dy for this row (island mode)
    const double dy = y * invRows - 0.5;
    const double dy2 = dy * dy;
    
    for (unsigned x = 0; x < _mapGen.cols; x++) {
      double  noise = SimplexNoise::noise(_mapGen.freq * nx, _mapGen.freq * ny);
      addOctaves (noise, nx, ny);

      if (_mapGen.isIsland) {
        double dx = x * invCols - 0.5;
        //  Average the diagonal and euclidean distance to edge of map
        double distance = 2 * max(abs(dx), abs(dy));
        distance += sqrt(dx*dx + dy2) * invSqrtHalf;
        distance *= 0.5;
        distance = distance * distance;  // pow(distance, 2) -> simple multiplication
        noise = (1 + noise - distance) * 0.5;
      }

      //  Redistribution - optimize pow for common exponent of 1.0
      if (_mapGen.exponent != 1.0) {
        noise = pow(noise, _mapGen.exponent);
      }
      noise = round(noise * _mapGen.terraces) * invTerraces;
      noise = noise * 255;

      _grid[x][y] = assignTerrain(noise);
      _grid[x][y].setElevation(noise);

      nx += xinc;
    }

    ny += yinc;
  }
}

// LEGACY REMOVAL: addTrees() function removed - legacy Spawner system no longer exists.
// Use addGeneticsPlants() instead for plant placement.

/**
 *  This method sets up the 2D grid of the environment and
 *  places a vector of tile objects at each point in it.
 **/
void World::set2Dgrid () {
	// Pre-allocate grid with resize to avoid repeated reallocations
	_grid.resize(_mapGen.cols);
	for	(unsigned x = 0; x < _mapGen.cols; x++) {
		// Resize each column to the number of rows, initialized with default tile
		_grid[x].resize(_mapGen.rows, _tileGen.begin()->prefab);
	}
}

// LEGACY REMOVAL: Legacy Tile Container Handling functions removed.
// Food and Spawner systems no longer exist. Use genetics-based Plant system instead.

//================================================================================
//  Update Objects
//================================================================================
/**
 *  Goes through each Tile object on the map and checks each
 *  Spawner object and updates it as necessary, such as checking
 *  if able to spawn and if so adding the relevant object to the world.
 */
void World::updateAllObjects () {
  // Use direct indexing [] instead of .at() since bounds are guaranteed by loop
  for (unsigned x = 0; x < _mapGen.cols; x++) {
    for (unsigned y = 0; y < _mapGen.rows; y++) {
      // LEGACY REMOVAL: Disabled legacy Food/Spawner update systems
      // These are replaced by genetics::Plant system
      // updateSpawners (_grid[x][y].getSpawners(), x, y);
      // _grid[x][y].updateFood();
    }
  }
  
  // Update genetics-based plants if initialized
  if (hasGeneticsPlants()) {
    updateGeneticsPlants();
  }
}

// LEGACY REMOVAL: updateSpawners() function removed - legacy Spawner system no longer exists.
// Plants now handle their own reproduction via SeedDispersal in updateGeneticsPlants().

//================================================================================
//  Genetics-Based Plants (Phase 2.4)
//================================================================================

void World::initializeGeneticsPlants() {
  // Create and initialize the gene registry
  _plantRegistry = std::make_shared<GeneRegistry>();
  UniversalGenes::registerDefaults(*_plantRegistry);
  
  // Create the plant factory with the registry
  _plantFactory = std::make_unique<PlantFactory>(_plantRegistry);
  
  // Register default species templates
  _plantFactory->registerTemplate(PlantFactory::createBerryBushTemplate());
  _plantFactory->registerTemplate(PlantFactory::createOakTreeTemplate());
  _plantFactory->registerTemplate(PlantFactory::createGrassTemplate());
  _plantFactory->registerTemplate(PlantFactory::createThornBushTemplate());
  
  // Initialize default environment state (direct struct member access)
  _currentEnvironment.temperature = 20.0f;  // 20°C default
  _currentEnvironment.humidity = 0.6f;      // Moderate humidity
  _currentEnvironment.time_of_day = 0.5f;   // Noon
  
  std::cout << "[World] Genetics plant system initialized with "
            << _plantFactory->getTemplateNames().size() << " species templates" << std::endl;
}

bool World::hasGeneticsPlants() const {
  return _plantFactory != nullptr && _plantRegistry != nullptr;
}

void World::addGeneticsPlants(unsigned int lowElev, unsigned int highElev,
                              unsigned int rate, const std::string& speciesName) {
  if (!hasGeneticsPlants()) {
    std::cerr << "[World] Error: Genetics plant system not initialized. "
              << "Call initializeGeneticsPlants() first." << std::endl;
    return;
  }
  
  if (!_plantFactory->hasTemplate(speciesName)) {
    std::cerr << "[World] Error: Unknown plant species '" << speciesName << "'" << std::endl;
    return;
  }
  
  uniform_int_distribution<unsigned short> dis(1, 100);
  unsigned int plantsAdded = 0;
  
  for (unsigned x = 0; x < _mapGen.cols; x++) {
    for (unsigned y = 0; y < _mapGen.rows; y++) {
      Tile& tile = _grid[x][y];
      
      // Check elevation is in range and tile is passable
      if (tile.getElevation() > lowElev &&
          tile.getElevation() < highElev &&
          tile.isPassable()) {
        
        // Random chance of placing plant
        if (dis(_rng) <= rate) {
          Plant plant = _plantFactory->createFromTemplate(speciesName,
                                                          static_cast<int>(x),
                                                          static_cast<int>(y));
          tile.addPlant(std::make_shared<Plant>(std::move(plant)));
          ++plantsAdded;
        }
      }
    }
  }
  
  std::cout << "[World] Added " << plantsAdded << " " << speciesName << " plants" << std::endl;
}

bool World::addGeneticsPlant(int x, int y, const std::string& speciesName) {
  if (!hasGeneticsPlants()) {
    std::cerr << "[World] Error: Genetics plant system not initialized." << std::endl;
    return false;
  }
  
  if (x < 0 || x >= static_cast<int>(_mapGen.cols) ||
      y < 0 || y >= static_cast<int>(_mapGen.rows)) {
    return false;
  }
  
  if (!_plantFactory->hasTemplate(speciesName)) {
    return false;
  }
  
  Tile& tile = _grid[x][y];
  if (!tile.isPassable()) {
    return false;
  }
  
  Plant plant = _plantFactory->createFromTemplate(speciesName, x, y);
  return tile.addPlant(std::make_shared<Plant>(std::move(plant)));
}

EnvironmentState& World::getEnvironment() {
  return _currentEnvironment;
}

void World::updateEnvironment(float temperature, float lightLevel, float waterAvailability) {
  _currentEnvironment.temperature = temperature;
  _currentEnvironment.humidity = waterAvailability;  // Map water availability to humidity
  _currentEnvironment.time_of_day = lightLevel;      // Map light level to time_of_day (0=dark, 1=bright)
}

PlantFactory* World::getPlantFactory() {
  return _plantFactory.get();
}

std::shared_ptr<GeneRegistry> World::getPlantRegistry() {
  return _plantRegistry;
}

void World::updateGeneticsPlants() {
  if (!hasGeneticsPlants()) {
    return;
  }
  
  // Collect seed dispersal events during iteration
  std::vector<std::pair<EcoSim::Genetics::DispersalEvent, std::shared_ptr<EcoSim::Genetics::Plant>>> dispersalEvents;
  
  // Update all plants on all tiles
  for (unsigned x = 0; x < _mapGen.cols; x++) {
    for (unsigned y = 0; y < _mapGen.rows; y++) {
      Tile& tile = _grid[x][y];
      
      // Update plants with current environment
      tile.updatePlants(_currentEnvironment);
      
      // Remove dead plants
      tile.removeDeadPlants();
      
      // Handle seed dispersal for mature plants
      for (auto& plant : tile.getPlants()) {
        if (!plant || !plant->isAlive()) continue;
        
        // Phase 2: Emit plant scent if plant has scent production capability
        float scentRate = plant->getScentProductionRate();
        if (scentRate > 0.01f) {  // Only emit if meaningful scent production
          std::array<float, 8> signature = plant->getScentSignature();
          float intensity = scentRate * plant->getCurrentSize() / plant->getMaxSize();
          // Scale intensity by maturity - young plants produce less scent
          
          // Create scent deposit for this plant
          // Plants don't have creature IDs, so use -1
          // Use a decay rate of 50 ticks for plant scents (they dissipate relatively quickly)
          EcoSim::ScentDeposit plantScent(
              EcoSim::ScentType::FOOD_TRAIL,  // Plant scents use FOOD_TRAIL type
              -1,                              // No creature ID for plants
              intensity,
              signature,
              _currentTick,
              50                               // Decay rate in ticks
          );
          
          _scentLayer.deposit(
              static_cast<int>(x),
              static_cast<int>(y),
              plantScent
          );
        }
        
        // LEGACY REMOVAL: No longer create legacy Food objects from plant fruit.
        // Creatures now feed directly from genetics::Plant via FeedingInteraction.
        // The plant's canProduceFruit() and energy are used directly by creatures.
        
        // Seed dispersal - mature plants attempt passive dispersal
        // Use a probability check to avoid too many seeds per tick
        // Support both fruit-based AND vegetative reproduction pathways
        bool canDisperse = plant->canProduceFruit() || plant->canSpreadVegetatively();
        
        if (canDisperse) {
          std::uniform_real_distribution<float> dist(0.0f, 1.0f);
          
          // Calculate dispersal chance based on reproduction method
          float dispersalChance;
          if (plant->canSpreadVegetatively()) {
            // Vegetative spread (runners/stolons) - scaled by plant maturity
            // Larger, more established plants spread more effectively
            float sizeRatio = plant->getCurrentSize() / plant->getMaxSize();
            dispersalChance = plant->getRunnerProduction() * 0.15f * sizeRatio;
          } else {
            // Fruit-based dispersal
            dispersalChance = plant->getFruitProductionRate() * 0.1f;
          }
          
          if (dist(_rng) < dispersalChance) {
            EcoSim::Genetics::DispersalEvent event = _seedDispersal.disperse(*plant, &_currentEnvironment);
            dispersalEvents.push_back({event, plant});
          }
          
          // Reset timer after dispersal attempt (successful or not)
          // This enforces the cooldown period before the next attempt
          plant->resetFruitTimer();
        }
      }
    }
  }
  
  // Process dispersal events - spawn new plants at target locations
  for (const auto& [event, parentPlant] : dispersalEvents) {
    // Check bounds
    if (event.targetX < 0 || event.targetX >= static_cast<int>(_mapGen.cols) ||
        event.targetY < 0 || event.targetY >= static_cast<int>(_mapGen.rows)) {
      continue;
    }
    
    Tile& targetTile = _grid[event.targetX][event.targetY];
    
    // Check if tile is passable and not already crowded with plants
    if (!targetTile.isPassable() || targetTile.getPlants().size() >= 3) {
      continue;
    }
    
    // Viability check - seed may not germinate
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(_rng) > event.seedViability) {
      continue;  // Seed failed to germinate
    }
    
    // Create offspring plant using parent's genome (with mutation)
    // Use PlantFactory::createOffspring for proper genome handling
    EcoSim::Genetics::Plant offspring = _plantFactory->createOffspring(
      *parentPlant, *parentPlant,  // Self-crossing (asexual via seed)
      event.targetX, event.targetY
    );
    
    // Add the new plant to the target tile
    targetTile.addPlant(std::make_shared<EcoSim::Genetics::Plant>(std::move(offspring)));
  }
}

//================================================================================
//  To String
//================================================================================
string World::toString () const {
  ostringstream ss;
  ss  << _mapGen.seed             << ","
      << _mapGen.scale            << ","
      << _mapGen.freq             << ","
      << _mapGen.exponent         << ","
      << _mapGen.terraces         << ","
      << _mapGen.rows             << ","
      << _mapGen.cols             << ","
      << _mapGen.isIsland         << ","
      << _octaveGen.quantity      << ","
      << _octaveGen.minWeight     << ","
      << _octaveGen.maxWeight     << ","
      << _octaveGen.freqInterval;

  // Output plant data for tiles that have plants
  const Tile *tile;
  for (unsigned x = 0; x < _mapGen.cols; x++) {
    for (unsigned y = 0; y < _mapGen.rows; y++) {
      tile = &_grid.at(x).at(y);
      if (!tile->getPlants().empty()) {
        ss  << endl << x << "," << y
            << endl << tile->contentToString();
      }
    }
  }
  return ss.str ();
}

//================================================================================
//  Scent Layer (Phase 1: Sensory System)
//================================================================================

EcoSim::ScentLayer& World::getScentLayer() {
  return _scentLayer;
}

const EcoSim::ScentLayer& World::getScentLayer() const {
  return _scentLayer;
}

void World::updateScentLayer() {
  ++_currentTick;
  _scentLayer.update(_currentTick);
}

unsigned int World::getCurrentTick() const {
  return _currentTick;
}

//================================================================================
//  Corpse Management
//================================================================================

void World::addCorpse(float x, float y, float size, const std::string& speciesName, float bodyCondition) {
  // Remove oldest/most decayed corpse if at capacity
  if (_corpses.size() >= MAX_CORPSES) {
    auto it = std::max_element(_corpses.begin(), _corpses.end(),
      [](const auto& a, const auto& b) {
        return a->getDecayProgress() < b->getDecayProgress();
      });
    if (it != _corpses.end()) {
      _corpses.erase(it);
    }
  }
  
  _corpses.push_back(std::make_unique<world::Corpse>(x, y, size, speciesName, bodyCondition));
}

void World::tickCorpses() {
  // Update decay for all corpses
  for (auto& corpse : _corpses) {
    corpse->tick();
  }
  
  // Remove fully decayed corpses
  _corpses.erase(
    std::remove_if(_corpses.begin(), _corpses.end(),
      [](const auto& c) { return c->isFullyDecayed(); }),
    _corpses.end()
  );
}

const std::vector<std::unique_ptr<world::Corpse>>& World::getCorpses() const {
  return _corpses;
}

world::Corpse* World::findNearestCorpse(float x, float y, float maxRange) {
  world::Corpse* nearest = nullptr;
  float nearestDistSq = maxRange * maxRange;
  
  for (auto& corpse : _corpses) {
    if (corpse->isExhausted()) continue;
    
    float dx = corpse->getX() - x;
    float dy = corpse->getY() - y;
    float distSq = dx * dx + dy * dy;
    
    if (distSq < nearestDistSq) {
      nearestDistSq = distSq;
      nearest = corpse.get();
    }
  }
  
  return nearest;
}

void World::removeCorpse(world::Corpse* corpse) {
  _corpses.erase(
    std::remove_if(_corpses.begin(), _corpses.end(),
      [corpse](const auto& c) { return c.get() == corpse; }),
    _corpses.end()
  );
}
