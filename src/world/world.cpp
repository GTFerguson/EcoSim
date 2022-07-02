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
  World::World (const MapGen &mapGen, const OctaveGen &octaveGen) {
    _mapGen     = mapGen;
    _octaveGen  = octaveGen;

    //  Set Terrain Prefabs
    _tileGen.push_back (TileGen { 
        90,   Tile (100, '~',  D_WATER_PAIR,    false, true ) });  //  Deep Water
    _tileGen.push_back (TileGen { 
        110,  Tile (100, '~',  WATER_PAIR,      false, true ) });  //  Water
    _tileGen.push_back (TileGen { 
        120,  Tile (100, '~',  S_WATER_PAIR,    true,  true ) });  //  Shallow Water
    _tileGen.push_back (TileGen { 
        130,  Tile (100, '~',  S_WATER_2_PAIR,  true,  true ) });  //  Shallow Water 2
    _tileGen.push_back (TileGen { 
        135,  Tile (100, '.',  SAND_PAIR,       true,  false) });  //  Sand
    _tileGen.push_back (TileGen { 
        138,  Tile (100, '.',  D_SAND_PAIR,     true,  false) });  //  Desert Sand
    _tileGen.push_back (TileGen { 
        155,  Tile (100, '.',  PLAINS_PAIR,     true,  false) });  //  Plains
    _tileGen.push_back (TileGen { 
        160,  Tile (100, '.',  SAVANNA_PAIR,    true,  false) });  //  Savanna
    _tileGen.push_back (TileGen { 
        165,  Tile (100, ',',  GRASS_PAIR,      true,  false) });  //  Short Grass 
    _tileGen.push_back (TileGen { 
        170,  Tile (100, '\'', GRASS_PAIR,      true,  false) });  //  Medium Grass  
    _tileGen.push_back (TileGen { 
        180,  Tile (100, '"',  L_GRASS_PAIR,    true,  false) });  //  Long Grass  
    _tileGen.push_back (TileGen { 
        200,  Tile (100, '"',  FOREST_PAIR,     true,  false) });  //  Forests
    _tileGen.push_back (TileGen { 
        205,  Tile (100, '.',  MOUNTAIN_PAIR,   true,  false) });  //  Mountains
    _tileGen.push_back (TileGen { 
        210,  Tile (100, '.',  MOUNTAIN_2_PAIR, true,  false) });  //  Mountains 2
    _tileGen.push_back (TileGen { 
        220,  Tile (100, '.',  MOUNTAIN_3_PAIR, true,  false) });  //  Mountains 3
    _tileGen.push_back (TileGen { 
        235,  Tile (100, '.',  SNOW_PAIR,       true,  false) });  //  Snow
    _tileGen.push_back (TileGen { 
        255,  Tile (100, '^',  PEAKS_PAIR,      false, false) });  //  Peaks

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

//  TODO update mapgen getters
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
  return _tileGen.end()->prefab;
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

  double ny   = _mapGen.seed;
  for (unsigned y = 0; y < _mapGen.rows; y++) {
    double nx = _mapGen.seed;
    for (unsigned x = 0; x < _mapGen.cols; x++) {
      double  noise = SimplexNoise::noise(_mapGen.freq * nx, _mapGen.freq * ny);
      addOctaves (noise, nx, ny);

      if (_mapGen.isIsland) {
        double dx = x / (double)_mapGen.cols - 0.5;
        double dy = y / (double)_mapGen.rows - 0.5;
        //  Average the diagonal and euclidean distance to edge of map
        double distance = 2 * max(abs(dx), abs(dy));
        distance += sqrt(dx*dx + dy*dy) / sqrt(0.5);
        distance /= 2;
        distance = pow (distance, 2);
        noise = (1 + noise - distance) / 2;
      }

      //  Redistribution
      noise = pow (noise, _mapGen.exponent); 
      noise = round (noise * _mapGen.terraces) / _mapGen.terraces;
      noise = noise * 255;

      _grid.at(x).at(y) = assignTerrain (noise);
      _grid.at(x).at(y).setElevation (noise);

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
                       const unsigned int &rate, Spawner tree) {
  random_device rd; 
  mt19937 gen(rd());
  uniform_int_distribution<unsigned short> dis (1, 100);
//  for (int y = 0; y < _rows; y++) {
//    for (int x = 0; x < _cols; x++) {
  for (vector<Tile> & row : _grid) {
    for (Tile & tile : row) {
      //  Check elevation is in range
      if (tile.getElevation() > lowElev &&
          tile.getElevation() < highElev) {
        //  The chance of spawning a tree 
 

        if (dis(gen) < rate) {
          uniform_int_distribution<unsigned> disTree (0, tree.getRate());
          tree.setTimer (disTree(gen));
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
	//	Set by x (columns) 
	for	(unsigned x = 0; x < _mapGen.cols; x++) {
		//	Initialise columns with a blank row 
		_grid.push_back (vector<Tile>());
		for	(unsigned y = 0; y < _mapGen.rows; y++) {
			///	Push into row
			_grid.at(x).push_back (_tileGen.begin()->prefab);
		}
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

  for (unsigned x = 0; x < _mapGen.cols; x++) { 
    for (unsigned y = 0; y < _mapGen.rows; y++) { 
      updateSpawners (_grid.at(x).at(y).getSpawners(), x, y);
      _grid.at(x).at(y).updateFood();
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
      vector<int> coords = 
        spawner.genCoordinates (curX, curY, _mapGen.cols, _mapGen.rows);
      Tile *tile = &_grid.at(coords[0]).at(coords[1]);
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
