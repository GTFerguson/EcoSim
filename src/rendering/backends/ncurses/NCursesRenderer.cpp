/**
 * @file NCursesRenderer.cpp
 * @brief Implementation of NCursesRenderer class
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the NCurses-based renderer, wrapping the existing
 * rendering code from main.cpp into the IRenderer interface.
 */

#include "../../../../include/rendering/backends/ncurses/NCursesRenderer.hpp"
#include "../../../../include/rendering/backends/ncurses/NCursesColorMapper.hpp"
#include "../../../../include/world/world.hpp"
#include "../../../../include/world/tile.hpp"
#include "../../../../include/objects/creature/creature.hpp"
#include "../../../../include/objects/food.hpp"
#include "../../../../include/objects/spawner.hpp"
#include "../../../../include/genetics/organisms/Plant.hpp"
#include "../../../../include/colorPairs.hpp"

#include <ncurses.h>

//==============================================================================
// Constructor / Destructor
//==============================================================================

NCursesRenderer::NCursesRenderer()
    : _initialized(false)
    , _rows(0)
    , _cols(0)
    , _hasColors(false) {
}

NCursesRenderer::~NCursesRenderer() {
    if (_initialized) {
        shutdown();
    }
}

//==============================================================================
// Lifecycle Methods
//==============================================================================

bool NCursesRenderer::initialize() {
    if (_initialized) {
        return true;  // Already initialized
    }
    
    // Initialize ncurses
    if (initscr() == nullptr) {
        return false;
    }
    
    // Setup input handling
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(0);  // Hide cursor
    
    // Enable colors
    if (has_colors()) {
        start_color();
        NCursesColorMapper::initializeColors();
        NCursesColorMapper::initializeColorPairs();
        _hasColors = true;
    }
    
    // Get initial screen dimensions
    getmaxyx(stdscr, _rows, _cols);
    
    _initialized = true;
    return true;
}

void NCursesRenderer::shutdown() {
    if (!_initialized) {
        return;
    }
    
    clear();
    endwin();
    _initialized = false;
}

void NCursesRenderer::beginFrame() {
    if (!_initialized) {
        return;
    }
    erase();
}

void NCursesRenderer::endFrame() {
    if (!_initialized) {
        return;
    }
    refresh();
}

//==============================================================================
// World Rendering Methods
//==============================================================================

void NCursesRenderer::renderWorld(const World& world, const Viewport& viewport) {
    if (!_initialized) {
        return;
    }
    
    // Get the grid from world (using const_cast temporarily since getGrid() isn't const)
    // This is safe because we're only reading from the grid
    World& mutableWorld = const_cast<World&>(world);
    std::vector<std::vector<Tile>>& grid = mutableWorld.getGrid();
    
    unsigned int mapRows = world.getRows();
    unsigned int mapCols = world.getCols();
    
    // Calculate rendering boundaries
    unsigned int startX = viewport.screenX;
    unsigned int startY = viewport.screenY;
    
    // Center the map if smaller than viewport
    if (viewport.width > mapCols) {
        startX += (viewport.width - mapCols) / 2;
    }
    if (viewport.height > mapRows) {
        startY += (viewport.height - mapRows) / 2;
    }
    
    // Calculate print boundaries
    unsigned int xRange = viewport.width + viewport.originX;
    unsigned int yRange = viewport.height + viewport.originY;
    if (xRange > mapCols) xRange = mapCols;
    if (yRange > mapRows) yRange = mapRows;
    
    // Screen offset for drawing
    int xScreen = static_cast<int>(startX) - viewport.originX;
    int yScreen = static_cast<int>(startY) - viewport.originY;
    
    // Render terrain, spawners, and food
    for (unsigned int y = viewport.originY; y < yRange; y++) {
        for (unsigned int x = viewport.originX; x < xRange; x++) {
            Tile* curTile = &grid.at(x).at(y);
            
            // Render terrain using TerrainType for renderer-agnostic colors
            int colorPair = NCursesColorMapper::terrainToColorPair(curTile->getTerrainType());
            attron(COLOR_PAIR(colorPair));
            mvaddch(yScreen + y, xScreen + x, curTile->getChar());
            attroff(COLOR_PAIR(colorPair));
            
            // Render spawners (trees) using EntityType
            const std::vector<Spawner>& spawners = curTile->getSpawners();
            if (!spawners.empty()) {
                int spawnerColor = NCursesColorMapper::entityToColorPair(spawners.begin()->getEntityType());
                attron(COLOR_PAIR(spawnerColor));
                mvaddch(yScreen + y, xScreen + x, spawners.begin()->getChar());
                attroff(COLOR_PAIR(spawnerColor));
            }
            
            // Render food using EntityType
            const std::vector<Food>& food = curTile->getFoodVec();
            if (!food.empty()) {
                int foodColor = NCursesColorMapper::entityToColorPair(food.begin()->getEntityType());
                attron(COLOR_PAIR(foodColor));
                mvaddch(yScreen + y, xScreen + x, food.begin()->getChar());
                attroff(COLOR_PAIR(foodColor));
            }
            
            // Render genetics-based plants (Phase 2.4)
            const auto& plants = curTile->getPlants();
            if (!plants.empty()) {
                const auto& plant = plants.front();
                if (plant && plant->isAlive()) {
                    int plantColor = NCursesColorMapper::entityToColorPair(plant->getEntityType());
                    attron(COLOR_PAIR(plantColor));
                    mvaddch(yScreen + y, xScreen + x, plant->getRenderCharacter());
                    attroff(COLOR_PAIR(plantColor));
                }
            }
        }
    }
}

void NCursesRenderer::renderTile(const Tile& tile, int screenX, int screenY) {
    if (!_initialized) {
        return;
    }
    
    // Render terrain using TerrainType for renderer-agnostic colors
    int colorPair = NCursesColorMapper::terrainToColorPair(tile.getTerrainType());
    attron(COLOR_PAIR(colorPair));
    mvaddch(screenY, screenX, tile.getChar());
    attroff(COLOR_PAIR(colorPair));
    
    // Render spawners using EntityType
    const std::vector<Spawner>& spawners = tile.getSpawners();
    if (!spawners.empty()) {
        int spawnerColor = NCursesColorMapper::entityToColorPair(spawners.begin()->getEntityType());
        attron(COLOR_PAIR(spawnerColor));
        mvaddch(screenY, screenX, spawners.begin()->getChar());
        attroff(COLOR_PAIR(spawnerColor));
    }
    
    // Render food using EntityType
    const std::vector<Food>& food = tile.getFoodVec();
    if (!food.empty()) {
        int foodColor = NCursesColorMapper::entityToColorPair(food.begin()->getEntityType());
        attron(COLOR_PAIR(foodColor));
        mvaddch(screenY, screenX, food.begin()->getChar());
        attroff(COLOR_PAIR(foodColor));
    }
    
    // Render genetics-based plants (Phase 2.4)
    const auto& plants = tile.getPlants();
    if (!plants.empty()) {
        const auto& plant = plants.front();
        if (plant && plant->isAlive()) {
            int plantColor = NCursesColorMapper::entityToColorPair(plant->getEntityType());
            attron(COLOR_PAIR(plantColor));
            mvaddch(screenY, screenX, plant->getRenderCharacter());
            attroff(COLOR_PAIR(plantColor));
        }
    }
}

void NCursesRenderer::renderCreatures(const std::vector<Creature>& creatures, 
                                      const Viewport& viewport) {
    if (!_initialized) {
        return;
    }
    
    // Calculate boundaries
    int xRange = viewport.originX + viewport.width;
    int yRange = viewport.originY + viewport.height;
    int xScreen = static_cast<int>(viewport.screenX) - viewport.originX;
    int yScreen = static_cast<int>(viewport.screenY) - viewport.originY;
    
    for (const Creature& creature : creatures) {
        int cX = creature.getX();
        int cY = creature.getY();
        
        // Check if creature is within the shown area of the map
        if (cX >= viewport.originX && cX < xRange &&
            cY >= viewport.originY && cY < yRange) {
            
            int cColor = getColorPairForProfile(creature);
            
            attron(COLOR_PAIR(cColor));
            mvaddch(yScreen + cY, xScreen + cX, creature.getChar());
            attroff(COLOR_PAIR(cColor));
        }
    }
}

void NCursesRenderer::renderCreature(const Creature& creature, 
                                    int screenX, int screenY) {
    if (!_initialized) {
        return;
    }
    
    int cColor = getColorPairForProfile(creature);
    
    attron(COLOR_PAIR(cColor));
    mvaddch(screenY, screenX, creature.getChar());
    attroff(COLOR_PAIR(cColor));
}

//==============================================================================
// UI Rendering Methods
//==============================================================================

void NCursesRenderer::renderHUD(const HUDData& data) {
    if (!_initialized) {
        return;
    }
    
    // Population statistics
    mvprintw(1, 2, "Population : %d", data.population);
    mvprintw(2, 2, "Births :     %d", data.births);
    mvprintw(3, 2, "Food Ate :   %d", data.foodEaten);
    
    // Death statistics
    mvprintw(5, 2, "Deaths");
    mvprintw(6, 2, "Old Age :    %d", data.deaths.oldAge);
    mvprintw(7, 2, "Starved :    %d", data.deaths.starved);
    mvprintw(8, 2, "Dehydrated : %d", data.deaths.dehydrated);
    mvprintw(9, 2, "Discomfort : %d", data.deaths.discomfort);
    mvprintw(10, 2, "Predator :   %d", data.deaths.predator);
    
    // Time display
    mvprintw(_rows - 1, 2, "%s", data.timeString.c_str());
    mvprintw(_rows - 1, 8, "%s", data.dateString.c_str());
    
    // Pause indicator
    if (data.paused) {
        mvprintw(_rows - 1, _cols - 10, "[PAUSED]");
    }
}

int NCursesRenderer::renderMenu(const std::string& title,
                                const std::vector<MenuOption>& options) {
    if (!_initialized || options.empty()) {
        return -1;
    }
    
    unsigned int startX = getScreenCenterX() - title.size() / 2;
    unsigned int startY = getScreenCenterY() - (options.size() + 1) / 2;
    unsigned int selected = 0;
    size_t optionCount = options.size();
    
    while (true) {
        clear();
        
        // Render title
        printCentered(title, startY);
        
        // Render options
        for (size_t i = 0; i < optionCount; i++) {
            unsigned int curY = startY + 2 + i;
            
            if (selected == i) {
                mvprintw(curY, startX + 3, ">>");
                attron(A_BLINK);
            }
            
            mvprintw(curY, startX + 6, "%s", options[i].text.c_str());
            
            if (selected == i) {
                attroff(A_BLINK);
            }
        }
        
        refresh();
        
        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
            case KEY_DOWN:
                if (selected < optionCount - 1) selected++;
                break;
            case 10:  // Enter key
                return static_cast<int>(selected);
            case 27:  // Escape key
                return -1;
        }
    }
}

void NCursesRenderer::renderWorldDetails(const World& world) {
    if (!_initialized) {
        return;
    }
    
    mvprintw(1,  2, "Seed      : %f", world.getSeed());
    mvprintw(2,  2, "Scale     : %f", world.getScale());
    mvprintw(3,  2, "Freq      : %f", world.getFreq());
    mvprintw(4,  2, "Exponent  : %f", world.getExponent());
    mvprintw(5,  2, "Terraces  : %d", world.getTerraces());
    
    mvprintw(7,  2, "OCTAVES");
    mvprintw(8,  2, "Quantity        : %d", world.getOctaveGen().quantity);
    mvprintw(9,  2, "Min Weight      : %f", world.getOctaveGen().minWeight);
    mvprintw(10, 2, "Max Weight      : %f", world.getOctaveGen().maxWeight);
    mvprintw(11, 2, "Freq. Interval  : %f", world.getOctaveGen().freqInterval);
}

void NCursesRenderer::renderMessage(const std::string& message, int row) {
    if (!_initialized) {
        return;
    }
    
    int targetRow = (row < 0) ? getScreenCenterY() : row;
    printCentered(message, targetRow);
}

//==============================================================================
// Screen Information Methods
//==============================================================================

unsigned int NCursesRenderer::getScreenWidth() const {
    return static_cast<unsigned int>(_cols);
}

unsigned int NCursesRenderer::getScreenHeight() const {
    return static_cast<unsigned int>(_rows);
}

unsigned int NCursesRenderer::getViewportMaxWidth() const {
    if (_cols > MAP_VERT_BORDER) {
        return static_cast<unsigned int>(_cols - MAP_VERT_BORDER);
    }
    return 0;
}

unsigned int NCursesRenderer::getViewportMaxHeight() const {
    if (_rows > MAP_HORI_BORDER) {
        return static_cast<unsigned int>(_rows - MAP_HORI_BORDER);
    }
    return 0;
}

unsigned int NCursesRenderer::getScreenCenterX() const {
    return static_cast<unsigned int>(_cols / 2);
}

unsigned int NCursesRenderer::getScreenCenterY() const {
    return static_cast<unsigned int>(_rows / 2);
}

//==============================================================================
// Capability Query Methods
//==============================================================================

RendererCapabilities NCursesRenderer::getCapabilities() const {
    RendererCapabilities caps;
    caps.supportsColor = _hasColors;
    caps.supportsSprites = false;
    caps.supportsMouse = false;  // Could enable ncurses mouse later
    caps.supportsResizing = true;
    caps.supportsUnicode = false;  // Basic ASCII only for now
    return caps;
}

bool NCursesRenderer::supportsColor() const {
    return _hasColors;
}

bool NCursesRenderer::supportsSprites() const {
    return false;
}

std::string NCursesRenderer::getName() const {
    return "NCurses Terminal Renderer";
}

//==============================================================================
// NCurses-Specific Methods
//==============================================================================

void NCursesRenderer::setHalfDelay(int tenths) {
    if (_initialized) {
        halfdelay(tenths);
    }
}

void NCursesRenderer::updateDimensions() {
    if (_initialized) {
        getmaxyx(stdscr, _rows, _cols);
    }
}

//==============================================================================
// Helper Methods
//==============================================================================

void NCursesRenderer::printCentered(const std::string& str, int y) {
    int x = getScreenCenterX() - str.length() / 2;
    mvprintw(y, x, "%s", str.c_str());
}

int NCursesRenderer::getColorPairForProfile(const Creature& creature) const {
    // Map from the Profile enum to color pairs
    switch (creature.getProfile()) {
        case Profile::hungry:   return HUNGRY_PAIR;
        case Profile::thirsty:  return THIRSTY_PAIR;
        case Profile::sleep:    return SLEEP_PAIR;
        case Profile::breed:    return BREED_PAIR;
        case Profile::migrate:  return MIGRATE_PAIR;
        default:                return DEFAULT_PAIR;
    }
}