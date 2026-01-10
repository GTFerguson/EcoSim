/**
 * @file PlantManager.cpp
 * @brief Implementation of PlantManager for genetics-based plant infrastructure
 */

#include "../../include/world/PlantManager.hpp"
#include "../../include/world/EnvironmentSystem.hpp"
#include "../../include/genetics/organisms/BiomeVariantExamples.hpp"

namespace EcoSim {

using namespace Genetics;

//==============================================================================
// Construction
//==============================================================================

PlantManager::PlantManager(WorldGrid& grid, ScentLayer& scents)
    : _grid(grid)
    , _scents(scents)
    , _environmentSystem(nullptr)
    , _rng(std::random_device{}()) {
}

PlantManager::~PlantManager() = default;

void PlantManager::setEnvironmentSystem(const EnvironmentSystem* envSystem) {
    _environmentSystem = envSystem;
    if (_environmentSystem && _environmentSystem->hasClimateData()) {
        std::cout << "[PlantManager] Connected to EnvironmentSystem with climate data" << std::endl;
    } else if (_environmentSystem) {
        std::cout << "[PlantManager] Connected to EnvironmentSystem (no climate data)" << std::endl;
    }
}

//==============================================================================
// Initialization
//==============================================================================

void PlantManager::initialize() {
    // Skip if already initialized (idempotent)
    if (isInitialized()) {
        return;
    }
    
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
    
    // Initialize default environment state
    _currentEnvironment.temperature = 20.0f;  // 20°C default
    _currentEnvironment.humidity = 0.6f;      // Moderate humidity
    _currentEnvironment.time_of_day = 0.5f;   // Noon
    
    // Create the biome variant factory for biome-specific plants
    // Pass existing plant factory to avoid re-registering genes
    _biomeFactory = std::make_unique<BiomeVariantFactory>(
        _plantRegistry, nullptr, _plantFactory.get());
    
    std::cout << "[PlantManager] Initialized with "
              << _plantFactory->getTemplateNames().size() << " species templates" << std::endl;
}

bool PlantManager::isInitialized() const {
    return _plantFactory != nullptr && _plantRegistry != nullptr;
}

//==============================================================================
// Population
//==============================================================================

void PlantManager::addPlants(unsigned lowElev, unsigned highElev,
                             unsigned rate, const std::string& species) {
    if (!isInitialized()) {
        std::cerr << "[PlantManager] Error: Plant system not initialized. "
                  << "Call initialize() first." << std::endl;
        return;
    }
    
    if (!_plantFactory->hasTemplate(species)) {
        std::cerr << "[PlantManager] Error: Unknown plant species '" << species << "'" << std::endl;
        return;
    }
    
    std::uniform_int_distribution<unsigned short> dis(1, 100);
    unsigned int plantsAdded = 0;
    
    unsigned cols = _grid.width();
    unsigned rows = _grid.height();
    
    for (unsigned x = 0; x < cols; x++) {
        for (unsigned y = 0; y < rows; y++) {
            Tile& tile = _grid(x, y);
            
            // Check elevation is in range and tile is passable
            if (tile.getElevation() > lowElev &&
                tile.getElevation() < highElev &&
                tile.isPassable()) {
                
                // Random chance of placing plant
                if (dis(_rng) <= rate) {
                    Plant plant = _plantFactory->createFromTemplate(species,
                                                                    static_cast<int>(x),
                                                                    static_cast<int>(y));
                    tile.addPlant(std::make_shared<Plant>(std::move(plant)));
                    ++plantsAdded;
                }
            }
        }
    }
    
    std::cout << "[PlantManager] Added " << plantsAdded << " " << species << " plants" << std::endl;
}

bool PlantManager::addPlant(int x, int y, const std::string& species) {
    if (!isInitialized()) {
        std::cerr << "[PlantManager] Error: Plant system not initialized." << std::endl;
        return false;
    }
    
    if (x < 0 || x >= static_cast<int>(_grid.width()) ||
        y < 0 || y >= static_cast<int>(_grid.height())) {
        return false;
    }
    
    if (!_plantFactory->hasTemplate(species)) {
        return false;
    }
    
    Tile& tile = _grid(static_cast<unsigned int>(x), static_cast<unsigned int>(y));
    if (!tile.isPassable()) {
        return false;
    }
    
    Plant plant = _plantFactory->createFromTemplate(species, x, y);
    return tile.addPlant(std::make_shared<Plant>(std::move(plant)));
}

std::function<Plant(int, int)> PlantManager::selectPlantForBiome(Biome biome) {
    // Water biomes cannot support plants
    switch (biome) {
        case Biome::OCEAN_DEEP:
        case Biome::OCEAN_SHALLOW:
        case Biome::OCEAN_COAST:
        case Biome::FRESHWATER:
            return nullptr;
        default:
            break;
    }
    
    // Cold biomes - tundra moss (cold-adapted ground cover)
    switch (biome) {
        case Biome::ICE_SHEET:
        case Biome::TUNDRA:
        case Biome::TAIGA:
        case Biome::BOREAL_FOREST:
        case Biome::ALPINE_TUNDRA:
        case Biome::GLACIER:
            return [this](int x, int y) { return _biomeFactory->createTundraMoss(x, y); };
        default:
            break;
    }
    
    // Hot dry biomes - desert cactus (heat-tolerant succulents)
    switch (biome) {
        case Biome::DESERT_HOT:
        case Biome::DESERT_COLD:
        case Biome::STEPPE:
        case Biome::SHRUBLAND:
            return [this](int x, int y) { return _biomeFactory->createDesertCactus(x, y); };
        default:
            break;
    }
    
    // Tropical biomes - rainforest vine (lush fruit-bearers)
    switch (biome) {
        case Biome::TROPICAL_RAINFOREST:
        case Biome::TROPICAL_SEASONAL_FOREST:
        case Biome::SAVANNA:
            return [this](int x, int y) { return _biomeFactory->createRainforestVine(x, y); };
        default:
            break;
    }
    
    // Temperate biomes - mixed species based on random selection
    // These include: TEMPERATE_RAINFOREST, TEMPERATE_FOREST, TEMPERATE_GRASSLAND, ALPINE_MEADOW
    std::uniform_int_distribution<int> speciesDist(0, 3);
    int speciesChoice = speciesDist(_rng);
    
    switch (speciesChoice) {
        case 0:
            return [this](int x, int y) { return _plantFactory->createFromTemplate("berry_bush", x, y); };
        case 1:
            return [this](int x, int y) { return _plantFactory->createFromTemplate("oak_tree", x, y); };
        case 2:
            return [this](int x, int y) { return _plantFactory->createFromTemplate("grass", x, y); };
        default:
            return [this](int x, int y) { return _plantFactory->createFromTemplate("thorn_bush", x, y); };
    }
}

void PlantManager::addPlantsByBiome(unsigned rate) {
    if (!isInitialized()) {
        std::cerr << "[PlantManager] Error: Plant system not initialized. "
                  << "Call initialize() first." << std::endl;
        return;
    }
    
    if (!_environmentSystem || !_environmentSystem->hasClimateData()) {
        std::cerr << "[PlantManager] Error: No environment system or climate data. "
                  << "Cannot spawn plants by biome." << std::endl;
        return;
    }
    
    std::uniform_int_distribution<unsigned short> chanceDist(1, 100);
    
    unsigned cols = _grid.width();
    unsigned rows = _grid.height();
    
    // Track counts by biome category for logging
    unsigned tundraPlants = 0;
    unsigned desertPlants = 0;
    unsigned tropicalPlants = 0;
    unsigned temperatePlants = 0;
    unsigned skippedWater = 0;
    
    for (unsigned x = 0; x < cols; x++) {
        for (unsigned y = 0; y < rows; y++) {
            Tile& tile = _grid(x, y);
            
            // Skip impassable tiles (mountains, cliffs, etc.)
            if (!tile.isPassable()) {
                continue;
            }
            
            // Random chance of placing plant
            if (chanceDist(_rng) > rate) {
                continue;
            }
            
            // Get biome at this location
            int biomeInt = _environmentSystem->getBiome(static_cast<int>(x), static_cast<int>(y));
            Biome biome = static_cast<Biome>(biomeInt);
            
            // Select appropriate plant creator for this biome
            auto plantCreator = selectPlantForBiome(biome);
            
            // Skip water biomes
            if (!plantCreator) {
                ++skippedWater;
                continue;
            }
            
            // Create and add the plant
            Plant plant = plantCreator(static_cast<int>(x), static_cast<int>(y));
            tile.addPlant(std::make_shared<Plant>(std::move(plant)));
            
            // Track for logging
            switch (biome) {
                case Biome::ICE_SHEET:
                case Biome::TUNDRA:
                case Biome::TAIGA:
                case Biome::BOREAL_FOREST:
                case Biome::ALPINE_TUNDRA:
                case Biome::GLACIER:
                    ++tundraPlants;
                    break;
                case Biome::DESERT_HOT:
                case Biome::DESERT_COLD:
                case Biome::STEPPE:
                case Biome::SHRUBLAND:
                    ++desertPlants;
                    break;
                case Biome::TROPICAL_RAINFOREST:
                case Biome::TROPICAL_SEASONAL_FOREST:
                case Biome::SAVANNA:
                    ++tropicalPlants;
                    break;
                default:
                    ++temperatePlants;
                    break;
            }
        }
    }
    
    unsigned total = tundraPlants + desertPlants + tropicalPlants + temperatePlants;
    std::cout << "[PlantManager] Added " << total << " biome-appropriate plants:" << std::endl;
    std::cout << "  Tundra (moss): " << tundraPlants << std::endl;
    std::cout << "  Desert (cactus): " << desertPlants << std::endl;
    std::cout << "  Tropical (vine): " << tropicalPlants << std::endl;
    std::cout << "  Temperate (mixed): " << temperatePlants << std::endl;
    std::cout << "  Skipped water tiles: " << skippedWater << std::endl;
}

bool PlantManager::addBiomePlant(int x, int y) {
    if (!isInitialized()) {
        std::cerr << "[PlantManager] Error: Plant system not initialized." << std::endl;
        return false;
    }
    
    if (x < 0 || x >= static_cast<int>(_grid.width()) ||
        y < 0 || y >= static_cast<int>(_grid.height())) {
        return false;
    }
    
    Tile& tile = _grid(static_cast<unsigned int>(x), static_cast<unsigned int>(y));
    if (!tile.isPassable()) {
        return false;
    }
    
    // Get biome at this location - fall back to temperate if no environment system
    Biome biome = Biome::TEMPERATE_GRASSLAND;
    if (_environmentSystem && _environmentSystem->hasClimateData()) {
        int biomeInt = _environmentSystem->getBiome(x, y);
        biome = static_cast<Biome>(biomeInt);
    }
    
    // Select appropriate plant creator for this biome
    auto plantCreator = selectPlantForBiome(biome);
    
    // Water biomes cannot support plants
    if (!plantCreator) {
        return false;
    }
    
    // Create and add the plant
    Plant plant = plantCreator(x, y);
    return tile.addPlant(std::make_shared<Plant>(std::move(plant)));
}

//==============================================================================
// Lifecycle
//==============================================================================

void PlantManager::tick(unsigned currentTick) {
    if (!isInitialized()) {
        return;
    }
    
    unsigned cols = _grid.width();
    unsigned rows = _grid.height();
    
    // Collect seed dispersal events during iteration
    std::vector<std::pair<DispersalEvent, std::shared_ptr<Plant>>> dispersalEvents;
    
    // Update all plants on all tiles
    for (unsigned x = 0; x < cols; x++) {
        for (unsigned y = 0; y < rows; y++) {
            Tile& tile = _grid(x, y);
            
            // Get per-tile environment if available, otherwise use global fallback
            EnvironmentState tileEnv;
            if (_environmentSystem) {
                tileEnv = _environmentSystem->getEnvironmentStateAt(
                    static_cast<int>(x), static_cast<int>(y));
            } else {
                tileEnv = _currentEnvironment;
            }
            
            // Update plants with location-specific environment
            tile.updatePlants(tileEnv);
            
            // Remove dead plants
            tile.removeDeadPlants();
            
            // Handle seed dispersal for mature plants
            for (auto& plant : tile.getPlants()) {
                if (!plant || !plant->isAlive()) continue;
                
                // Emit plant scent if plant has scent production capability
                float scentRate = plant->getScentProductionRate();
                if (scentRate > 0.01f) {
                    std::array<float, 8> signature = plant->getScentSignature();
                    float intensity = scentRate * plant->getCurrentSize() / plant->getMaxSize();
                    
                    ScentDeposit plantScent(
                        ScentType::FOOD_TRAIL,
                        -1,
                        intensity,
                        signature,
                        currentTick,
                        50
                    );
                    
                    _scents.deposit(
                        static_cast<int>(x),
                        static_cast<int>(y),
                        plantScent
                    );
                }
                
                // Seed dispersal - mature plants attempt passive dispersal
                bool canDisperse = plant->canProduceFruit() || plant->canSpreadVegetatively();
                
                if (canDisperse) {
                    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                    
                    float dispersalChance;
                    if (plant->canSpreadVegetatively()) {
                        float sizeRatio = plant->getCurrentSize() / plant->getMaxSize();
                        dispersalChance = plant->getRunnerProduction() * 0.15f * sizeRatio;
                    } else {
                        dispersalChance = plant->getFruitProductionRate() * 0.1f;
                    }
                    
                    if (dist(_rng) < dispersalChance) {
                        DispersalEvent event = _seedDispersal.disperse(*plant, &tileEnv);
                        dispersalEvents.push_back({event, plant});
                    }
                    
                    plant->resetFruitTimer();
                }
            }
        }
    }
    
    // Process dispersal events - spawn new plants at target locations
    for (const auto& [event, parentPlant] : dispersalEvents) {
        if (event.targetX < 0 || event.targetX >= static_cast<int>(cols) ||
            event.targetY < 0 || event.targetY >= static_cast<int>(rows)) {
            continue;
        }
        
        Tile& targetTile = _grid(static_cast<unsigned int>(event.targetX),
                                 static_cast<unsigned int>(event.targetY));
        
        if (!targetTile.isPassable() || targetTile.getPlants().size() >= 3) {
            continue;
        }
        
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(_rng) > event.seedViability) {
            continue;
        }
        
        Plant offspring = _plantFactory->createOffspring(
            *parentPlant, *parentPlant,
            event.targetX, event.targetY
        );
        
        targetTile.addPlant(std::make_shared<Plant>(std::move(offspring)));
    }
}

//==============================================================================
// Environment
//==============================================================================

EnvironmentState& PlantManager::environment() {
    return _currentEnvironment;
}

const EnvironmentState& PlantManager::environment() const {
    return _currentEnvironment;
}

void PlantManager::updateEnvironment(float temperature, float lightLevel, float waterAvailability) {
    _currentEnvironment.temperature = temperature;
    _currentEnvironment.humidity = waterAvailability;
    _currentEnvironment.time_of_day = lightLevel;
}

//==============================================================================
// Access
//==============================================================================

PlantFactory* PlantManager::factory() {
    return _plantFactory.get();
}

const PlantFactory* PlantManager::factory() const {
    return _plantFactory.get();
}

std::shared_ptr<GeneRegistry> PlantManager::registry() {
    return _plantRegistry;
}

std::shared_ptr<const GeneRegistry> PlantManager::registry() const {
    return _plantRegistry;
}

} // namespace EcoSim
