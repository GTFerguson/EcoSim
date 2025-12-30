/**
 * @file PlantManager.cpp
 * @brief Implementation of PlantManager for genetics-based plant infrastructure
 */

#include "../../include/world/PlantManager.hpp"

namespace EcoSim {

using namespace Genetics;

//==============================================================================
// Construction
//==============================================================================

PlantManager::PlantManager(WorldGrid& grid, ScentLayer& scents)
    : _grid(grid)
    , _scents(scents)
    , _rng(std::random_device{}()) {
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
            
            // Update plants with current environment
            tile.updatePlants(_currentEnvironment);
            
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
                        DispersalEvent event = _seedDispersal.disperse(*plant, &_currentEnvironment);
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
