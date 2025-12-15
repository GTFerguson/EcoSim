/**
 * @file SDL2Renderer.cpp
 * @brief Implementation of SDL2Renderer class
 * @author Gary Ferguson
 * @date December 2024
 * 
 * This file implements the SDL2-based renderer, providing hardware-accelerated
 * graphical rendering for the ecological simulation.
 */

#include "../../../../include/rendering/backends/sdl2/SDL2Renderer.hpp"
#include "../../../../include/rendering/backends/sdl2/SDL2ColorMapper.hpp"
#include "../../../../include/world/world.hpp"
#include "../../../../include/world/tile.hpp"
#include "../../../../include/objects/creature/creature.hpp"
#include "../../../../include/objects/food.hpp"
#include "../../../../include/objects/spawner.hpp"

#include <iostream>
#include <sstream>

//==============================================================================
// Constructor / Destructor
//==============================================================================

SDL2Renderer::SDL2Renderer()
    : _window(nullptr)
    , _renderer(nullptr)
    , _initialized(false)
    , _screenWidth(DEFAULT_SCREEN_WIDTH)
    , _screenHeight(DEFAULT_SCREEN_HEIGHT)
    , _tileSize(DEFAULT_TILE_SIZE) {
}

SDL2Renderer::~SDL2Renderer() {
    if (_initialized) {
        shutdown();
    }
}

//==============================================================================
// Lifecycle Methods
//==============================================================================

bool SDL2Renderer::initialize() {
    if (_initialized) {
        return true;  // Already initialized
    }
    
    // Initialize SDL2 video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL2Renderer: SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window
    _window = SDL_CreateWindow(
        "EcoSim - Ecological Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        _screenWidth,
        _screenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (_window == nullptr) {
        std::cerr << "SDL2Renderer: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    // Create hardware-accelerated renderer with vsync
    _renderer = SDL_CreateRenderer(
        _window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (_renderer == nullptr) {
        std::cerr << "SDL2Renderer: SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
        return false;
    }
    
    // Enable alpha blending
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    
    _initialized = true;
    return true;
}

void SDL2Renderer::shutdown() {
    if (!_initialized) {
        return;
    }
    
    if (_renderer != nullptr) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    
    if (_window != nullptr) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    
    SDL_Quit();
    _initialized = false;
}

void SDL2Renderer::beginFrame() {
    if (!_initialized) {
        return;
    }
    
    // Clear screen with dark background
    SDL_SetRenderDrawColor(_renderer, 20, 20, 30, 255);
    SDL_RenderClear(_renderer);
}

void SDL2Renderer::endFrame() {
    if (!_initialized) {
        return;
    }
    
    SDL_RenderPresent(_renderer);
}

//==============================================================================
// World Rendering Methods
//==============================================================================

void SDL2Renderer::renderWorld(const World& world, const Viewport& viewport) {
    if (!_initialized) {
        return;
    }
    
    // Get the grid from world (using const_cast temporarily since getGrid() isn't const)
    World& mutableWorld = const_cast<World&>(world);
    std::vector<std::vector<Tile>>& grid = mutableWorld.getGrid();
    
    unsigned int mapRows = world.getRows();
    unsigned int mapCols = world.getCols();
    
    // Calculate print boundaries
    unsigned int xRange = viewport.width + viewport.originX;
    unsigned int yRange = viewport.height + viewport.originY;
    if (xRange > mapCols) xRange = mapCols;
    if (yRange > mapRows) yRange = mapRows;
    
    // Screen offset for drawing - convert from tile units to pixels
    // viewport.screenX/Y are in tile units (like NCurses character units)
    int baseScreenX = static_cast<int>(viewport.screenX) * _tileSize;
    int baseScreenY = static_cast<int>(viewport.screenY) * _tileSize;
    
    // Render terrain, spawners, and food
    for (unsigned int y = viewport.originY; y < yRange; y++) {
        for (unsigned int x = viewport.originX; x < xRange; x++) {
            Tile* curTile = &grid.at(x).at(y);
            
            // Calculate screen position (offset already in pixels, add tile offset)
            int screenX = baseScreenX + (x - viewport.originX) * _tileSize;
            int screenY = baseScreenY + (y - viewport.originY) * _tileSize;
            
            // Render terrain
            SDL_Color terrainColor = getTerrainColor(curTile->getTerrainType());
            drawFilledRect(screenX, screenY, _tileSize, _tileSize, terrainColor);
            
            // Render spawners (trees) - draw as smaller dark green square
            const std::vector<Spawner>& spawners = curTile->getSpawners();
            if (!spawners.empty()) {
                SDL_Color spawnerColor = getEntityColor(spawners.begin()->getEntityType());
                int padding = _tileSize / 4;
                drawFilledRect(screenX + padding, screenY + padding, 
                              _tileSize - 2 * padding, _tileSize - 2 * padding, 
                              spawnerColor);
            }
            
            // Render food - draw as small colored circles (approximated as squares)
            const std::vector<Food>& food = curTile->getFoodVec();
            if (!food.empty()) {
                SDL_Color foodColor = getEntityColor(food.begin()->getEntityType());
                int padding = _tileSize / 3;
                drawFilledRect(screenX + padding, screenY + padding,
                              _tileSize - 2 * padding, _tileSize - 2 * padding,
                              foodColor);
            }
        }
    }
}

void SDL2Renderer::renderTile(const Tile& tile, int screenX, int screenY) {
    if (!_initialized) {
        return;
    }
    
    // Render terrain
    SDL_Color terrainColor = getTerrainColor(tile.getTerrainType());
    drawFilledRect(screenX, screenY, _tileSize, _tileSize, terrainColor);
    
    // Render spawners
    const std::vector<Spawner>& spawners = tile.getSpawners();
    if (!spawners.empty()) {
        SDL_Color spawnerColor = getEntityColor(spawners.begin()->getEntityType());
        int padding = _tileSize / 4;
        drawFilledRect(screenX + padding, screenY + padding,
                      _tileSize - 2 * padding, _tileSize - 2 * padding,
                      spawnerColor);
    }
    
    // Render food
    const std::vector<Food>& food = tile.getFoodVec();
    if (!food.empty()) {
        SDL_Color foodColor = getEntityColor(food.begin()->getEntityType());
        int padding = _tileSize / 3;
        drawFilledRect(screenX + padding, screenY + padding,
                      _tileSize - 2 * padding, _tileSize - 2 * padding,
                      foodColor);
    }
}

void SDL2Renderer::renderCreatures(const std::vector<Creature>& creatures,
                                   const Viewport& viewport) {
    if (!_initialized) {
        return;
    }
    
    // Calculate boundaries
    int xRange = viewport.originX + viewport.width;
    int yRange = viewport.originY + viewport.height;
    // Convert from tile units to pixels (same as renderWorld)
    int baseScreenX = static_cast<int>(viewport.screenX) * _tileSize;
    int baseScreenY = static_cast<int>(viewport.screenY) * _tileSize;
    
    for (const Creature& creature : creatures) {
        int cX = creature.getX();
        int cY = creature.getY();
        
        // Check if creature is within the shown area of the map
        if (cX >= viewport.originX && cX < xRange &&
            cY >= viewport.originY && cY < yRange) {
            
            // Calculate screen position (offset already in pixels, add tile offset)
            int screenX = baseScreenX + (cX - viewport.originX) * _tileSize;
            int screenY = baseScreenY + (cY - viewport.originY) * _tileSize;
            
            renderCreature(creature, screenX, screenY);
        }
    }
}

void SDL2Renderer::renderCreature(const Creature& creature, int screenX, int screenY) {
    if (!_initialized) {
        return;
    }
    
    // Get color based on creature's behavior profile
    SDL_Color creatureColor = getProfileColor(creature);
    
    // Draw creature as a smaller rectangle within the tile (with padding)
    int padding = 2;
    drawFilledRect(screenX + padding, screenY + padding,
                  _tileSize - 2 * padding, _tileSize - 2 * padding,
                  creatureColor);
    
    // Draw a darker outline
    SDL_Color outlineColor = {
        static_cast<Uint8>(creatureColor.r / 2),
        static_cast<Uint8>(creatureColor.g / 2),
        static_cast<Uint8>(creatureColor.b / 2),
        255
    };
    drawRect(screenX + padding, screenY + padding,
            _tileSize - 2 * padding, _tileSize - 2 * padding,
            outlineColor);
}

//==============================================================================
// UI Rendering Methods
//==============================================================================

void SDL2Renderer::renderHUD(const HUDData& data) {
    if (!_initialized) {
        return;
    }
    
    // Draw HUD background at bottom of screen
    SDL_Color hudBgColor = {40, 40, 50, 230};
    drawFilledRect(0, _screenHeight - HUD_HEIGHT, _screenWidth, HUD_HEIGHT, hudBgColor);
    
    // Draw a separator line
    SDL_Color lineColor = {100, 100, 120, 255};
    drawFilledRect(0, _screenHeight - HUD_HEIGHT, _screenWidth, 2, lineColor);
    
    // Text positions
    int textY = _screenHeight - HUD_HEIGHT + HUD_PADDING;
    int col1X = HUD_PADDING;
    int col2X = 200;
    int col3X = 400;
    int lineHeight = 18;
    
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color valueColor = {200, 200, 100, 255};
    SDL_Color labelColor = {150, 150, 150, 255};
    
    // Column 1: Population statistics
    drawText("POPULATION", col1X, textY, labelColor);
    drawText("Count: " + std::to_string(data.population), col1X, textY + lineHeight, textColor);
    drawText("Births: " + std::to_string(data.births), col1X, textY + 2 * lineHeight, textColor);
    drawText("Food Eaten: " + std::to_string(data.foodEaten), col1X, textY + 3 * lineHeight, textColor);
    
    // Column 2: Death statistics
    drawText("DEATHS", col2X, textY, labelColor);
    drawText("Old Age: " + std::to_string(data.deaths.oldAge), col2X, textY + lineHeight, textColor);
    drawText("Starved: " + std::to_string(data.deaths.starved), col2X, textY + 2 * lineHeight, textColor);
    drawText("Dehydrated: " + std::to_string(data.deaths.dehydrated), col2X, textY + 3 * lineHeight, textColor);
    drawText("Discomfort: " + std::to_string(data.deaths.discomfort), col2X, textY + 4 * lineHeight, textColor);
    drawText("Predator: " + std::to_string(data.deaths.predator), col2X, textY + 5 * lineHeight, textColor);
    
    // Column 3: Time and status
    drawText("TIME", col3X, textY, labelColor);
    drawText(data.timeString, col3X, textY + lineHeight, valueColor);
    drawText(data.dateString, col3X, textY + 2 * lineHeight, textColor);
    
    // World info
    std::stringstream worldInfo;
    worldInfo << "World: " << data.worldWidth << "x" << data.worldHeight;
    drawText(worldInfo.str(), col3X, textY + 4 * lineHeight, textColor);
    
    // Pause indicator
    if (data.paused) {
        SDL_Color pauseColor = {255, 100, 100, 255};
        drawText("[PAUSED]", _screenWidth - 100, textY, pauseColor);
    }
}

int SDL2Renderer::renderMenu(const std::string& title,
                             const std::vector<MenuOption>& options) {
    if (!_initialized || options.empty()) {
        return -1;
    }
    
    int selected = 0;
    int optionCount = static_cast<int>(options.size());
    
    // Menu dimensions
    int menuWidth = 400;
    int menuHeight = 60 + optionCount * 40;
    int menuX = (_screenWidth - menuWidth) / 2;
    int menuY = (_screenHeight - menuHeight) / 2;
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return -1;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        if (selected > 0) selected--;
                        break;
                    case SDLK_DOWN:
                        if (selected < optionCount - 1) selected++;
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        return selected;
                    case SDLK_ESCAPE:
                        return -1;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                
                // Check if click is on a menu option
                for (int i = 0; i < optionCount; i++) {
                    int optionY = menuY + 50 + i * 40;
                    if (mouseX >= menuX && mouseX <= menuX + menuWidth &&
                        mouseY >= optionY && mouseY <= optionY + 35) {
                        return i;
                    }
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                
                // Update selection based on mouse position
                for (int i = 0; i < optionCount; i++) {
                    int optionY = menuY + 50 + i * 40;
                    if (mouseX >= menuX && mouseX <= menuX + menuWidth &&
                        mouseY >= optionY && mouseY <= optionY + 35) {
                        selected = i;
                        break;
                    }
                }
            }
        }
        
        // Render
        beginFrame();
        
        // Draw menu background
        SDL_Color menuBgColor = {50, 50, 60, 240};
        drawFilledRect(menuX, menuY, menuWidth, menuHeight, menuBgColor);
        
        // Draw border
        SDL_Color borderColor = {100, 100, 120, 255};
        drawRect(menuX, menuY, menuWidth, menuHeight, borderColor);
        
        // Draw title
        SDL_Color titleColor = {255, 255, 200, 255};
        int titleX = menuX + (menuWidth - title.length() * 10) / 2;
        drawText(title, titleX, menuY + 15, titleColor);
        
        // Draw options
        for (int i = 0; i < optionCount; i++) {
            int optionY = menuY + 50 + i * 40;
            
            // Highlight selected option
            if (i == selected) {
                SDL_Color highlightColor = {80, 80, 100, 200};
                drawFilledRect(menuX + 10, optionY, menuWidth - 20, 35, highlightColor);
                
                // Draw selection indicator
                SDL_Color indicatorColor = {255, 255, 100, 255};
                drawText(">>", menuX + 20, optionY + 8, indicatorColor);
            }
            
            // Draw option text
            SDL_Color optionColor = options[i].enabled ? 
                SDL_Color{255, 255, 255, 255} : SDL_Color{100, 100, 100, 255};
            drawText(options[i].text, menuX + 50, optionY + 8, optionColor);
        }
        
        endFrame();
        
        // Small delay to avoid high CPU usage
        SDL_Delay(16);
    }
    
    return -1;
}

void SDL2Renderer::renderWorldDetails(const World& world) {
    if (!_initialized) {
        return;
    }
    
    // Draw world details overlay in top-left corner
    SDL_Color bgColor = {30, 30, 40, 220};
    drawFilledRect(5, 5, 250, 200, bgColor);
    
    SDL_Color labelColor = {150, 150, 150, 255};
    SDL_Color valueColor = {255, 255, 255, 255};
    
    int y = 15;
    int lineHeight = 18;
    int labelX = 15;
    int valueX = 120;
    
    drawText("WORLD DETAILS", labelX, y, {255, 255, 200, 255});
    y += lineHeight + 5;
    
    std::stringstream ss;
    
    drawText("Seed:", labelX, y, labelColor);
    ss << world.getSeed();
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
    
    drawText("Scale:", labelX, y, labelColor);
    ss << world.getScale();
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
    
    drawText("Frequency:", labelX, y, labelColor);
    ss << world.getFreq();
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
    
    drawText("Exponent:", labelX, y, labelColor);
    ss << world.getExponent();
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
    
    drawText("Terraces:", labelX, y, labelColor);
    ss << world.getTerraces();
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
    
    y += 5;
    drawText("OCTAVES", labelX, y, {255, 255, 200, 255});
    y += lineHeight;
    
    drawText("Quantity:", labelX, y, labelColor);
    ss << world.getOctaveGen().quantity;
    drawText(ss.str(), valueX, y, valueColor);
    y += lineHeight;
    ss.str("");
}

void SDL2Renderer::renderMessage(const std::string& message, int row) {
    if (!_initialized) {
        return;
    }
    
    // Calculate position
    int textWidth = message.length() * 10;  // Approximate
    int x = (_screenWidth - textWidth) / 2;
    int y = (row < 0) ? _screenHeight / 2 : row;
    
    // Draw background
    SDL_Color bgColor = {40, 40, 50, 220};
    drawFilledRect(x - 20, y - 10, textWidth + 40, 30, bgColor);
    
    // Draw text
    SDL_Color textColor = {255, 255, 255, 255};
    drawText(message, x, y, textColor);
}

//==============================================================================
// Screen Information Methods
//==============================================================================

unsigned int SDL2Renderer::getScreenWidth() const {
    return static_cast<unsigned int>(_screenWidth);
}

unsigned int SDL2Renderer::getScreenHeight() const {
    return static_cast<unsigned int>(_screenHeight);
}

unsigned int SDL2Renderer::getViewportMaxWidth() const {
    // Available width divided by tile size
    return static_cast<unsigned int>(_screenWidth / _tileSize);
}

unsigned int SDL2Renderer::getViewportMaxHeight() const {
    // Available height (minus HUD) divided by tile size
    int availableHeight = _screenHeight - HUD_HEIGHT;
    return static_cast<unsigned int>(availableHeight / _tileSize);
}

unsigned int SDL2Renderer::getScreenCenterX() const {
    // Return in tile units for consistency with NCurses (character units)
    // This matches how main.cpp calculates viewport positions
    return getViewportMaxWidth() / 2;
}

unsigned int SDL2Renderer::getScreenCenterY() const {
    // Return in tile units for consistency with NCurses (character units)
    // This matches how main.cpp calculates viewport positions
    return getViewportMaxHeight() / 2;
}

//==============================================================================
// Capability Query Methods
//==============================================================================

RendererCapabilities SDL2Renderer::getCapabilities() const {
    RendererCapabilities caps;
    caps.supportsColor = true;
    caps.supportsSprites = true;
    caps.supportsMouse = true;
    caps.supportsResizing = true;
    caps.supportsUnicode = true;
    return caps;
}

bool SDL2Renderer::supportsColor() const {
    return true;
}

bool SDL2Renderer::supportsSprites() const {
    return true;
}

std::string SDL2Renderer::getName() const {
    return "SDL2 Graphics Renderer";
}

//==============================================================================
// SDL2-Specific Methods
//==============================================================================

void SDL2Renderer::setTileSize(int size) {
    if (size > 0 && size <= 64) {
        _tileSize = size;
    }
}

void SDL2Renderer::handleResize() {
    if (_initialized && _window != nullptr) {
        SDL_GetWindowSize(_window, &_screenWidth, &_screenHeight);
    }
}

//==============================================================================
// Helper Methods
//==============================================================================

void SDL2Renderer::drawFilledRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(_renderer, &rect);
}

void SDL2Renderer::drawRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(_renderer, &rect);
}

void SDL2Renderer::drawText(const std::string& text, int x, int y, SDL_Color color) {
    // Bitmap font rendering - 5x7 pixel characters
    // Each character is defined as a 5x7 bitmap where each row is a byte
    // Bit pattern: most significant bit (within the 5-bit width) is leftmost pixel
    
    static const unsigned char FONT_5X7[128][7] = {
        // Control characters (0-31) - blank
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
        // Space (32)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        // ! (33)
        {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
        // " (34)
        {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},
        // # (35)
        {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00},
        // $ (36)
        {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04},
        // % (37)
        {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13},
        // & (38)
        {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D},
        // ' (39)
        {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
        // ( (40)
        {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
        // ) (41)
        {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
        // * (42)
        {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
        // + (43)
        {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
        // , (44)
        {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},
        // - (45)
        {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
        // . (46)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00},
        // / (47)
        {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10},
        // 0 (48)
        {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
        // 1 (49)
        {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
        // 2 (50)
        {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F},
        // 3 (51)
        {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E},
        // 4 (52)
        {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
        // 5 (53)
        {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
        // 6 (54)
        {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
        // 7 (55)
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
        // 8 (56)
        {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
        // 9 (57)
        {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
        // : (58)
        {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00},
        // ; (59)
        {0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x08},
        // < (60)
        {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
        // = (61)
        {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
        // > (62)
        {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
        // ? (63)
        {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
        // @ (64)
        {0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E},
        // A (65)
        {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
        // B (66)
        {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
        // C (67)
        {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
        // D (68)
        {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C},
        // E (69)
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
        // F (70)
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
        // G (71)
        {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
        // H (72)
        {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
        // I (73)
        {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
        // J (74)
        {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
        // K (75)
        {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
        // L (76)
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
        // M (77)
        {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
        // N (78)
        {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
        // O (79)
        {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
        // P (80)
        {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
        // Q (81)
        {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
        // R (82)
        {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
        // S (83)
        {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E},
        // T (84)
        {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        // U (85)
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
        // V (86)
        {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04},
        // W (87)
        {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11},
        // X (88)
        {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
        // Y (89)
        {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
        // Z (90)
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
        // [ (91)
        {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E},
        // \ (92)
        {0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01},
        // ] (93)
        {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E},
        // ^ (94)
        {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00},
        // _ (95)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
        // ` (96)
        {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
        // a (97)
        {0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F},
        // b (98)
        {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x1E},
        // c (99)
        {0x00, 0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E},
        // d (100)
        {0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F},
        // e (101)
        {0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E},
        // f (102)
        {0x06, 0x08, 0x1C, 0x08, 0x08, 0x08, 0x08},
        // g (103)
        {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E},
        // h (104)
        {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x11},
        // i (105)
        {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E},
        // j (106)
        {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C},
        // k (107)
        {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12},
        // l (108)
        {0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
        // m (109)
        {0x00, 0x00, 0x1A, 0x15, 0x15, 0x11, 0x11},
        // n (110)
        {0x00, 0x00, 0x1E, 0x11, 0x11, 0x11, 0x11},
        // o (111)
        {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E},
        // p (112)
        {0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10},
        // q (113)
        {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01},
        // r (114)
        {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10},
        // s (115)
        {0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E},
        // t (116)
        {0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06},
        // u (117)
        {0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0F},
        // v (118)
        {0x00, 0x00, 0x11, 0x11, 0x0A, 0x0A, 0x04},
        // w (119)
        {0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A},
        // x (120)
        {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11},
        // y (121)
        {0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E},
        // z (122)
        {0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F},
        // { (123)
        {0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02},
        // | (124)
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        // } (125)
        {0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08},
        // ~ (126)
        {0x00, 0x08, 0x15, 0x02, 0x00, 0x00, 0x00},
        // DEL (127) - blank
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };
    
    const int charWidth = 5;
    const int charHeight = 7;
    const int charSpacing = 1;  // Space between characters
    const int scale = 2;  // Scale up for better visibility
    
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
    
    int cursorX = x;
    
    for (size_t i = 0; i < text.length(); i++) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        
        // Clamp to valid ASCII range
        if (c > 127) c = '?';
        
        // Get the glyph
        const unsigned char* glyph = FONT_5X7[c];
        
        // Render each pixel of the character
        for (int row = 0; row < charHeight; row++) {
            unsigned char rowData = glyph[row];
            for (int col = 0; col < charWidth; col++) {
                if (rowData & (1 << (charWidth - 1 - col))) {
                    // Draw scaled pixel
                    SDL_Rect pixel = {
                        cursorX + col * scale,
                        y + row * scale,
                        scale,
                        scale
                    };
                    SDL_RenderFillRect(_renderer, &pixel);
                }
            }
        }
        
        // Move cursor for next character
        cursorX += (charWidth + charSpacing) * scale;
    }
}

SDL_Color SDL2Renderer::getTerrainColor(TerrainType terrain) const {
    return SDL2ColorMapper::terrainToColor(terrain);
}

SDL_Color SDL2Renderer::getProfileColor(BehaviorProfile profile) const {
    return SDL2ColorMapper::profileToColor(profile);
}

SDL_Color SDL2Renderer::getProfileColor(const Creature& creature) const {
    return SDL2ColorMapper::creatureProfileToColor(creature);
}

SDL_Color SDL2Renderer::getEntityColor(EntityType entity) const {
    return SDL2ColorMapper::entityToColor(entity);
}