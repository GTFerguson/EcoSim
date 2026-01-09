/**
 *	Title		  : EcoSim
 *	Author		: Gary Ferguson
 *	Date		  : November 18th 2019
 *	Purpose		: This program aims to simulate the evolution of an ecosystem
 *			        through the application of Agent-Based Co-Evolutionary Algorithms
 *			        for Multi-Objective Optimization. The aim being that the ecosystem
 *			        generated would be stable and maintain biodiversity.
 */

#include "../include/statistics/statistics.hpp"
#include "../include/statistics/genomeStats.hpp"
#include "../include/objects/creature/creature.hpp"
#include "../include/objects/creature/navigator.hpp"
// LEGACY REMOVAL: Food and Spawner classes removed - using genetics Plants only
// #include "../include/objects/food.hpp"
// #include "../include/objects/spawner.hpp"
#include "../include/world/world.hpp"
#include "../include/world/Corpse.hpp"
#include "../include/fileHandling.hpp"
#include "../include/calendar.hpp"
#include "../include/timing.hpp"

// RenderSystem includes - abstract rendering interface
#include "../include/rendering/RenderSystem.hpp"
#include "../include/rendering/RenderTypes.hpp"
#include "../include/rendering/IRenderer.hpp"

// Genetics system integration
#include "../include/genetics/defaults/UniversalGenes.hpp"
#include "../include/genetics/organisms/PlantFactory.hpp"
#include "../include/genetics/organisms/CreatureFactory.hpp"
#include "../include/genetics/organisms/BiomeVariantExamples.hpp"

// World systems
#include "../include/world/ClimateWorldGenerator.hpp"

// Logging system for diagnostics
#include "../include/logging/Logger.hpp"

#include <stdlib.h>
#include <random>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

using namespace std;

//================================================================================
//  General simulation constants
//================================================================================
// Simulation speed: 1 tick per second (1000ms per tick)
// 1 real second = 1 game minute (calendar advances 1 minute per tick)
// Use EcoSim::Timing::SimulationSpeed for predefined speeds
const static double   SIMULATION_TICK_MS      = EcoSim::Timing::SimulationSpeed::NORMAL;  // 1000ms
const static unsigned INITIAL_POPULATION      = 200;
const static float    STARTING_RESOURCE_MIN   = 4.0f;
const static float    STARTING_RESOURCE_MAX   = 10.0f;
const static unsigned PLANT_WARMUP            = 100;   // Ticks for plants to mature before creatures spawn

//================================================================================
//  World generation defualt values
//================================================================================
const static unsigned MAP_ROWS                = 500;
const static unsigned MAP_COLS                = 500;
const static double   WORLD_DEFAULT_SCALE     = 0.0035;
const static double   WORLD_DEFAULT_FREQUENCY = 4;
const static double   WORLD_DEFAULT_EXPONENT  = 0.8;
const static unsigned WORLD_DEFAULT_TERRACES  = 64;

// LEGACY REMOVAL: Food Type constants removed - using genetics Plants only
// const static float    APPLE_CALS              = 1.5f;
// const static unsigned APPLE_LIFESPAN          = 1000;
// const static unsigned APPLE_RATE              = 1500;
// const static unsigned APPLE_MIN_RANGE         = 2;
// const static unsigned APPLE_MAX_RANGE         = 4;
// const static unsigned APPLE_MIN_ALTITUDE      = 180;
// const static unsigned APPLE_MAX_ALTITUDE      = 195;
// const static float    BANANA_CALS             = 2.0f;
// const static unsigned BANANA_LIFESPAN         = 500;
// const static unsigned BANANA_RATE             = 1000;
// const static unsigned BANANA_MIN_RANGE        = 2;
// const static unsigned BANANA_MAX_RANGE        = 4;
// const static unsigned BANANA_MIN_ALTITUDE     = 160;
// const static unsigned BANANA_MAX_ALTITUDE     = 170;
// const static float    CORPSE_CALS             = 1.0f;
// const static unsigned CORPSE_LIFESPAN         = 2000;

//================================================================================
//  Genetics-based Plant Constants (Phase 2.4)
//================================================================================
const static unsigned GRASS_MIN_ALTITUDE      = 165;
const static unsigned GRASS_MAX_ALTITUDE      = 200;
const static unsigned GRASS_SPAWN_RATE        = 5;

const static unsigned BERRY_MIN_ALTITUDE      = 170;
const static unsigned BERRY_MAX_ALTITUDE      = 190;
const static unsigned BERRY_SPAWN_RATE        = 3;

const static unsigned OAK_MIN_ALTITUDE        = 175;
const static unsigned OAK_MAX_ALTITUDE        = 195;
const static unsigned OAK_SPAWN_RATE          = 2;

const static unsigned THORN_MIN_ALTITUDE      = 160;
const static unsigned THORN_MAX_ALTITUDE      = 175;
const static unsigned THORN_SPAWN_RATE        = 2;

//================================================================================
//  UI Constants
//================================================================================
const static unsigned MAP_HORI_BORDER         = 2;
const static unsigned MAP_VERT_BORDER         = 4;

const static vector<string> SAVE_FILES = {
  "SAVE_01",
  "SAVE_02",
  "SAVE_03"
};

//================================================================================
//  Random Number Generator (Thread-Safe Singleton)
//================================================================================
class RandomGenerator {
private:
  std::mt19937 _gen;
  std::mutex _mutex;
  
  RandomGenerator() : _gen(std::random_device{}()) {}
  
public:
  // Delete copy constructor and assignment operator
  RandomGenerator(const RandomGenerator&) = delete;
  RandomGenerator& operator=(const RandomGenerator&) = delete;
  
  static RandomGenerator& instance() {
    static RandomGenerator instance;
    return instance;
  }
  
  std::mt19937& generator() {
    return _gen;
  }
  
  std::mutex& mutex() {
    return _mutex;
  }
  
  // Thread-safe random number generation
  template<typename Distribution>
  auto generate(Distribution& dist) {
    std::lock_guard<std::mutex> lock(_mutex);
    return dist(_gen);
  }
};

// Convenience function to get the random generator
static std::mt19937& gen() {
  return RandomGenerator::instance().generator();
}

//================================================================================
//  Structs
//================================================================================
struct Settings { bool alive, hudIsOn, isPaused; };


//================================================================================
//  Random Number Generation 
//================================================================================
/**
 *	Generates a random double between 0 and 10 to be used as a seed
 *	for procedural generation.
 *	
 *	@return	A double value to be used as a seed.	
 */
double randSeed () {
  uniform_real_distribution<> dis(0, 10);
  return RandomGenerator::instance().generate(dis);
}

//================================================================================
//  Rendering via RenderSystem
//================================================================================
/**
 *  This method renders the world and all creatures within the viewport.
 *  Uses the RenderSystem interface instead of direct ncurses calls.
 *
 *  @param world      The world to render.
 *  @param creatures  The vector of creature objects.
 *  @param viewport   The viewport configuration.
 */
void renderWorldAndCreatures(const World& world,
                             const vector<Creature>& creatures,
                             const Viewport& viewport) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  
  // Render world terrain, spawners, and food
  renderer.renderWorld(world, viewport);
  
  // Render creatures on top
  renderer.renderCreatures(creatures, viewport);
}

/**
 *  This renders the world generation details overlay.
 *  Uses the RenderSystem interface.
 *
 *  @param world  The world object.
 */
void renderWorldDetailsOverlay(const World& world) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  renderer.renderWorldDetails(world);
}

/**
 *  Displays the HUD with simulation statistics.
 *  Uses the RenderSystem interface.
 *
 *  @param calendar   An object that tracks the in-game date and time.
 *  @param gs         General data stored on the simulation.
 *  @param creatures  A vector of all creatures.
 *  @param viewport   The current viewport.
 *  @param paused     Whether the simulation is paused.
 */
void renderHUDDisplay(const Calendar& calendar,
                      const GeneralStats& gs,
                      const vector<Creature>& creatures,
                      const Viewport& viewport,
                      bool paused) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  
  HUDData hudData;
  hudData.population = gs.population;
  hudData.births = gs.births;
  hudData.foodEaten = gs.foodAte;
  hudData.deaths.oldAge = gs.deaths.oldAge;
  hudData.deaths.starved = gs.deaths.starved;
  hudData.deaths.dehydrated = gs.deaths.dehydrated;
  hudData.deaths.discomfort = gs.deaths.discomfort;
  hudData.deaths.predator = gs.deaths.predator;
  hudData.timeString = calendar.shortTime();
  hudData.dateString = calendar.longDate();
  hudData.worldWidth = MAP_COLS;
  hudData.worldHeight = MAP_ROWS;
  hudData.viewportX = viewport.originX;
  hudData.viewportY = viewport.originY;
  hudData.tickRate = static_cast<unsigned>(1000.0 / SIMULATION_TICK_MS);  // Ticks per second
  hudData.paused = paused;
  
  renderer.renderHUD(hudData);
}

//================================================================================
//  Simulation Advancement
//================================================================================
/**
 *	This controls the behaviour of each individual creature.
 *  Returns true if the creature died this turn (for deferred removal).
 *
 *	@param w        A reference to the world map.
 *	@param gs       General data stored on the simuation.
 *	@param c	      A vector containing all of the creatures.
 *	@param cIndex	  Current creature acting.
 *	@return		      True if creature died and should be removed.
 */
bool takeTurn (World &w, GeneralStats &gs, vector<Creature> &c,
               const unsigned int &cIndex) {
  Creature *activeC = &c.at(cIndex);

  short dc = activeC->deathCheck();
  if (dc != 0) {
    //  Record death statistic and determine cause string for logging
    std::string deathCause;
    switch (dc) {
      case 1: gs.deaths.oldAge++;     deathCause = "old_age";     break;
      case 2: gs.deaths.starved++;    deathCause = "starvation";  break;
      case 3: gs.deaths.dehydrated++; deathCause = "dehydration"; break;
      case 4: gs.deaths.discomfort++; deathCause = "discomfort";  break;
      case 5: gs.deaths.predator++;   deathCause = "combat";      break;
      default:
        std::cerr << "[ERROR] Unknown death code: " << dc << " for creature "
                  << activeC->getId() << std::endl;
        deathCause = "unknown";
        break;
    }
    
    // Log the death event
    logging::Logger::getInstance().creatureDied(
      activeC->getId(),
      activeC->generateName(),
      deathCause,
      activeC->getHunger(),
      activeC->getAge()
    );

    //  Create corpse for scavengers
    //  Size is based on creature's max health (larger creatures = more nutrition)
    float creatureSize = activeC->getMaxHealth() / 50.0f;  // 50 HP per size unit
    if (creatureSize > 0.1f) {
      // Body condition is based on creature's energy level when they died (0-1 range)
      // Higher energy = better fed = more nutritious corpse
      // RESOURCE_LIMIT is 10.0f - max hunger value
      float bodyCondition = std::max(0.0f, std::min(1.0f, activeC->getHunger() / 10.0f));
      w.addCorpse(activeC->getWorldX(), activeC->getWorldY(), creatureSize, activeC->generateName(), bodyCondition);
    }

    // Mark creature as dead by setting health below zero.
    // This ensures deathCheck() returns non-zero and isAlive() returns false.
    // Actual removal is deferred to end of tick to prevent spatial index
    // pointer invalidation.
    activeC->setHealth(-1.0f);
    return true;  // Creature died

  //  If not dead update and take action
  } else {
    activeC->update();
    
    // Update creature phenotype with location-specific environment data
    auto localEnv = w.environment().getEnvironmentStateAt(
        static_cast<int>(activeC->getWorldX()),
        static_cast<int>(activeC->getWorldY()));
    activeC->updatePhenotypeContext(localEnv);

    switch(activeC->getMotivation()) {
      case Motivation::Hungry:
        activeC->hungryBehavior(w, c, cIndex, gs);
        break;
      case Motivation::Thirsty:
        activeC->thirstyBehavior(w, c, cIndex);
        break;
      case Motivation::Amorous:
        activeC->amorousBehavior(w, c, cIndex, gs);
        break;
      case Motivation::Content:
        activeC->contentBehavior(w, c, cIndex);
        break;
      case Motivation::Tired:
        activeC->tiredBehavior(w, c, cIndex);
        break;
    }
    return false;  // Creature survived
  }
}

/**
 *  Advances the simulation a singular turn
 *
 *  @param w The world object.
 *  @param c A vector of creature objects.
 */
void advanceSimulation (World &w, vector<Creature> &c, GeneralStats &gs) {
  //  Rebuild spatial index for O(1) neighbor queries (Phase 3 optimization)
  //  This is called once per tick - O(n) rebuild cost enables O(1) queries
  w.rebuildCreatureIndex(c);
  
  //  Push simulation forward
  w.updateAllObjects ();
  
  // Update scent layer for pheromone decay (Phase 2: Sensory System)
  w.updateScentLayer();
  
  // Update corpses (decay, remove fully decayed)
  w.tickCorpses();
  
  // PRE-PASS: Have ALL breeding creatures deposit scents BEFORE any creature acts
  // This ensures scents from all potential mates are available during detection
  // (Phase 2: Gradient Navigation)
  unsigned int currentTick = w.getCurrentTick();
  for (auto& creature : c) {
    if (creature.getMotivation() == Motivation::Amorous) {
      creature.depositBreedingScent(w.getScentLayer(), currentTick);
    }
  }
  
  // DEFERRED REMOVAL: Process all creature turns first, then remove dead ones
  // This prevents spatial index pointer invalidation during the tick.
  // When creatures die, they're marked dead via die() but not removed until
  // the end of the tick, keeping all vector pointers stable.
  for (size_t i = 0; i < c.size(); ++i) {
    // Skip already-dead creatures (could have been killed by another creature this tick)
    if (!c[i].isAlive()) continue;
    takeTurn(w, gs, c, static_cast<unsigned int>(i));
  }
  
  // Remove all dead creatures at the end of the tick (stable erase)
  // Use erase-remove idiom for efficient O(n) removal
  c.erase(
    std::remove_if(c.begin(), c.end(),
      [](const Creature& creature) { return !creature.isAlive(); }),
    c.end()
  );

  gs.population = c.size ();
}

/**
 * This method creates an initial population of creatures and
 * adds them to the world using the CreatureFactory for ecological balance.
 *
 * Uses the combat-balanced archetype system with 10 distinct creature types:
 *
 * PREDATORS (25%):
 * - Apex Predator: Large territorial dominant hunters
 * - Pack Hunter: Coordinated group hunters
 * - Ambush Predator: Patient opportunistic attackers
 * - Pursuit Hunter: Speed-based chasers
 *
 * HERBIVORES (60%):
 * - Tank Herbivore: Large armored horn defenders
 * - Armored Grazer: Scaled tail-club defenders
 * - Fleet Runner: Speed-based escape artists
 * - Spiky Defender: Counter-attack spine bearers
 *
 * OPPORTUNISTS (15%):
 * - Scavenger: Corpse-feeding specialists
 * - Omnivore Generalist: Adaptable generalists
 *
 * @param w       A reference to the world map.
 * @param c       A vector containing all of the creatures.
 * @param amount  The amount of creatures to be added.
 */
void populateWorld (World &w, vector<Creature> &c, unsigned amount) {
  // Create creature factory with gene registry
  auto registry = std::make_shared<EcoSim::Genetics::GeneRegistry>();
  EcoSim::Genetics::CreatureFactory factory(registry);
  factory.registerDefaultTemplates();
  
  std::cout << "[World] Populating with " << amount << " combat-balanced creatures:" << std::endl;
  std::cout << "  Distribution: 60% Herbivores, 25% Predators, 15% Opportunists" << std::endl;
  
  // Use the balanced ecosystem mix for combat-focused archetypes
  std::vector<Creature> newCreatures = factory.createEcosystemMix(amount, MAP_COLS, MAP_ROWS);
  
  // Verify positions are passable, relocate if needed
  uniform_int_distribution<int> colDis(0, MAP_COLS-1);
  uniform_int_distribution<int> rowDis(0, MAP_ROWS-1);
  const unsigned MAX_ATTEMPTS = 10000;
  RandomGenerator& rng = RandomGenerator::instance();
  
  for (auto& creature : newCreatures) {
    int x = creature.getX();
    int y = creature.getY();
    
    // Check if position is passable, if not find a new one
    if (!w.getGrid().at(x).at(y).isPassable()) {
      unsigned attempts = 0;
      do {
        x = rng.generate(colDis);
        y = rng.generate(rowDis);
        if (++attempts > MAX_ATTEMPTS) {
          std::cerr << "[World] Warning: Could not find passable tile for creature" << std::endl;
          continue;
        }
      } while (!w.getGrid().at(x).at(y).isPassable());
      
      creature.setXY(x, y);
      creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
    }
    
    c.push_back(std::move(creature));
  }
  
  std::cout << "[World] Successfully added " << c.size() << " creatures" << std::endl;
}

/**
 * Populates the world with creatures appropriate to each biome using the
 * BiomeVariantFactory. Creatures are spawned in biomes where they can survive.
 *
 * Biome-creature mapping:
 * - Tundra/Taiga: Arctic wolves, woolly mammoths
 * - Desert/Steppe: Desert fennecs, desert camels
 * - Tropical/Savanna: Tropical jaguars, jungle elephants
 * - Temperate/Forest: Standard archetypes (pack hunters, tank herbivores, etc.)
 *
 * @param w       Reference to the world
 * @param c       Vector of creatures to populate
 * @param amount  Target total number of creatures
 */
void populateWorldByBiome(World& w, vector<Creature>& c, unsigned amount) {
  using namespace EcoSim;
  using namespace EcoSim::Genetics;
  
  auto registry = std::make_shared<GeneRegistry>();
  BiomeVariantFactory biomeFactory(registry);
  CreatureFactory standardFactory(registry);
  standardFactory.registerDefaultTemplates();
  
  std::cout << "[World] Populating by biome with " << amount << " creatures:" << std::endl;
  
  // Collect valid spawn positions for each biome category
  std::vector<std::pair<int, int>> tundraPositions;
  std::vector<std::pair<int, int>> desertPositions;
  std::vector<std::pair<int, int>> tropicalPositions;
  std::vector<std::pair<int, int>> temperatePositions;
  
  // Scan the world grid to categorize spawn positions by biome
  const auto& grid = w.grid();
  for (unsigned x = 0; x < grid.width(); ++x) {
    for (unsigned y = 0; y < grid.height(); ++y) {
      if (!grid(x, y).isPassable()) continue;
      
      // Get biome from environment system
      int biomeInt = w.environment().getBiome(static_cast<int>(x), static_cast<int>(y));
      Biome biome = static_cast<Biome>(biomeInt);
      
      switch (biome) {
        // Cold biomes
        case Biome::ICE_SHEET:
        case Biome::TUNDRA:
        case Biome::TAIGA:
        case Biome::BOREAL_FOREST:
        case Biome::ALPINE_TUNDRA:
        case Biome::GLACIER:
          tundraPositions.push_back({x, y});
          break;
          
        // Hot dry biomes
        case Biome::DESERT_HOT:
        case Biome::DESERT_COLD:
        case Biome::STEPPE:
        case Biome::SHRUBLAND:
          desertPositions.push_back({x, y});
          break;
          
        // Tropical biomes
        case Biome::TROPICAL_RAINFOREST:
        case Biome::TROPICAL_SEASONAL_FOREST:
        case Biome::SAVANNA:
          tropicalPositions.push_back({x, y});
          break;
          
        // Temperate biomes
        case Biome::TEMPERATE_RAINFOREST:
        case Biome::TEMPERATE_FOREST:
        case Biome::TEMPERATE_GRASSLAND:
        case Biome::ALPINE_MEADOW:
        default:
          temperatePositions.push_back({x, y});
          break;
      }
    }
  }
  
  // Calculate population distribution based on available biome area
  unsigned totalPositions = static_cast<unsigned>(
    tundraPositions.size() + desertPositions.size() +
    tropicalPositions.size() + temperatePositions.size());
  
  if (totalPositions == 0) {
    std::cerr << "[World] No valid spawn positions found!" << std::endl;
    return;
  }
  
  // Distribute creatures proportionally to biome area, but maintain minimum counts
  auto calculateBiomeCount = [&](size_t biomePositions) -> unsigned {
    if (biomePositions == 0) return 0;
    float proportion = static_cast<float>(biomePositions) / static_cast<float>(totalPositions);
    return std::max(2u, static_cast<unsigned>(amount * proportion));
  };
  
  unsigned tundraCount = calculateBiomeCount(tundraPositions.size());
  unsigned desertCount = calculateBiomeCount(desertPositions.size());
  unsigned tropicalCount = calculateBiomeCount(tropicalPositions.size());
  unsigned temperateCount = calculateBiomeCount(temperatePositions.size());
  
  // Normalize to match requested amount
  unsigned totalAllocated = tundraCount + desertCount + tropicalCount + temperateCount;
  if (totalAllocated > amount && temperateCount > 10) {
    temperateCount = amount - tundraCount - desertCount - tropicalCount;
  }
  
  std::cout << "  Biome distribution:" << std::endl;
  std::cout << "    Tundra: " << tundraCount << " creatures (" << tundraPositions.size() << " tiles)" << std::endl;
  std::cout << "    Desert: " << desertCount << " creatures (" << desertPositions.size() << " tiles)" << std::endl;
  std::cout << "    Tropical: " << tropicalCount << " creatures (" << tropicalPositions.size() << " tiles)" << std::endl;
  std::cout << "    Temperate: " << temperateCount << " creatures (" << temperatePositions.size() << " tiles)" << std::endl;
  
  RandomGenerator& rng = RandomGenerator::instance();
  
  // Helper to spawn creatures in a biome
  auto spawnInBiome = [&](
      std::vector<std::pair<int, int>>& positions,
      unsigned count,
      std::function<Creature(int, int)> createHerbivore,
      std::function<Creature(int, int)> createCarnivore) {
    
    if (positions.empty() || count == 0) return;
    
    std::uniform_int_distribution<size_t> posDist(0, positions.size() - 1);
    
    // 70% herbivores, 30% carnivores for balance
    unsigned herbivoreCount = static_cast<unsigned>(count * 0.70f);
    unsigned carnivoreCount = count - herbivoreCount;
    
    for (unsigned i = 0; i < herbivoreCount && !positions.empty(); ++i) {
      size_t idx = rng.generate(posDist);
      auto [x, y] = positions[idx];
      Creature creature = createHerbivore(x, y);
      creature.setXY(x, y);
      creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
      c.push_back(std::move(creature));
    }
    
    for (unsigned i = 0; i < carnivoreCount && !positions.empty(); ++i) {
      size_t idx = rng.generate(posDist);
      auto [x, y] = positions[idx];
      Creature creature = createCarnivore(x, y);
      creature.setXY(x, y);
      creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
      c.push_back(std::move(creature));
    }
  };
  
  // Spawn tundra creatures
  spawnInBiome(tundraPositions, tundraCount,
    [&](int x, int y) { return biomeFactory.createWoollyMammoth(x, y); },
    [&](int x, int y) { return biomeFactory.createArcticWolf(x, y); });
  
  // Spawn desert creatures
  spawnInBiome(desertPositions, desertCount,
    [&](int x, int y) { return biomeFactory.createDesertCamel(x, y); },
    [&](int x, int y) { return biomeFactory.createDesertFennec(x, y); });
  
  // Spawn tropical creatures
  spawnInBiome(tropicalPositions, tropicalCount,
    [&](int x, int y) { return biomeFactory.createJungleElephant(x, y); },
    [&](int x, int y) { return biomeFactory.createTropicalJaguar(x, y); });
  
  // Spawn temperate creatures using standard archetypes
  if (!temperatePositions.empty() && temperateCount > 0) {
    std::uniform_int_distribution<size_t> posDist(0, temperatePositions.size() - 1);
    
    // Use ecosystem mix for temperate zones - diverse population
    std::vector<Creature> tempCreatures = standardFactory.createEcosystemMix(
        temperateCount, MAP_COLS, MAP_ROWS);
    
    for (auto& creature : tempCreatures) {
      size_t idx = rng.generate(posDist);
      auto [x, y] = temperatePositions[idx];
      creature.setXY(x, y);
      creature.setWorldPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
      c.push_back(std::move(creature));
    }
  }
  
  std::cout << "[World] Successfully spawned " << c.size() << " biome-adapted creatures" << std::endl;
}

//================================================================================
//  Input Handling via RenderSystem
//================================================================================
/**
 *	This handles user input from a keyboard using the IInputHandler interface.
 *
 *	@param w			    The world object.
 *  @param c          The vector of creatures.
 *  @param calendar   The calendar object.
 *  @param stats      Statistics tracker.
 *  @param file       File handling object.
 *	@param xOrigin    The left most point displayed.
 *	@param yOrigin    The top most point displayed.
 *	@param settings   Simulation settings.
 *	@param mapHeight	Height of the world map.
 *	@param mapWidth		Width of the world map.
 */
void takeInput(World& w,
               vector<Creature>& c,
               Calendar& calendar,
               Statistics& stats,
               FileHandling& file,
               int& xOrigin,
               int& yOrigin,
               Settings& settings,
               const unsigned& mapHeight,
               const unsigned& mapWidth) {
  IInputHandler& input = RenderSystem::getInstance().getInputHandler();
  InputEvent event = input.pollInput();
  
  const int inc = 5;
  
  // Handle by action first (semantic actions)
  switch (event.action) {
    case InputAction::MOVE_UP:
      yOrigin -= inc;
      if (yOrigin < 0) yOrigin = 0;
      return;
      
    case InputAction::MOVE_DOWN:
      {
        yOrigin += inc;
        int relativePos = MAP_ROWS - (int)mapHeight;
        if (yOrigin > relativePos) yOrigin = relativePos;
      }
      return;
      
    case InputAction::MOVE_LEFT:
      xOrigin -= inc;
      if (xOrigin < 0) xOrigin = 0;
      return;
      
    case InputAction::MOVE_RIGHT:
      {
        xOrigin += inc;
        int relativePos = MAP_COLS - (int)mapWidth;
        if (xOrigin > relativePos) xOrigin = relativePos;
      }
      return;
      
    case InputAction::TOGGLE_HUD:
      settings.hudIsOn = !settings.hudIsOn;
      return;
      
    case InputAction::PAUSE:
      settings.isPaused = !settings.isPaused;
      return;
      
    case InputAction::ADD_CREATURES:
      populateWorld(w, c, 100);
      return;
      
    case InputAction::SAVE_STATE:
      {
        string filepath = "last_save.csv";
        file.saveGenomes(filepath, c);
        file.saveState(w, c, calendar, stats);
      }
      return;
      
    case InputAction::QUIT:
      settings.alive = false;
      return;
      
    case InputAction::TOGGLE_PAUSE_MENU:
      {
        IRenderer& renderer = RenderSystem::getInstance().getRenderer();
        renderer.togglePauseMenu();
      }
      return;
      
    case InputAction::ZOOM_IN:
      {
        IRenderer& renderer = RenderSystem::getInstance().getRenderer();
        renderer.zoomIn();
      }
      return;
      
    case InputAction::ZOOM_OUT:
      {
        IRenderer& renderer = RenderSystem::getInstance().getRenderer();
        renderer.zoomOut();
      }
      return;
      
    default:
      break;
  }
}

//================================================================================
//  Map Creator Input Handling - Helper Functions
//================================================================================
namespace MapCreator {
  void incTrn(World& w, const unsigned& level) {
    w.setTerrainLevel(level, w.getTerrainLevel(level) + 1);
  }

  void decTrn(World& w, const unsigned int& level) {
    w.setTerrainLevel(level, w.getTerrainLevel(level) - 1);
  }

  void handleMovement(InputAction action, int& xOrigin, int& yOrigin,
                      unsigned mapHeight, unsigned mapWidth) {
    const int inc = 5;
    
    switch (action) {
      case InputAction::MOVE_UP:
        yOrigin -= inc;
        if (yOrigin < 0) yOrigin = 0;
        break;
      case InputAction::MOVE_DOWN:
        {
          yOrigin += inc;
          int relativePos = MAP_ROWS - (int)mapHeight;
          if (yOrigin > relativePos) yOrigin = relativePos;
          break;
        }
      case InputAction::MOVE_LEFT:
        xOrigin -= inc;
        if (xOrigin < 0) xOrigin = 0;
        break;
      case InputAction::MOVE_RIGHT:
        {
          xOrigin += inc;
          int relativePos = MAP_COLS - (int)mapWidth;
          if (xOrigin > relativePos) xOrigin = relativePos;
          break;
        }
      default:
        break;
    }
  }

  void handleScaleChange(InputAction action, World& w) {
    switch (action) {
      case InputAction::INCREASE_SCALE:
        if (w.getScale() < 1)
          w.setScale(w.getScale() + 0.0001);
        else
          w.setScale(1);
        w.simplexGen();
        break;
      case InputAction::DECREASE_SCALE:
        if (w.getScale() > 0.0001)
          w.setScale(w.getScale() - 0.0001);
        else
          w.setScale(0.0001);
        w.simplexGen();
        break;
      default:
        break;
    }
  }

  void handleSeedChange(InputAction action, World& w) {
    switch (action) {
      case InputAction::NEW_SEED:
        w.setSeed(randSeed());
        w.simplexGen();
        break;
      case InputAction::DECREASE_SEED:
        w.setSeed(w.getSeed() - 0.005);
        w.simplexGen();
        break;
      case InputAction::INCREASE_SEED:
        w.setSeed(w.getSeed() + 0.005);
        w.simplexGen();
        break;
      default:
        break;
    }
  }

  void handleFrequencyChange(InputAction action, World& w) {
    switch (action) {
      case InputAction::INCREASE_FREQ:
        w.setFreq(w.getFreq() + 0.01);
        w.simplexGen();
        break;
      case InputAction::DECREASE_FREQ:
        w.setFreq(w.getFreq() - 0.01);
        w.simplexGen();
        break;
      default:
        break;
    }
  }

  void handleExponentChange(InputAction action, World& w) {
    switch (action) {
      case InputAction::INCREASE_EXPONENT:
        w.setExponent(w.getExponent() + 0.01);
        w.simplexGen();
        break;
      case InputAction::DECREASE_EXPONENT:
        w.setExponent(w.getExponent() - 0.01);
        w.simplexGen();
        break;
      default:
        break;
    }
  }

  void handleTerraceChange(InputAction action, World& w) {
    switch (action) {
      case InputAction::INCREASE_TERRACES:
        w.setTerraces(w.getTerraces() + 1);
        w.simplexGen();
        break;
      case InputAction::DECREASE_TERRACES:
        if (w.getTerraces() > 1)
          w.setTerraces(w.getTerraces() - 1);
        else
          w.setTerraces(1);
        w.simplexGen();
        break;
      default:
        break;
    }
  }

  void handleTerrainLevelChange(InputAction action, World& w, unsigned& trnSelector) {
    switch (action) {
      case InputAction::SELECT_TERRAIN_1: trnSelector = 0; break;
      case InputAction::SELECT_TERRAIN_2: trnSelector = 1; break;
      case InputAction::SELECT_TERRAIN_3: trnSelector = 2; break;
      case InputAction::SELECT_TERRAIN_4: trnSelector = 3; break;
      case InputAction::SELECT_TERRAIN_5: trnSelector = 4; break;
      case InputAction::SELECT_TERRAIN_6: trnSelector = 5; break;
      case InputAction::SELECT_TERRAIN_7: trnSelector = 6; break;
      case InputAction::SELECT_TERRAIN_8: trnSelector = 7; break;
      case InputAction::SELECT_TERRAIN_9: trnSelector = 8; break;
      case InputAction::INCREASE_TERRAIN_LEVEL:
        incTrn(w, trnSelector);
        w.simplexGen();
        break;
      case InputAction::DECREASE_TERRAIN_LEVEL:
        decTrn(w, trnSelector);
        w.simplexGen();
        break;
      default:
        break;
    }
  }
} // namespace MapCreator

/**
 *  Handles input for the world editor/map creator using IInputHandler.
 */
void mapCreatorInput(World& w,
                     int& xOrigin,
                     int& yOrigin,
                     unsigned& mapHeight,
                     unsigned& mapWidth,
                     unsigned& trnSelector,
                     bool& alive) {
  IInputHandler& input = RenderSystem::getInstance().getInputHandler();
  InputEvent event = input.pollInput();
  
  InputAction action = event.action;
  
  // Movement actions
  if (action == InputAction::MOVE_UP || action == InputAction::MOVE_DOWN ||
      action == InputAction::MOVE_LEFT || action == InputAction::MOVE_RIGHT) {
    MapCreator::handleMovement(action, xOrigin, yOrigin, mapHeight, mapWidth);
    return;
  }
  
  // Exit key (Enter to confirm world)
  if (action == InputAction::CONFIRM_WORLD_EDIT || action == InputAction::MENU_SELECT) {
    alive = false;
    return;
  }
  
  // Scale actions
  if (action == InputAction::INCREASE_SCALE || action == InputAction::DECREASE_SCALE) {
    MapCreator::handleScaleChange(action, w);
    return;
  }
  
  // Seed actions
  if (action == InputAction::NEW_SEED || action == InputAction::INCREASE_SEED ||
      action == InputAction::DECREASE_SEED) {
    MapCreator::handleSeedChange(action, w);
    return;
  }
  
  // Frequency actions
  if (action == InputAction::INCREASE_FREQ || action == InputAction::DECREASE_FREQ) {
    MapCreator::handleFrequencyChange(action, w);
    return;
  }
  
  // Exponent actions
  if (action == InputAction::INCREASE_EXPONENT || action == InputAction::DECREASE_EXPONENT) {
    MapCreator::handleExponentChange(action, w);
    return;
  }
  
  // Terrace actions
  if (action == InputAction::INCREASE_TERRACES || action == InputAction::DECREASE_TERRACES) {
    MapCreator::handleTerraceChange(action, w);
    return;
  }
  
  // Terrain level selection and adjustment
  if (action >= InputAction::SELECT_TERRAIN_1 && action <= InputAction::SELECT_TERRAIN_9) {
    MapCreator::handleTerrainLevelChange(action, w, trnSelector);
    return;
  }
  if (action == InputAction::INCREASE_TERRAIN_LEVEL || action == InputAction::DECREASE_TERRAIN_LEVEL) {
    MapCreator::handleTerrainLevelChange(action, w, trnSelector);
    return;
  }
}

//================================================================================
//  Main Method - Helper Functions
//================================================================================
/**
 *  Creates default map and octave generation parameters.
 */
World initializeWorld () {
  double seed = randSeed();
  
  MapGen mg { seed,
    WORLD_DEFAULT_SCALE,
    WORLD_DEFAULT_FREQUENCY,
    WORLD_DEFAULT_EXPONENT,
    WORLD_DEFAULT_TERRACES,
    MAP_ROWS,
    MAP_COLS,
    false
  };

  OctaveGen og { 2, 0.25, 0.5, 2 };

  return World (mg, og);
}

/**
 *  Runs the world editor loop for creating/editing a new world.
 *  Uses the RenderSystem interface.
 */
void runWorldEditor(World& w, vector<Creature>& creatures,
                    int& xOrigin, int& yOrigin) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  
  // Zoom out to minimum level so user can see the entire world map
  // This allows users to evaluate the generated terrain before accepting it
  while (renderer.getZoomLevel() > 4) {
    renderer.zoomOut();
  }
  
  bool inWorldEdit = true;
  unsigned trnSelector = 0;
  
  while (inWorldEdit) {
    unsigned mapHeight = renderer.getViewportMaxHeight();
    unsigned mapWidth = renderer.getViewportMaxWidth();
    int startx = renderer.getScreenCenterX() - mapWidth / 2;
    int starty = renderer.getScreenCenterY() - mapHeight / 2;
    
    // Create viewport for rendering
    Viewport viewport;
    viewport.originX = xOrigin;
    viewport.originY = yOrigin;
    viewport.width = mapWidth;
    viewport.height = mapHeight;
    viewport.screenX = startx;
    viewport.screenY = starty;

    mapCreatorInput(w, xOrigin, yOrigin, mapHeight,
                    mapWidth, trnSelector, inWorldEdit);
    
    renderer.beginFrame();
    renderWorldAndCreatures(w, creatures, viewport);
    renderWorldDetailsOverlay(w);
    renderer.endFrame();
  }
}

// LEGACY REMOVAL: addFoodSpawners function removed - using genetics Plants only
// void addFoodSpawners (World &w) {
//   Food banana (0, "Banana", "A curved yellow fruit", true, ')', 1,
//                BANANA_CALS, BANANA_LIFESPAN);
//   Food apple  (1, "Apple", "A delicious red apple", true, '*', 1,
//                APPLE_CALS, APPLE_LIFESPAN);
//   Spawner bananaPlant ("Banana Plant", "A tall plant that makes bananas",
//                        true, 'T', 13, BANANA_RATE, BANANA_MIN_RANGE,
//                        BANANA_MAX_RANGE, banana);
//   Spawner appleTree   ("Apple Tree", "A big tree that makes apples",
//                        true, '^', 13, APPLE_RATE, APPLE_MIN_RANGE,
//                        APPLE_MAX_RANGE, apple);
//   w.addTrees (APPLE_MIN_ALTITUDE,  APPLE_MAX_ALTITUDE,  2, appleTree);
//   w.addTrees (BANANA_MIN_ALTITUDE, BANANA_MAX_ALTITUDE, 2, bananaPlant);
// }

/**
 *  Initializes the genetics system and adds genetics-based plants.
 *  Uses the PlantManager component for plant lifecycle management.
 */
void addGeneticsPlants(World& w) {
  // Initialize the plant manager
  w.plants().initialize();
  
  // Add genetics-based plants by species and elevation range
  w.plants().addPlants(GRASS_MIN_ALTITUDE, GRASS_MAX_ALTITUDE,
                       GRASS_SPAWN_RATE, "grass");
  w.plants().addPlants(BERRY_MIN_ALTITUDE, BERRY_MAX_ALTITUDE,
                       BERRY_SPAWN_RATE, "berry_bush");
  w.plants().addPlants(OAK_MIN_ALTITUDE, OAK_MAX_ALTITUDE,
                       OAK_SPAWN_RATE, "oak_tree");
  w.plants().addPlants(THORN_MIN_ALTITUDE, THORN_MAX_ALTITUDE,
                       THORN_SPAWN_RATE, "thorn_bush");
}

/**
 *  Handles the New World menu option.
 *  Uses the RenderSystem interface.
 */
void handleNewWorld(World& w, vector<Creature>& creatures,
                    FileHandling& file, int& xOrigin, int& yOrigin) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  
  // Reset creature-specific ID counter for new games
  Creature::resetCreatureIdCounter(0);
  
  file.saveStatsHeader();
  renderer.renderMessage("CREATING NEW WORLD");
  renderer.endFrame();

  // Edit world to liking
  runWorldEditor(w, creatures, xOrigin, yOrigin);

  // LEGACY REMOVAL: Food spawners removed - using genetics Plants only
  // addFoodSpawners(w);
  
  // Add genetics-based plants FIRST (Phase 2.4 - now the only plant system)
  addGeneticsPlants(w);
  
  // Plant establishment period - allow plants to mature before creatures spawn
  // This ensures herbivores have viable food sources from the start
  // (seedlings are too small to provide sufficient nutrition vs their defenses)
  // Render the world during warmup so users can watch plants grow
  std::cout << "[World] Running plant establishment period ("
            << PLANT_WARMUP << " ticks)..." << std::endl;
  
  const unsigned RENDER_INTERVAL = 10;  // Render every N ticks for smooth visualization
  
  for (unsigned i = 0; i < PLANT_WARMUP; ++i) {
    w.updateAllObjects();
    
    // Render the world periodically during warmup
    if (i % RENDER_INTERVAL == 0) {
      unsigned mapHeight = renderer.getViewportMaxHeight();
      unsigned mapWidth = renderer.getViewportMaxWidth();
      int startx = renderer.getScreenCenterX() - mapWidth / 2;
      int starty = renderer.getScreenCenterY() - mapHeight / 2;
      
      // Create viewport for rendering
      Viewport viewport;
      viewport.originX = xOrigin;
      viewport.originY = yOrigin;
      viewport.width = mapWidth;
      viewport.height = mapHeight;
      viewport.screenX = startx;
      viewport.screenY = starty;
      
      renderer.beginFrame();
      renderWorldAndCreatures(w, creatures, viewport);
      
      // Show warmup progress message
      int progress = static_cast<int>((i * 100) / PLANT_WARMUP);
      std::string progressMsg = "Plant Establishment: " + std::to_string(progress) + "% (" +
                                std::to_string(i) + "/" + std::to_string(PLANT_WARMUP) + " ticks)";
      renderer.renderMessage(progressMsg, -2);
      renderer.renderMessage("Watch the plants grow across the world...", 0);
      
      renderer.endFrame();
    }
    
    if (i % 200 == 0) {
      std::cout << "  Plant warmup: " << i << "/" << PLANT_WARMUP << std::endl;
    }
  }
  std::cout << "[World] Plant establishment complete." << std::endl;
  
  // Add Creatures AFTER plants have matured
  // Use biome-based spawning to place creatures in appropriate biomes
  populateWorldByBiome(w, creatures, INITIAL_POPULATION);
}

/**
 *  Handles loading an existing world.
 *  Uses the RenderSystem interface.
 *  @return True if load was successful, false otherwise.
 */
bool handleLoadWorld(World& w, vector<Creature>& creatures,
                     Calendar& calendar, Statistics& stats,
                     FileHandling& file, int& xOrigin, int& yOrigin) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  IInputHandler& input = RenderSystem::getInstance().getInputHandler();
  
  renderer.renderMessage("LOADING WORLD");
  renderer.endFrame();
  
  if (file.loadState(w, creatures, calendar, stats)) {
    return true;
  } else {
    renderer.beginFrame();
    renderer.renderMessage("FAILED TO LOAD", -2);
    renderer.renderMessage("NEW WORLD WILL BE CREATED", 0);
    renderer.endFrame();
    
    // Wait for any key
    input.waitForInput(-1);
    
    renderer.beginFrame();
    handleNewWorld(w, creatures, file, xOrigin, yOrigin);
    return true;
  }
}

/**
 *  Processes statistics at the end of each simulation tick.
 */
void processStatistics (Statistics &stats, Calendar &calendar,
                        FileHandling &file, vector<Creature> &creatures,
                        const GeneralStats &gs) {
  stats.addRecord (gs);
  
  if (calendar.getMinute() == 0) {
    if (calendar.getHour() == 0) {
      stats.accumulate ();
      string filepath = calendar.shortDate() + ".csv";
      file.saveGenomes (filepath, creatures);
      file.appendStats (stats.toString());
      stats.clearRecords ();
    } else {
      stats.accumulateByHour ();
    }
  }
}

/**
 *  Runs the main game loop using the "Fix Your Timestep" pattern.
 *  Uses the RenderSystem interface.
 *
 *  This implements a proper game loop where:
 *  - Input is polled EVERY frame (responsive controls, no lag)
 *  - Simulation runs at a FIXED timestep (consistent physics/AI)
 *  - Rendering happens EVERY frame (smooth visuals)
 *
 *  The fixed timestep ensures simulation consistency regardless of frame rate,
 *  while input and rendering remain responsive.
 *
 *  @see include/timing.hpp for timing utilities and constants
 */
void runGameLoop(World& w, vector<Creature>& creatures,
                 Calendar& calendar, Statistics& stats, FileHandling& file,
                 int& xOrigin, int& yOrigin, Settings& settings) {
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  
  // Initialize the game clock for fixed timestep timing
  EcoSim::Timing::GameClock gameClock(SIMULATION_TICK_MS);
  gameClock.start();
  
  // Statistics tracking - persists across pause/unpause
  GeneralStats gs = { calendar, 0, 0, 0, 0 };
  
  while (settings.alive) {
    // Track tick count for saving/loading and logging
    // Declared at loop scope so it's accessible throughout all sections
    static int tickCount = 0;
    
    // Update timing at the start of each frame
    gameClock.tick();
    
    unsigned mapHeight = renderer.getViewportMaxHeight();
    unsigned mapWidth = renderer.getViewportMaxWidth();
    int startx = renderer.getScreenCenterX() - mapWidth / 2;
    int starty = renderer.getScreenCenterY() - mapHeight / 2;
    
    // Create viewport for rendering
    Viewport viewport;
    viewport.originX = xOrigin;
    viewport.originY = yOrigin;
    viewport.width = mapWidth;
    viewport.height = mapHeight;
    viewport.screenX = startx;
    viewport.screenY = starty;

    // =========================================================================
    // 1. PROCESS INPUT (every frame - responsive controls)
    // =========================================================================
    // Input is polled every frame regardless of simulation state.
    // This ensures viewport movement and UI controls feel responsive.
    takeInput(w, creatures, calendar, stats, file,
              xOrigin, yOrigin, settings, mapHeight, mapWidth);
    
    // =========================================================================
    // 1.5 HANDLE PAUSE MENU ACTIONS
    // =========================================================================
    // Check for quit request from pause menu
    if (renderer.shouldQuit()) {
      settings.alive = false;
      continue;  // Exit loop immediately
    }
    
    // Track dialog state to only populate save list once when dialog opens
    static bool wasSaveDialogOpen = false;
    static bool wasLoadDialogOpen = false;
    
    bool isSaveOpen = renderer.isSaveDialogOpen();
    bool isLoadOpen = renderer.isLoadDialogOpen();
    
    // Populate save files list only when dialog FIRST opens (transition from closed to open)
    if ((isSaveOpen && !wasSaveDialogOpen) || (isLoadOpen && !wasLoadDialogOpen)) {
      // Get list of save files and their metadata
      std::vector<std::string> saveFileNames = file.listSaveFiles();
      std::vector<SaveFileInfo> saveInfoList;
      
      for (const auto& filename : saveFileNames) {
        SaveFileInfo info;
        // Remove .json extension for display
        if (filename.size() > 5) {
          info.filename = filename.substr(0, filename.size() - 5);
        } else {
          info.filename = filename;
        }
        
        // Get metadata from file
        auto metadata = file.getSaveMetadata(filename);
        if (metadata) {
          info.timestamp = metadata->savedAt;
          info.creatureCount = metadata->creatureCount;
          info.plantCount = metadata->plantCount;
          info.tick = metadata->tick;
        } else {
          info.timestamp = "Unknown";
          info.creatureCount = 0;
          info.plantCount = 0;
          info.tick = 0;
        }
        
        saveInfoList.push_back(info);
      }
      
      renderer.setSaveFiles(saveInfoList);
      renderer.setFileExistsChecker([&file](const std::string& name) {
        return file.saveFileExists(name);
      });
    }
    
    // Update state tracking for next frame
    wasSaveDialogOpen = isSaveOpen;
    wasLoadDialogOpen = isLoadOpen;
    
    // Handle save request from pause menu (uses JSON format with user-specified filename)
    if (renderer.shouldSave()) {
      std::string filename = renderer.getSaveFilename();
      if (filename.empty()) {
        filename = "quicksave";  // Fallback
      }
      
      bool success = file.saveGameJson(
        filename + ".json",
        creatures,
        w,
        calendar,
        static_cast<unsigned>(tickCount),
        MAP_COLS,
        MAP_ROWS
      );
      
      if (success) {
        std::cout << "[Save] Game saved to '" << filename << ".json'" << std::endl;
      } else {
        std::cerr << "[Save] Failed to save game" << std::endl;
      }
      
      renderer.resetSaveFlag();
      renderer.clearSaveFilename();
    }
    
    // Handle load request from pause menu (uses JSON format with user-specified filename)
    if (renderer.shouldLoad()) {
      std::string filename = renderer.getLoadFilename();
      if (filename.empty()) {
        filename = "quicksave";  // Fallback
      }
      
      unsigned loadedTick = 0;
      bool success = file.loadGameJson(
        filename + ".json",
        creatures,
        w,
        calendar,
        loadedTick,
        MAP_COLS,
        MAP_ROWS
      );
      
      if (success) {
        tickCount = static_cast<int>(loadedTick);
        std::cout << "[Load] Loaded game from '" << filename << ".json'" << std::endl;
        
        // Reset creature ID counters to avoid ID conflicts with new creatures
        // Find the maximum IDs among loaded creatures and set counters to max+1
        int maxId = 0;
        int maxCreatureId = 0;
        for (const auto& creature : creatures) {
          if (creature.getId() > maxId) {
            maxId = creature.getId();
          }
          if (creature.getCreatureId() > maxCreatureId) {
            maxCreatureId = creature.getCreatureId();
          }
        }
        Creature::resetIdCounter(maxId + 1);
        Creature::resetCreatureIdCounter(maxCreatureId + 1);
        std::cout << "[Load] Reset organism ID counter to " << (maxId + 1) << std::endl;
        std::cout << "[Load] Reset creature ID counter to " << (maxCreatureId + 1) << std::endl;
      } else {
        std::cerr << "[Load] Failed to load game" << std::endl;
      }
      
      renderer.resetLoadFlag();
      renderer.clearLoadFilename();
    }
    
    // Check for UI-driven viewport centering requests (e.g., double-click creature list)
    if (renderer.hasViewportCenterRequest()) {
      auto [targetX, targetY] = renderer.getViewportCenterRequest();
      if (targetX >= 0 && targetY >= 0) {
        // Center viewport on target position with bounds clamping
        xOrigin = targetX - static_cast<int>(mapWidth) / 2;
        yOrigin = targetY - static_cast<int>(mapHeight) / 2;
        
        // Clamp to world boundaries
        if (xOrigin < 0) xOrigin = 0;
        if (yOrigin < 0) yOrigin = 0;
        int maxX = static_cast<int>(MAP_COLS) - static_cast<int>(mapWidth);
        int maxY = static_cast<int>(MAP_ROWS) - static_cast<int>(mapHeight);
        if (xOrigin > maxX) xOrigin = maxX;
        if (yOrigin > maxY) yOrigin = maxY;
      }
      renderer.clearViewportCenterRequest();
    }

    // =========================================================================
    // 2. UPDATE SIMULATION (fixed timestep - consistent simulation)
    // =========================================================================
    // Simulation only advances when not paused AND pause menu is not open.
    // Uses accumulator pattern to run multiple ticks if frame was slow,
    // or skip ticks if frame was fast, maintaining consistent simulation speed.
    
    // Pause simulation when either:
    // 1. User has manually paused via SPACE/P key (settings.isPaused)
    // 2. Pause menu is open (ESC menu)
    bool effectivelyPaused = settings.isPaused || renderer.isPauseMenuOpen();
    
    if (!effectivelyPaused) {
      while (gameClock.shouldUpdate()) {
        // Set current tick for the logger
        logging::Logger::getInstance().setCurrentTick(tickCount);
        
        gs = { calendar, 0, 0, 0, 0 };  // Reset stats for this tick
        advanceSimulation(w, creatures, gs);
        processStatistics(stats, calendar, file, creatures, gs);
        
        // Population snapshot every 20 ticks
        if (tickCount % 20 == 0) {
          logging::Logger::getInstance().populationSnapshot(
            tickCount,
            static_cast<int>(creatures.size()),
            0,  // plant count - would need to iterate tiles to count
            0   // food count - not tracked separately
          );
        }
        
        // Check for extinction
        if (creatures.empty()) {
          logging::Logger::getInstance().extinction("creatures");
        }
        
        // Signal tick completion (flushes logs)
        logging::Logger::getInstance().onTickEnd();
        
        calendar++;
        tickCount++;
        gameClock.consumeTick();
      }
    } else {
      // When paused, still consume accumulated time to prevent
      // simulation "catching up" when unpaused
      while (gameClock.shouldUpdate()) {
        gameClock.consumeTick();
      }
    }

    // =========================================================================
    // 3. RENDER (every frame - smooth visuals)
    // =========================================================================
    renderer.beginFrame();
    renderWorldAndCreatures(w, creatures, viewport);
    if (settings.hudIsOn)
      renderHUDDisplay(calendar, gs, creatures, viewport, settings.isPaused);
    renderer.endFrame();
    
    // Note: No sleep_for() here! The fixed timestep pattern naturally
    // handles frame pacing. The loop will run as fast as possible for
    // responsive input, while simulation stays at fixed rate.
    // If CPU usage is a concern, a small sleep can be added, but it
    // should be minimal (1-2ms) to avoid input lag.
  }
}

//================================================================================
//  Main Method
//================================================================================
/**
 *	This is the main game loop where all the action happens.
 *  Uses the RenderSystem interface instead of direct ncurses calls.
 *
 *	@return 0 is returned if program was successfully ran,
 *			    1 is returned if the program was terminated with an error.
 */
int main() {
  // Initialize the render system
  RenderConfig config;
  config.backend = RenderBackend::BACKEND_AUTO;
  // Note: inputDelayMs is no longer used for simulation timing.
  // The fixed timestep pattern in runGameLoop handles simulation pacing.
  // Input delay is set to 0 for maximum responsiveness.
  config.inputDelayMs = 0;
  config.targetFPS = 60;
  
  if (!RenderSystem::initialize(config)) {
    std::cerr << "Failed to initialize render system" << std::endl;
    return 1;
  }
  
  IRenderer& renderer = RenderSystem::getInstance().getRenderer();
  IInputHandler& input = RenderSystem::getInstance().getInputHandler();
  
  // Input delay is set to 0 for immediate, responsive input.
  // Simulation pacing is handled by the GameClock in runGameLoop.
  input.setInputDelay(0);
  
  // Initialize the creature gene registry before any creatures are created
  // This ensures the new genetics system is ready for creature-plant interactions
  Creature::initializeGeneRegistry();
  
  // Configure logger to suppress noisy plant/seed events
  // Keep COMBAT and other important events visible
  {
    logging::Logger& logger = logging::Logger::getInstance();
    logger.disableEventType("SEED_DISPERSAL");
    logger.disableEventType("PLANT_SPAWNED");
    logger.disableEventType("PLANT_DIED");
    logger.disableEventType("STARVATION");
    // Combat events use the new detailed format via combatEvent()
    logger.setCombatLogDetail(logging::CombatLogDetail::STANDARD);
  }
  
  World w = initializeWorld();

  std::vector<Creature> creatures;
  Calendar calendar;
  Statistics stats;
  FileHandling file(SAVE_FILES.at(1));
  Settings settings { true, true, false };

  // Origin coordinates for drawing world map.
  int xOrigin = 0, yOrigin = 0;

  // =========================================================================
  // ImGui Start Menu - Unified menu system
  // =========================================================================
  // Show the start menu with New Game, Load Game, Quit options
  renderer.showStartMenu();
  
  // Create minimal HUD data for rendering (menu only, no simulation yet)
  HUDData startMenuHudData;
  startMenuHudData.population = 0;
  startMenuHudData.births = 0;
  startMenuHudData.foodEaten = 0;
  startMenuHudData.deaths.oldAge = 0;
  startMenuHudData.deaths.starved = 0;
  startMenuHudData.deaths.dehydrated = 0;
  startMenuHudData.deaths.discomfort = 0;
  startMenuHudData.deaths.predator = 0;
  startMenuHudData.timeString = "";
  startMenuHudData.dateString = "";
  startMenuHudData.worldWidth = MAP_COLS;
  startMenuHudData.worldHeight = MAP_ROWS;
  startMenuHudData.viewportX = 0;
  startMenuHudData.viewportY = 0;
  startMenuHudData.tickRate = 0;
  startMenuHudData.paused = true;
  
  // Populate save files list for the load dialog
  {
    std::vector<std::string> saveFileNames = file.listSaveFiles();
    std::vector<SaveFileInfo> saveInfoList;
    
    for (const auto& filename : saveFileNames) {
      SaveFileInfo info;
      // Remove .json extension for display
      if (filename.size() > 5) {
        info.filename = filename.substr(0, filename.size() - 5);
      } else {
        info.filename = filename;
      }
      
      // Get metadata from file
      auto metadata = file.getSaveMetadata(filename);
      if (metadata) {
        info.timestamp = metadata->savedAt;
        info.creatureCount = metadata->creatureCount;
        info.plantCount = metadata->plantCount;
        info.tick = metadata->tick;
      } else {
        info.timestamp = "Unknown";
        info.creatureCount = 0;
        info.plantCount = 0;
        info.tick = 0;
      }
      
      saveInfoList.push_back(info);
    }
    
    renderer.setSaveFiles(saveInfoList);
  }
  
  // Start menu loop - wait for user to select an action
  while (!renderer.shouldQuit() && !renderer.shouldStartNewGame() && !renderer.shouldLoad()) {
    // Process input events
    input.pollInput();
    
    // Render the start menu
    renderer.beginFrame();
    renderer.renderHUD(startMenuHudData);  // Renders ImGui overlay including start menu
    renderer.endFrame();
    
    // Small delay to prevent high CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  
  // Handle the selected action
  if (renderer.shouldQuit()) {
    // User selected Quit from start menu
    settings.alive = false;
  } else if (renderer.shouldStartNewGame()) {
    // User selected New Game
    renderer.resetStartNewGameFlag();
    renderer.hideMenu();
    handleNewWorld(w, creatures, file, xOrigin, yOrigin);
  } else if (renderer.shouldLoad()) {
    // User selected and confirmed a save file to load
    std::string filename = renderer.getLoadFilename();
    if (filename.empty()) {
      filename = "quicksave";  // Fallback
    }
    
    unsigned loadedTick = 0;
    bool success = file.loadGameJson(
      filename + ".json",
      creatures,
      w,
      calendar,
      loadedTick,
      MAP_COLS,
      MAP_ROWS
    );
    
    if (success) {
      std::cout << "[Load] Loaded game from '" << filename << ".json'" << std::endl;
      
      // Reset creature ID counters to avoid ID conflicts with new creatures
      int maxId = 0;
      int maxCreatureId = 0;
      for (const auto& creature : creatures) {
        if (creature.getId() > maxId) {
          maxId = creature.getId();
        }
        if (creature.getCreatureId() > maxCreatureId) {
          maxCreatureId = creature.getCreatureId();
        }
      }
      Creature::resetIdCounter(maxId + 1);
      Creature::resetCreatureIdCounter(maxCreatureId + 1);
      std::cout << "[Load] Reset organism ID counter to " << (maxId + 1) << std::endl;
      std::cout << "[Load] Reset creature ID counter to " << (maxCreatureId + 1) << std::endl;
    } else {
      std::cerr << "[Load] Failed to load game - creating new world instead" << std::endl;
      handleNewWorld(w, creatures, file, xOrigin, yOrigin);
    }
    
    renderer.resetLoadFlag();
    renderer.clearLoadFilename();
    renderer.hideMenu();
  }

  runGameLoop(w, creatures, calendar, stats, file, xOrigin, yOrigin, settings);

  // Shutdown the render system
  RenderSystem::shutdown();
  return 0;
}
