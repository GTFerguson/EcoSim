/**
 * @file ImGuiOverlay.cpp
 * @brief Implementation of Dear ImGui overlay for debug UI panels
 * @author Gary Ferguson
 * @date December 2025
 */

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "rendering/backends/sdl2/ImGuiOverlay.hpp"
#include "world/world.hpp"
#include "objects/creature/creature.hpp"
#include "objects/creature/genome.hpp"

// New genetics system includes
#include "genetics/core/Genome.hpp"
#include "genetics/expression/Phenotype.hpp"
#include "genetics/expression/EnergyBudget.hpp"
#include "genetics/defaults/UniversalGenes.hpp"
#include "genetics/organisms/Plant.hpp"

#include <cstring>
#include <algorithm>
#include <cfloat>  // For FLT_MIN
#include <cstdio>  // For debug logging

//==============================================================================
// Constructor / Destructor
//==============================================================================

ImGuiOverlay::ImGuiOverlay()
    : _window(nullptr)
    , _renderer(nullptr)
    , _context(nullptr)
    , _initialized(false)
    , _showStatistics(true)
    , _showWorldInfo(false)
    , _showCreatureList(true)
    , _showPerformance(false)
    , _showCreatureInspector(false)
    , _showPlantInspector(false)
    , _showControls(false)
    , _showDemo(false)
    , _selectedCreatureId(-1)
    , _selectedPlantId(-1)
    , _creatureSortMode(0)
    , _frameTimeIndex(0)
    , _historyIndex(0)
    , _lastBirths(0)
    , _lastDeaths(0)
{
    // Initialize filter text
    std::memset(_creatureFilterText, 0, sizeof(_creatureFilterText));
    
    // Initialize frame time history to zero
    for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; i++) {
        _frameTimes[i] = 0.0f;
    }
    
    // Initialize population dynamics history to zero
    for (int i = 0; i < HISTORY_SIZE; i++) {
        _populationHistory[i] = 0.0f;
        _birthsHistory[i] = 0.0f;
        _deathsHistory[i] = 0.0f;
    }
    
    // Initialize last HUD data
    _lastHudData = HUDData();
}

ImGuiOverlay::~ImGuiOverlay() {
    shutdown();
}

//==============================================================================
// Lifecycle Methods
//==============================================================================

bool ImGuiOverlay::initialize(SDL_Window* window, SDL_Renderer* renderer) {
    if (_initialized) {
        return true;
    }
    
    if (!window || !renderer) {
        return false;
    }
    
    _window = window;
    _renderer = renderer;
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    _context = ImGui::CreateContext();
    if (!_context) {
        return false;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard navigation
    
    // Disable saving imgui.ini (optional - prevents cluttering the project)
    io.IniFilename = nullptr;
    
    // Setup custom style
    setupStyle();
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer)) {
        ImGui::DestroyContext(_context);
        _context = nullptr;
        return false;
    }
    
    if (!ImGui_ImplSDLRenderer2_Init(_renderer)) {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(_context);
        _context = nullptr;
        return false;
    }
    
    _initialized = true;
    return true;
}

void ImGuiOverlay::shutdown() {
    if (!_initialized) {
        return;
    }
    
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext(_context);
    
    _context = nullptr;
    _window = nullptr;
    _renderer = nullptr;
    _initialized = false;
}

//==============================================================================
// Rendering Methods
//==============================================================================

void ImGuiOverlay::beginFrame() {
    if (!_initialized) {
        return;
    }
    
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiOverlay::render(const HUDData& hudData, const World* world,
                         const std::vector<Creature>* creatures) {
    if (!_initialized) {
        return;
    }
    
    // Store last HUD data for controls panel
    _lastHudData = hudData;
    
    // Render main menu bar
    renderMainMenuBar();
    
    // Render windows based on visibility flags
    if (_showStatistics) {
        renderStatisticsWindow(hudData);
    }
    
    if (_showWorldInfo) {
        renderWorldInfoWindow(world, creatures);
    }
    
    if (_showPerformance) {
        renderPerformanceWindow();
    }
    
    if (_showCreatureList && creatures) {
        renderCreatureListWindow(creatures);
    }
    
    // Find and render selected creature inspector
    if (_showCreatureInspector && creatures && _selectedCreatureId >= 0) {
        const Creature* selectedCreature = nullptr;
        
        // Find creature by unique ID (not vector index)
        for (const auto& c : *creatures) {
            if (c.getId() == _selectedCreatureId) {
                selectedCreature = &c;
                break;
            }
        }
        
        if (selectedCreature) {
            renderCreatureInspectorWindow(selectedCreature);
        } else {
            // Creature no longer exists (died), deselect
            _selectedCreatureId = -1;
        }
    }
    
    if (_showControls) {
        renderControlsWindow();
    }
    
    // Show ImGui demo window for development reference
    if (_showDemo) {
        ImGui::ShowDemoWindow(&_showDemo);
    }
}

void ImGuiOverlay::endFrame() {
    if (!_initialized) {
        return;
    }
    
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
}

//==============================================================================
// Private Rendering Methods
//==============================================================================

void ImGuiOverlay::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Statistics", "F1", &_showStatistics);
            ImGui::MenuItem("World Info", "F2", &_showWorldInfo);
            ImGui::MenuItem("Performance", "F3", &_showPerformance);
            ImGui::MenuItem("Creature List", "F4", &_showCreatureList);
            ImGui::MenuItem("Creature Inspector", "F5", &_showCreatureInspector);
            ImGui::MenuItem("Controls", "F6", &_showControls);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &_showDemo);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Reset Layout")) {
                // Reset window positions on next frame
                // This could be expanded to save/load layouts
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show All Panels")) {
                _showStatistics = true;
                _showWorldInfo = true;
                _showPerformance = true;
                _showCreatureList = true;
                _showControls = true;
            }
            if (ImGui::MenuItem("Hide All Panels")) {
                _showStatistics = false;
                _showWorldInfo = false;
                _showPerformance = false;
                _showCreatureList = false;
                _showCreatureInspector = false;
                _showControls = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About EcoSim")) {
                // Could show about dialog
            }
            if (ImGui::MenuItem("Controls")) {
                // Could show controls help
            }
            ImGui::EndMenu();
        }
        
        // Display quick stats on menu bar
        ImGui::Separator();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        
        // Show pause status
        if (_lastHudData.paused) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "| PAUSED");
        }
        
        // Show population
        ImGui::SameLine();
        ImGui::Text("| Pop: %d", _lastHudData.population);
        
        ImGui::EndMainMenuBar();
    }
}

void ImGuiOverlay::renderStatisticsWindow(const HUDData& hudData) {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 650), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Simulation Statistics", &_showStatistics, ImGuiWindowFlags_NoCollapse)) {
        // Time display
        ImGui::Text("Time: %s", hudData.timeString.c_str());
        ImGui::Text("Date: %s", hudData.dateString.c_str());
        
        ImGui::Separator();
        
        // Current population
        ImGui::Text("Current Population: %d", hudData.population);
        
        // Calculate current total deaths
        unsigned int currentTotalDeaths = hudData.deaths.oldAge + hudData.deaths.starved +
                                         hudData.deaths.dehydrated + hudData.deaths.discomfort +
                                         hudData.deaths.predator;
        
        // Calculate per-frame changes
        int birthsThisFrame = static_cast<int>(hudData.births) - static_cast<int>(_lastBirths);
        int deathsThisFrame = static_cast<int>(currentTotalDeaths) - static_cast<int>(_lastDeaths);
        int netChange = birthsThisFrame - deathsThisFrame;
        
        // Clamp to non-negative (in case of reset or first frame)
        if (birthsThisFrame < 0) birthsThisFrame = 0;
        if (deathsThisFrame < 0) deathsThisFrame = 0;
        
        // Update history arrays
        _populationHistory[_historyIndex] = static_cast<float>(hudData.population);
        _birthsHistory[_historyIndex] = static_cast<float>(birthsThisFrame);
        _deathsHistory[_historyIndex] = static_cast<float>(deathsThisFrame);
        
        // Store for next frame
        _lastBirths = hudData.births;
        _lastDeaths = currentTotalDeaths;
        
        // Advance history index
        _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;
        
        ImGui::Separator();
        
        // Population dynamics section
        ImGui::Text("Population Dynamics (This Frame)");
        ImGui::Indent();
        
        // Color code based on net change
        if (netChange > 0) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Births: %d", birthsThisFrame);
        } else {
            ImGui::Text("Births: %d", birthsThisFrame);
        }
        
        if (netChange < 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Deaths: %d", deathsThisFrame);
        } else {
            ImGui::Text("Deaths: %d", deathsThisFrame);
        }
        
        // Net change with color
        ImVec4 changeColor;
        if (netChange > 0) {
            changeColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);  // Green for growth
            ImGui::TextColored(changeColor, "Net Change: +%d (Growing)", netChange);
        } else if (netChange < 0) {
            changeColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red for decline
            ImGui::TextColored(changeColor, "Net Change: %d (Declining)", netChange);
        } else {
            changeColor = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);  // Yellow for stable
            ImGui::TextColored(changeColor, "Net Change: 0 (Stable)");
        }
        
        ImGui::Unindent();
        ImGui::Separator();
        
        // Lifetime totals (collapsed by default)
        if (ImGui::CollapsingHeader("Lifetime Totals", ImGuiTreeNodeFlags_None)) {
            ImGui::Indent();
            ImGui::Text("Total Births: %d", hudData.births);
            ImGui::Text("Total Deaths: %d", currentTotalDeaths);
            ImGui::Spacing();
            ImGui::Text("Deaths by Cause:");
            ImGui::Indent();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Old Age: %d", hudData.deaths.oldAge);
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.3f, 1.0f), "Starved: %d", hudData.deaths.starved);
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "Dehydrated: %d", hudData.deaths.dehydrated);
            ImGui::TextColored(ImVec4(0.6f, 0.4f, 0.8f, 1.0f), "Discomfort: %d", hudData.deaths.discomfort);
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Predator: %d", hudData.deaths.predator);
            ImGui::Unindent();
            ImGui::Text("Food Eaten: %d", hudData.foodEaten);
            ImGui::Unindent();
        }
        
        ImGui::Separator();
        
        // Population over time chart
        // Calculate max for scaling
        float maxPop = 1.0f;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (_populationHistory[i] > maxPop) maxPop = _populationHistory[i];
        }
        maxPop = maxPop * 1.2f;  // Add headroom
        
        ImGui::Text("Population History");
        ImGui::PlotLines("##PopHistory", _populationHistory, HISTORY_SIZE, _historyIndex,
                        nullptr, 0.0f, maxPop, ImVec2(-1, 80));
        
        // Births vs Deaths chart
        ImGui::Text("Births vs Deaths (Per Frame)");
        
        // Find max for scaling
        float maxBirthsDeath = 1.0f;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (_birthsHistory[i] > maxBirthsDeath) maxBirthsDeath = _birthsHistory[i];
            if (_deathsHistory[i] > maxBirthsDeath) maxBirthsDeath = _deathsHistory[i];
        }
        maxBirthsDeath = maxBirthsDeath * 1.2f;  // Add headroom
        if (maxBirthsDeath < 1.0f) maxBirthsDeath = 5.0f;  // Minimum scale
        
        // Births line (green)
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::PlotLines("##Births", _birthsHistory, HISTORY_SIZE, _historyIndex,
                        "Births", 0.0f, maxBirthsDeath, ImVec2(-1, 60));
        ImGui::PopStyleColor();
        
        // Deaths line (red)
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PlotLines("##Deaths", _deathsHistory, HISTORY_SIZE, _historyIndex,
                        "Deaths", 0.0f, maxBirthsDeath, ImVec2(-1, 60));
        ImGui::PopStyleColor();
        
        // Legend
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "■"); ImGui::SameLine();
        ImGui::Text("Births"); ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "■"); ImGui::SameLine();
        ImGui::Text("Deaths");
        
        ImGui::Separator();
        
        // Simulation controls display
        ImGui::Text("Simulation");
        ImGui::Indent();
        ImGui::Text("Tick Rate: %d", hudData.tickRate);
        ImGui::Text("Paused: %s", hudData.paused ? "Yes" : "No");
        ImGui::Unindent();
    }
    ImGui::End();
}

void ImGuiOverlay::renderWorldInfoWindow(const World* world, const std::vector<Creature>* creatures) {
    ImGui::SetNextWindowPos(ImVec2(1070, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("World Information", &_showWorldInfo, ImGuiWindowFlags_NoCollapse)) {
        if (world) {
            // World dimensions
            if (ImGui::CollapsingHeader("Dimensions", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Width:  %d tiles", world->getCols());
                ImGui::Text("Height: %d tiles", world->getRows());
                
                unsigned int totalTiles = world->getCols() * world->getRows();
                ImGui::Text("Total Tiles: %d", totalTiles);
            }
            
            ImGui::Spacing();
            
            // Generation parameters
            if (ImGui::CollapsingHeader("Generation", ImGuiTreeNodeFlags_DefaultOpen)) {
                MapGen mapGen = world->getMapGen();
                OctaveGen octaveGen = world->getOctaveGen();
                
                ImGui::Text("Seed: %.2f", mapGen.seed);
                ImGui::Text("Scale: %.4f", mapGen.scale);
                ImGui::Text("Frequency: %.2f", mapGen.freq);
                ImGui::Text("Exponent: %.2f", mapGen.exponent);
                ImGui::Text("Terraces: %d", mapGen.terraces);
                ImGui::Text("Island Mode: %s", mapGen.isIsland ? "Yes" : "No");
                
                ImGui::Spacing();
                ImGui::Text("Octaves: %d", octaveGen.quantity);
            }
            
        } else {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "No world data available");
        }
    }
    ImGui::End();
}

void ImGuiOverlay::renderPerformanceWindow() {
    ImGui::SetNextWindowPos(ImVec2(10, 490), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 180), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Performance", &_showPerformance, ImGuiWindowFlags_NoCollapse)) {
        ImGuiIO& io = ImGui::GetIO();
        
        // Track frame time
        _frameTimes[_frameTimeIndex] = 1000.0f / io.Framerate;
        _frameTimeIndex = (_frameTimeIndex + 1) % FRAME_TIME_HISTORY_SIZE;
        
        // FPS display with color coding
        float fps = io.Framerate;
        ImVec4 fpsColor;
        if (fps >= 55.0f) {
            fpsColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);  // Green - good
        } else if (fps >= 30.0f) {
            fpsColor = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);  // Yellow - okay
        } else {
            fpsColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);  // Red - poor
        }
        
        ImGui::TextColored(fpsColor, "FPS: %.1f", fps);
        ImGui::SameLine();
        ImGui::Text("(%.2f ms/frame)", 1000.0f / fps);
        
        // Frame time graph
        ImGui::Spacing();
        ImGui::Text("Frame Time History:");
        ImGui::PlotLines("##FrameTime", _frameTimes, FRAME_TIME_HISTORY_SIZE, 
                        _frameTimeIndex, nullptr, 0.0f, 33.3f, ImVec2(-1, 60));
        
        // Memory usage (approximate)
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("ImGui Vertices: %d", io.MetricsRenderVertices);
        ImGui::Text("ImGui Indices: %d", io.MetricsRenderIndices);
    }
    ImGui::End();
}

void ImGuiOverlay::renderCreatureListWindow(const std::vector<Creature>* creatures) {
    ImGui::SetNextWindowPos(ImVec2(1070, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 330), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Creatures", &_showCreatureList, ImGuiWindowFlags_NoCollapse)) {
        if (!creatures || creatures->empty()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "No creatures available");
            ImGui::End();
            return;
        }
        
        // Population Summary (moved from World Info)
        ImGui::Text("Living: %zu", creatures->size());
        
        // Count by diet
        int herbivores = 0, carnivores = 0, omnivores = 0, scavengers = 0;
        for (const auto& c : *creatures) {
            switch (c.getGenome().getDiet()) {
                case Diet::apple:
                case Diet::banana:
                    herbivores++;
                    break;
                case Diet::predator:
                    carnivores++;
                    break;
                case Diet::scavenger:
                    scavengers++;
                    break;
                default:
                    omnivores++;
                    break;
            }
        }
        ImGui::Text("H:%d C:%d O:%d S:%d", herbivores, carnivores, omnivores, scavengers);
        ImGui::Separator();
        
        // Filter controls
        ImGui::Text("Filter:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##filter", _creatureFilterText, sizeof(_creatureFilterText));
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            std::memset(_creatureFilterText, 0, sizeof(_creatureFilterText));
        }
        
        // Sorting options
        ImGui::Text("Sort by:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        const char* sortModes[] = { "Index", "Age", "Hunger", "Thirst", "Fatigue" };
        ImGui::Combo("##sort", &_creatureSortMode, sortModes, 5);
        
        ImGui::Separator();
        
        // Scrollable creature list
        if (ImGui::BeginChild("CreatureScroll", ImVec2(0, 0), true)) {
            // Create sorted index list
            std::vector<size_t> sortedIndices;
            for (size_t i = 0; i < creatures->size(); i++) {
                sortedIndices.push_back(i);
            }
            
            // Sort based on selected mode
            switch (_creatureSortMode) {
                case 1: // Age
                    std::sort(sortedIndices.begin(), sortedIndices.end(),
                        [creatures](size_t a, size_t b) {
                            return (*creatures)[a].getAge() > (*creatures)[b].getAge();
                        });
                    break;
                case 2: // Hunger (lower = more hungry)
                    std::sort(sortedIndices.begin(), sortedIndices.end(),
                        [creatures](size_t a, size_t b) {
                            return (*creatures)[a].getHunger() < (*creatures)[b].getHunger();
                        });
                    break;
                case 3: // Thirst
                    std::sort(sortedIndices.begin(), sortedIndices.end(),
                        [creatures](size_t a, size_t b) {
                            return (*creatures)[a].getThirst() < (*creatures)[b].getThirst();
                        });
                    break;
                case 4: // Fatigue
                    std::sort(sortedIndices.begin(), sortedIndices.end(),
                        [creatures](size_t a, size_t b) {
                            return (*creatures)[a].getFatigue() > (*creatures)[b].getFatigue();
                        });
                    break;
                default: // Index (no sort needed)
                    break;
            }
            
            for (size_t idx : sortedIndices) {
                const Creature& creature = (*creatures)[idx];
                int creatureId = creature.getId();
                
                // Filter check - allow filtering by creature ID or position
                if (std::strlen(_creatureFilterText) > 0) {
                    std::string idStr = std::to_string(creatureId);
                    std::string posStr = std::to_string(creature.getX()) + "," + std::to_string(creature.getY());
                    if (idStr.find(_creatureFilterText) == std::string::npos &&
                        posStr.find(_creatureFilterText) == std::string::npos) {
                        continue;
                    }
                }
                
                ImGui::PushID(creatureId);
                
                // Highlight if selected - compare with creature's unique ID
                bool isSelected = (_selectedCreatureId == creatureId);
                
                // Creature entry - use Selectable for proper click handling in scrollable regions
                char label[128];
                snprintf(label, sizeof(label), "#%d [%d,%d] Age:%d",
                        creatureId, creature.getX(), creature.getY(), creature.getAge());
                
                // Calculate width to leave space for profile text
                float availWidth = ImGui::GetContentRegionAvail().x;
                float profileTextWidth = 100.0f;  // Space for profile text like "[Sleeping]"
                float selectableWidth = availWidth - profileTextWidth;
                
                // Use Selectable with explicit width that leaves room for profile text
                ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_None, ImVec2(selectableWidth, 0));
                
                // Use IsItemClicked() which works reliably for click detection
                // Store the creature's unique ID, not the vector index
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    _selectedCreatureId = creatureId;
                    _showCreatureInspector = true;
                }
                
                // Show profile on same line as selectable
                ImGui::SameLine();
                
                // Color-coded profile
                const char* profileStr = getProfileName(static_cast<int>(creature.getProfile()));
                ImVec4 profileColor;
                
                switch (creature.getProfile()) {
                    case Profile::hungry:
                        profileColor = ImVec4(1.0f, 0.5f, 0.2f, 1.0f);
                        break;
                    case Profile::thirsty:
                        profileColor = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
                        break;
                    case Profile::breed:
                        profileColor = ImVec4(1.0f, 0.4f, 0.7f, 1.0f);
                        break;
                    case Profile::sleep:
                        profileColor = ImVec4(0.6f, 0.6f, 0.8f, 1.0f);
                        break;
                    case Profile::migrate:
                        profileColor = ImVec4(0.5f, 0.8f, 0.5f, 1.0f);
                        break;
                    default:
                        profileColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                }
                
                ImGui::TextColored(profileColor, "[%s]", profileStr);
                
                // Tooltip with quick details
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Creature #%d", creatureId);
                    ImGui::Separator();
                    ImGui::Text("Position: (%d, %d)", creature.getX(), creature.getY());
                    ImGui::Text("Age: %d / %d", creature.getAge(), creature.getLifespan());
                    ImGui::Text("Profile: %s", profileStr);
                    ImGui::Separator();
                    ImGui::Text("Hunger: %.1f%%", creature.getHunger() * 10.0f);
                    ImGui::Text("Thirst: %.1f%%", creature.getThirst() * 10.0f);
                    ImGui::Text("Fatigue: %.1f%%", creature.getFatigue() * 10.0f);
                    ImGui::Text("Mate Drive: %.1f%%", creature.getMate() * 10.0f);
                    ImGui::EndTooltip();
                }
                
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void ImGuiOverlay::renderCreatureInspectorWindow(const Creature* creature) {
    ImGui::SetNextWindowPos(ImVec2(1070, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 500), ImGuiCond_FirstUseEver);
    
    char title[64];
    snprintf(title, sizeof(title), "Creature Inspector - #%d###Inspector", _selectedCreatureId);
    
    if (ImGui::Begin(title, &_showCreatureInspector, ImGuiWindowFlags_NoCollapse)) {
        if (!creature) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Creature not found");
            ImGui::End();
            return;
        }
        
        // Basic Info Section
        if (ImGui::CollapsingHeader("Basic Information", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("ID: #%d", _selectedCreatureId);
            ImGui::Text("Position: (%d, %d)", creature->getX(), creature->getY());
            ImGui::Text("Age: %d / %d", creature->getAge(), creature->getLifespan());
            
            // Direction
            const char* dirStr = "Unknown";
            switch (creature->getDirection()) {
                case Direction::N: dirStr = "North"; break;
                case Direction::S: dirStr = "South"; break;
                case Direction::E: dirStr = "East"; break;
                case Direction::W: dirStr = "West"; break;
                case Direction::NE: dirStr = "Northeast"; break;
                case Direction::NW: dirStr = "Northwest"; break;
                case Direction::SE: dirStr = "Southeast"; break;
                case Direction::SW: dirStr = "Southwest"; break;
                case Direction::none: dirStr = "Stationary"; break;
            }
            ImGui::Text("Direction: %s", dirStr);
            
            // Profile with color
            const char* profileStr = getProfileName(static_cast<int>(creature->getProfile()));
            ImGui::Text("Behavior: %s", profileStr);
            
            // Show diet type - use emergent if new genetics enabled
            if (creature->usesNewGenetics() && creature->getPhenotype()) {
                const auto& phenotype = *creature->getPhenotype();
                EcoSim::Genetics::DietType diet = phenotype.calculateDietType();
                const char* dietStr = getEmergentDietName(diet);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Diet: %s (Emergent)", dietStr);
            } else {
                const Genome& genome = creature->getGenome();
                ImGui::Text("Diet: %s (Legacy)", getDietName(static_cast<int>(genome.getDiet())));
            }
        }
        
        // Needs Section
        if (ImGui::CollapsingHeader("Needs & Status", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Progress bars for needs (values seem to be 0-10 scale based on code)
            float hunger = creature->getHunger() / 10.0f;
            float thirst = creature->getThirst() / 10.0f;
            float fatigue = creature->getFatigue() / 10.0f;
            float mate = creature->getMate() / 10.0f;
            
            // Clamp values
            hunger = std::max(0.0f, std::min(1.0f, hunger));
            thirst = std::max(0.0f, std::min(1.0f, thirst));
            fatigue = std::max(0.0f, std::min(1.0f, fatigue));
            mate = std::max(0.0f, std::min(1.0f, mate));
            
            ImGui::Text("Hunger:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
            ImGui::ProgressBar(hunger, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Thirst:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
            ImGui::ProgressBar(thirst, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Fatigue:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.6f, 0.8f, 1.0f));
            ImGui::ProgressBar(fatigue, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Mate Drive:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.4f, 0.7f, 1.0f));
            ImGui::ProgressBar(mate, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
        }
        
        // New Genetics System - Digestion & Metabolism Section
        if (creature->usesNewGenetics() && creature->getPhenotype()) {
            const auto& phenotype = *creature->getPhenotype();
            
            if (ImGui::CollapsingHeader("Digestion & Metabolism", ImGuiTreeNodeFlags_DefaultOpen)) {
                using UG = EcoSim::Genetics::UniversalGenes;
                
                // Digestion genes
                float plantDigestion = phenotype.getTrait(UG::PLANT_DIGESTION_EFFICIENCY);
                float meatDigestion = phenotype.getTrait(UG::MEAT_DIGESTION_EFFICIENCY);
                float celluloseBreakdown = phenotype.getTrait(UG::CELLULOSE_BREAKDOWN);
                float toxinTolerance = phenotype.getTrait(UG::TOXIN_TOLERANCE);
                float toxinMetabolism = phenotype.getTrait(UG::TOXIN_METABOLISM);
                
                ImGui::Text("Plant Digestion:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::ProgressBar(plantDigestion, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Meat Digestion:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
                ImGui::ProgressBar(meatDigestion, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Cellulose:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.7f, 0.3f, 1.0f));
                ImGui::ProgressBar(celluloseBreakdown, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Toxin Tolerance:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f, 0.3f, 0.7f, 1.0f));
                ImGui::ProgressBar(toxinTolerance, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Toxin Metabolism:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.4f, 0.6f, 1.0f));
                ImGui::ProgressBar(toxinMetabolism, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
            }
            
            // Senses Section
            if (ImGui::CollapsingHeader("Senses")) {
                using UG = EcoSim::Genetics::UniversalGenes;
                
                float sightRange = phenotype.getTrait(UG::SIGHT_RANGE);
                float scentDetection = phenotype.getTrait(UG::SCENT_DETECTION);
                float colorVision = phenotype.getTrait(UG::COLOR_VISION);
                
                ImGui::Text("Sight Range: %.1f", sightRange);
                ImGui::Text("Scent Detection:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(scentDetection, ImVec2(-1, 0), "");
                
                ImGui::Text("Color Vision:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
                ImGui::ProgressBar(colorVision, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
            }
            
            // Physical Attributes Section
            if (ImGui::CollapsingHeader("Physical Attributes")) {
                using UG = EcoSim::Genetics::UniversalGenes;
                
                float maxSize = phenotype.getTrait(UG::MAX_SIZE);
                float locomotion = phenotype.getTrait(UG::LOCOMOTION);
                float hideThickness = phenotype.getTrait(UG::HIDE_THICKNESS);
                float furDensity = phenotype.getTrait(UG::FUR_DENSITY);
                float toothSharpness = phenotype.getTrait(UG::TOOTH_SHARPNESS);
                float toothGrinding = phenotype.getTrait(UG::TOOTH_GRINDING);
                float gutLength = phenotype.getTrait(UG::GUT_LENGTH);
                float jawStrength = phenotype.getTrait(UG::JAW_STRENGTH);
                
                ImGui::Text("Max Size: %.1f", maxSize);
                ImGui::Text("Locomotion: %.2f", locomotion);
                
                ImGui::Separator();
                ImGui::Text("Hide Thickness:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(hideThickness, ImVec2(-1, 0), "");
                
                ImGui::Text("Fur Density:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(furDensity, ImVec2(-1, 0), "");
                
                ImGui::Separator();
                ImGui::Text("Tooth Sharpness:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.5f, 0.5f, 1.0f));
                ImGui::ProgressBar(toothSharpness, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Tooth Grinding:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
                ImGui::ProgressBar(toothGrinding, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                ImGui::Text("Gut Length: %.2f", gutLength);
                ImGui::Text("Jaw Strength:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(jawStrength, ImVec2(-1, 0), "");
            }
            
            // Behavior Section
            if (ImGui::CollapsingHeader("Behavior")) {
                using UG = EcoSim::Genetics::UniversalGenes;
                
                float cachingInstinct = phenotype.getTrait(UG::CACHING_INSTINCT);
                float groomingFrequency = phenotype.getTrait(UG::GROOMING_FREQUENCY);
                float spatialMemory = phenotype.getTrait(UG::SPATIAL_MEMORY);
                float sweetnessPreference = phenotype.getTrait(UG::SWEETNESS_PREFERENCE);
                
                ImGui::Text("Caching Instinct:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(cachingInstinct, ImVec2(-1, 0), "");
                
                ImGui::Text("Grooming:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(groomingFrequency, ImVec2(-1, 0), "");
                
                ImGui::Text("Spatial Memory:");
                ImGui::SameLine(130);
                ImGui::ProgressBar(spatialMemory, ImVec2(-1, 0), "");
                
                ImGui::Text("Sweet Preference:");
                ImGui::SameLine(130);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.7f, 0.3f, 1.0f));
                ImGui::ProgressBar(sweetnessPreference, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
            }
            
            // Energy Budget Section
            if (ImGui::CollapsingHeader("Energy Budget", ImGuiTreeNodeFlags_DefaultOpen)) {
                float maintenanceCost = phenotype.getTotalMaintenanceCost();
                float specialistBonus = phenotype.getSpecialistBonus();
                float metabolicOverhead = phenotype.getMetabolicOverhead();
                
                ImGui::Text("Maintenance Cost: %.3f/tick", maintenanceCost);
                
                // Specialist bonus color-coded (green for high specialization)
                float bonusPercent = (specialistBonus - 1.0f) * 100.0f;
                if (bonusPercent > 15.0f) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f),
                        "Specialist Bonus: +%.0f%%", bonusPercent);
                } else if (bonusPercent > 5.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f),
                        "Specialist Bonus: +%.0f%%", bonusPercent);
                } else {
                    ImGui::Text("Specialist Bonus: +%.0f%%", bonusPercent);
                }
                
                // Metabolic overhead color-coded (red for high cost)
                float overheadPercent = (metabolicOverhead - 1.0f) * 100.0f;
                if (overheadPercent > 20.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                        "Metabolic Overhead: +%.0f%%", overheadPercent);
                } else if (overheadPercent > 10.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f),
                        "Metabolic Overhead: +%.0f%%", overheadPercent);
                } else {
                    ImGui::Text("Metabolic Overhead: +%.0f%%", overheadPercent);
                }
            }
        } else {
            // Legacy genome system display
            if (ImGui::CollapsingHeader("Genome (Legacy)", ImGuiTreeNodeFlags_DefaultOpen)) {
                const Genome& genome = creature->getGenome();
                
                ImGui::Text("Lifespan: %d", genome.getLifespan());
                ImGui::Text("Sight Range: %d", genome.getSight());
                ImGui::Text("Diet: %s", getDietName(static_cast<int>(genome.getDiet())));
                ImGui::Text("Flocking: %s", genome.ifFlocks() ? "Yes" : "No");
                
                ImGui::Separator();
                ImGui::Text("Thresholds:");
                ImGui::BulletText("Hunger: %.2f", genome.getTHunger());
                ImGui::BulletText("Thirst: %.2f", genome.getTThirst());
                ImGui::BulletText("Fatigue: %.2f", genome.getTFatigue());
                ImGui::BulletText("Mate: %.2f", genome.getTMate());
                
                ImGui::Separator();
                ImGui::Text("Comfort:");
                ImGui::BulletText("Increase: %.3f", genome.getComfInc());
                ImGui::BulletText("Decrease: %.3f", genome.getComfDec());
                
                if (genome.getDiet() == Diet::predator || genome.getDiet() == Diet::scavenger) {
                    ImGui::Separator();
                    ImGui::Text("Combat:");
                    ImGui::BulletText("Flee Distance: %d", genome.getFlee());
                    ImGui::BulletText("Pursue Distance: %d", genome.getPursue());
                }
            }
            
            // Speed and metabolism
            if (ImGui::CollapsingHeader("Physical Attributes")) {
                ImGui::Text("Speed: %d", creature->getSpeed());
                ImGui::Text("Metabolism: %.4f", creature->getMetabolism());
            }
        }
        
        // Action buttons
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Deselect", ImVec2(-1, 0))) {
            _selectedCreatureId = -1;
            _showCreatureInspector = false;
        }
    }
    ImGui::End();
}

void ImGuiOverlay::renderControlsWindow() {
    ImGui::SetNextWindowPos(ImVec2(660, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 280), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Controls", &_showControls, ImGuiWindowFlags_NoCollapse)) {
        // Simulation status
        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Pause state indicator
            if (_lastHudData.paused) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Status: PAUSED");
            } else {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Status: RUNNING");
            }
            
            ImGui::Text("Tick Rate: %dx", _lastHudData.tickRate);
            
            ImGui::Separator();
            ImGui::TextWrapped("Press SPACE to toggle pause");
        }
        
        // Time display
        if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Time: %s", _lastHudData.timeString.c_str());
            ImGui::Text("Date: %s", _lastHudData.dateString.c_str());
        }
        
        // Quick stats
        if (ImGui::CollapsingHeader("Quick Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Population: %d", _lastHudData.population);
            ImGui::Text("Births: %d", _lastHudData.births);
            ImGui::Text("Food Eaten: %d", _lastHudData.foodEaten);
            
            unsigned int totalDeaths = _lastHudData.deaths.oldAge +
                                       _lastHudData.deaths.starved +
                                       _lastHudData.deaths.dehydrated +
                                       _lastHudData.deaths.discomfort +
                                       _lastHudData.deaths.predator;
            ImGui::Text("Total Deaths: %d", totalDeaths);
        }
        
        // Keyboard shortcuts help
        if (ImGui::CollapsingHeader("Keyboard Shortcuts")) {
            ImGui::BulletText("WASD/Arrows: Move camera");
            ImGui::BulletText("SPACE: Pause/Resume");
            ImGui::BulletText("H: Toggle HUD");
            ImGui::BulletText("A: Add 100 creatures");
            ImGui::BulletText("S: Save state");
            ImGui::BulletText("ESC: Quit");
        }
    }
    ImGui::End();
}

const char* ImGuiOverlay::getProfileName(int profile) const {
    switch (static_cast<Profile>(profile)) {
        case Profile::hungry: return "Hungry";
        case Profile::thirsty: return "Thirsty";
        case Profile::breed: return "Breeding";
        case Profile::sleep: return "Sleeping";
        case Profile::migrate: return "Migrating";
        default: return "Unknown";
    }
}

// DRY refactoring: Uses centralized DietInfo lookup table instead of switch statement
const char* ImGuiOverlay::getDietName(int diet) const {
    return getDietInfo(static_cast<Diet>(diet)).displayName;
}

const char* ImGuiOverlay::getEmergentDietName(EcoSim::Genetics::DietType type) const {
    switch (type) {
        case EcoSim::Genetics::DietType::HERBIVORE: return "Herbivore";
        case EcoSim::Genetics::DietType::FRUGIVORE: return "Frugivore";
        case EcoSim::Genetics::DietType::OMNIVORE: return "Omnivore";
        case EcoSim::Genetics::DietType::CARNIVORE: return "Carnivore";
        default: return "Unknown";
    }
}

const char* ImGuiOverlay::getDispersalStrategyName(int strategy) const {
    switch (static_cast<EcoSim::Genetics::DispersalStrategy>(strategy)) {
        case EcoSim::Genetics::DispersalStrategy::GRAVITY: return "Gravity";
        case EcoSim::Genetics::DispersalStrategy::WIND: return "Wind";
        case EcoSim::Genetics::DispersalStrategy::ANIMAL_FRUIT: return "Animal (Fruit)";
        case EcoSim::Genetics::DispersalStrategy::ANIMAL_BURR: return "Animal (Burr)";
        case EcoSim::Genetics::DispersalStrategy::EXPLOSIVE: return "Explosive";
        case EcoSim::Genetics::DispersalStrategy::VEGETATIVE: return "Vegetative";
        default: return "Unknown";
    }
}

void ImGuiOverlay::renderPlantInspectorWindow(const EcoSim::Genetics::Plant* plant) {
    ImGui::SetNextWindowPos(ImVec2(800, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 450), ImGuiCond_FirstUseEver);
    
    char title[64];
    snprintf(title, sizeof(title), "Plant Inspector - #%d###PlantInspector", _selectedPlantId);
    
    if (ImGui::Begin(title, &_showPlantInspector, ImGuiWindowFlags_NoCollapse)) {
        if (!plant) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Plant not found");
            ImGui::End();
            return;
        }
        
        const auto& phenotype = plant->getPhenotype();
        using UG = EcoSim::Genetics::UniversalGenes;
        
        // Basic Info Section
        if (ImGui::CollapsingHeader("Basic Information", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("ID: #%d", _selectedPlantId);
            ImGui::Text("Position: (%d, %d)", plant->getX(), plant->getY());
            ImGui::Text("Age: %u / %u", plant->getAge(), plant->getMaxLifespan());
            ImGui::Text("Alive: %s", plant->isAlive() ? "Yes" : "No");
        }
        
        // Growth Section
        if (ImGui::CollapsingHeader("Growth", ImGuiTreeNodeFlags_DefaultOpen)) {
            float currentSize = plant->getCurrentSize();
            float maxSize = plant->getMaxSize();
            float growthProgress = (maxSize > 0) ? (currentSize / maxSize) : 0.0f;
            
            ImGui::Text("Current Size: %.2f", currentSize);
            ImGui::Text("Max Size: %.2f", maxSize);
            
            ImGui::Text("Growth:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::ProgressBar(growthProgress, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Growth Rate: %.2f", plant->getGrowthRate());
            ImGui::Text("Can Spread Seeds: %s", plant->canSpreadSeeds() ? "Yes" : "No");
        }
        
        // Defense Section
        if (ImGui::CollapsingHeader("Defenses")) {
            float toxicity = plant->getToxicity();
            float thornDamage = plant->getThornDamage();
            float regrowth = plant->getRegrowthRate();
            
            ImGui::Text("Toxicity:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f, 0.3f, 0.7f, 1.0f));
            ImGui::ProgressBar(toxicity, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Thorns:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.5f, 0.3f, 1.0f));
            ImGui::ProgressBar(thornDamage, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Regrowth:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
            ImGui::ProgressBar(regrowth, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
        }
        
        // Reproduction Section
        if (ImGui::CollapsingHeader("Reproduction")) {
            float fruitRate = plant->getFruitProductionRate();
            float fruitAppeal = plant->getFruitAppeal();
            float spreadDist = plant->getSpreadDistance();
            int seedCount = plant->getSeedCount();
            
            ImGui::Text("Fruit Production:");
            ImGui::SameLine(130);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
            ImGui::ProgressBar(fruitRate, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Fruit Appeal:");
            ImGui::SameLine(130);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
            ImGui::ProgressBar(fruitAppeal, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Spread Distance: %.1f", spreadDist);
            ImGui::Text("Seeds per Cycle: %d", seedCount);
            ImGui::Text("Can Produce Fruit: %s", plant->canProduceFruit() ? "Yes" : "No");
            
            // Dispersal strategy
            auto strategy = plant->getPrimaryDispersalStrategy();
            const char* strategyName = getDispersalStrategyName(static_cast<int>(strategy));
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Dispersal: %s", strategyName);
        }
        
        // Seed Properties Section
        if (ImGui::CollapsingHeader("Seed Properties")) {
            float seedMass = plant->getSeedMass();
            float seedAero = plant->getSeedAerodynamics();
            float seedHook = plant->getSeedHookStrength();
            float seedCoat = plant->getSeedCoatDurability();
            
            ImGui::Text("Seed Mass: %.3f mg", seedMass);
            
            ImGui::Text("Aerodynamics:");
            ImGui::SameLine(130);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
            ImGui::ProgressBar(seedAero, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Hook Strength:");
            ImGui::SameLine(130);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.6f, 0.4f, 1.0f));
            ImGui::ProgressBar(seedHook, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Coat Durability:");
            ImGui::SameLine(130);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.5f, 0.8f, 1.0f));
            ImGui::ProgressBar(seedCoat, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
        }
        
        // Environment Requirements Section
        if (ImGui::CollapsingHeader("Environment")) {
            float lightNeed = plant->getLightNeed();
            float waterNeed = plant->getWaterNeed();
            float hardiness = plant->getHardiness();
            
            ImGui::Text("Light Need:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
            ImGui::ProgressBar(lightNeed, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Water Need:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
            ImGui::ProgressBar(waterNeed, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("Hardiness:");
            ImGui::SameLine(100);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.8f, 0.6f, 1.0f));
            ImGui::ProgressBar(hardiness, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
        }
        
        // Energy Budget Section
        if (ImGui::CollapsingHeader("Energy Budget")) {
            const auto& energyState = plant->getEnergyState();
            
            float energyRatio = energyState.getEnergyRatio();
            ImGui::Text("Energy:");
            ImGui::SameLine(100);
            
            // Color code based on energy level
            ImVec4 energyColor;
            if (energyRatio > 0.5f) {
                energyColor = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
            } else if (energyRatio > 0.2f) {
                energyColor = ImVec4(0.9f, 0.9f, 0.3f, 1.0f);
            } else {
                energyColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
            }
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, energyColor);
            ImGui::ProgressBar(energyRatio, ImVec2(-1, 0), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("%.1f / %.1f", energyState.currentEnergy, energyState.maxEnergy);
            ImGui::Text("Maintenance: %.3f/tick", energyState.maintenanceCost);
        }
        
        // Action buttons
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Deselect", ImVec2(-1, 0))) {
            _selectedPlantId = -1;
            _showPlantInspector = false;
        }
    }
    ImGui::End();
}

//==============================================================================
// Style Setup
//==============================================================================

void ImGuiOverlay::setupStyle() {
    // Use dark style as base
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Window styling
    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowPadding = ImVec2(10, 10);
    
    // Frame styling
    style.FrameRounding = 4.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(6, 4);
    
    // Widget styling
    style.GrabRounding = 4.0f;
    style.GrabMinSize = 10.0f;
    style.ScrollbarRounding = 4.0f;
    style.ScrollbarSize = 12.0f;
    
    // Tab styling
    style.TabRounding = 4.0f;
    
    // Popup styling
    style.PopupRounding = 4.0f;
    style.PopupBorderSize = 1.0f;
    
    // Colors - EcoSim themed
    ImVec4* colors = style.Colors;
    
    // Background colors - deep blue-gray
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.10f, 0.14f, 0.94f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.12f, 0.16f, 0.96f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    
    // Header colors - forest green accent
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.40f, 0.30f, 0.70f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.50f, 0.35f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.55f, 0.40f, 1.00f);
    
    // Title bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.14f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.25f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.10f, 0.08f, 0.75f);
    
    // Tab colors
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.25f, 0.20f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.35f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.40f, 0.30f, 1.00f);
    
    // Button colors
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.35f, 0.28f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.48f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.55f, 0.45f, 1.00f);
    
    // Frame (input fields, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.16f, 0.14f, 0.70f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.24f, 0.20f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.30f, 0.26f, 1.00f);
    
    // Slider grab
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.55f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.65f, 0.50f, 1.00f);
    
    // Check mark
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.80f, 0.55f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.35f, 0.30f, 0.50f);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.40f, 0.30f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.55f, 0.40f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.65f, 0.50f, 0.95f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.10f, 0.10f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.35f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.45f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.55f, 0.45f, 1.00f);
    
    // Plot lines (for graphs)
    colors[ImGuiCol_PlotLines] = ImVec4(0.50f, 0.80f, 0.60f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.70f, 1.00f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.70f, 0.50f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.55f, 0.85f, 0.65f, 1.00f);
    
    // Table colors
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.22f, 0.18f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.35f, 0.30f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.28f, 0.24f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
    
    // Nav highlight
    colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.70f, 0.50f, 1.00f);
}