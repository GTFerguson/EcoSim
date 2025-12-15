/** 
 *	Title	  : EcoSim - World 
 *	Author	: Gary Ferguson
 *	Date	  : Nov 18th, 2020
 *	Purpose	: Storage and manipulation of	world data.
 */

#include "../../include/world/world.hpp"

using namespace std;

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
    : _rng(std::random_device{}()) {
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

/**
 *  This uses an implementation on Poisson Sampling to place
 *  trees in a set elevation range.
 *
 *  @param lowElev  The lowest elevation that trees can spawn.
 *  @param highElev The highest elevation that trees can spawn.
 *  @param rate     The percentage chance of it being placed on a tile.
 *  @param tree     A Spawner object of the tree to be placed.
 */
void World::addTrees  (const unsigned int &lowElev, const unsigned int &highElev,
                       const unsigned int &rate, const Spawner& treeTemplate) {
  uniform_int_distribution<unsigned short> dis (1, 100);
//  for (int y = 0; y < _rows; y++) {
//    for (int x = 0; x < _cols; x++) {
  for (vector<Tile> & row : _grid) {
    for (Tile & tile : row) {
      //  Check elevation is in range
      if (tile.getElevation() > lowElev &&
          tile.getElevation() < highElev) {
        //  The chance of spawning a tree
 

        if (dis(_rng) < rate) {
          Spawner tree = treeTemplate;  // Copy only when needed
          uniform_int_distribution<unsigned> disTree (0, tree.getRate());
          tree.setTimer (disTree(_rng));
          tile.addSpawner (tree);
        }
      }
    }
  }
}

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

//================================================================================
//  Tile Container Handling
//================================================================================
void World::addFood (const int &x, const int &y, const Food &obj) {
  _grid[x][y].addFood (obj);  
}

void World::removeFood (const int &x, const int &y, const string &objName) {
  _grid[x][y].removeFood (objName); 
}

void World::addSpawner (const int &x, const int &y, const Spawner &obj) {
  _grid[x][y].addSpawner (obj); 
}

void World::removeSpawner (const int &x, const int &y, const string &objName) {
  _grid[x][y].removeSpawner (objName);  
}

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
      updateSpawners (_grid[x][y].getSpawners(), x, y);
      _grid[x][y].updateFood();
    }
  }
}

/**
 *  Goes through each Spawner object on a Tile and updates it
 *  as necessary, such as checking if able to spawn and if so 
 *  adding the relevant object to the world.
 *
 *  @param objects  A vector of Spawner objects to be checked.
 *  @param curX     The x coordinate of the Spawner objects.
 *  @param curY     The y coordinate of the Spawner objects.
 */
void World::updateSpawners (vector<Spawner> &spawners, const int &curX, const int &curY) {
  for (Spawner & spawner : spawners) {
    if (spawner.canSpawn()) {
      auto coords = spawner.genCoordinates(curX, curY, _mapGen.cols, _mapGen.rows);
      
      // Validate coords before accessing grid
      if (coords.first < 0 || coords.first >= static_cast<int>(_mapGen.cols) ||
          coords.second < 0 || coords.second >= static_cast<int>(_mapGen.rows)) {
        continue;
      }
      
      Tile *tile = &_grid[coords.first][coords.second];
      if (tile->isPassable())
        tile->addFood (spawner.getObject());
    }
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

  const Tile *tile;
  for (unsigned x = 0; x < _mapGen.cols; x++) {
    for (unsigned y = 0; y < _mapGen.rows; y++) {
      tile = &_grid.at(x).at(y);
      if (!tile->getFoodVec().empty() || 
          !tile->getSpawners().empty()) {
        ss  << endl << x << "," << y
            << endl << tile->contentToString();
      }
    }
  }
  return ss.str ();
}
