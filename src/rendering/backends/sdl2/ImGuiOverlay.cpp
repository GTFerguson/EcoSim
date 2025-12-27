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
#include "world/Corpse.hpp"
#include "objects/creature/creature.hpp"

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
// Helper Functions for Action Display
//==============================================================================

static const char* getActionName(Action action) {
    switch (action) {
        case Action::Idle: return "Idle";
        case Action::Wandering: return "Wandering";
        case Action::Searching: return "Searching";
        case Action::Navigating: return "Navigating";
        case Action::Eating: return "Eating";
        case Action::Grazing: return "Grazing";
        case Action::Hunting: return "Hunting";
        case Action::Chasing: return "Chasing";
        case Action::Attacking: return "Attacking";
        case Action::Fleeing: return "Fleeing";
        case Action::Drinking: return "Drinking";
        case Action::Courting: return "Courting";
        case Action::Mating: return "Mating";
        case Action::Resting: return "Resting";
        default: return "Unknown";
    }
}

static ImVec4 getActionColor(Action action) {
    switch (action) {
        case Action::Attacking: return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red
        case Action::Fleeing: return ImVec4(1.0f, 0.8f, 0.3f, 1.0f);    // Yellow
        case Action::Hunting:
        case Action::Chasing: return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);    // Orange
        case Action::Eating:
        case Action::Grazing: return ImVec4(0.3f, 1.0f, 0.3f, 1.0f);    // Green
        case Action::Mating:
        case Action::Courting: return ImVec4(1.0f, 0.5f, 0.8f, 1.0f);   // Pink
        case Action::Resting: return ImVec4(0.5f, 0.5f, 1.0f, 1.0f);    // Blue
        case Action::Drinking: return ImVec4(0.3f, 0.7f, 1.0f, 1.0f);   // Light Blue
        case Action::Searching: return ImVec4(0.9f, 0.9f, 0.5f, 1.0f);  // Light Yellow
        default: return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);                 // Gray
    }
}

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
    , _pendingCenterX(-1)
    , _pendingCenterY(-1)
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
    // Calculate right-side position dynamically based on window size
    ImGuiIO& io = ImGui::GetIO();
    const float rightMargin = 10.0f;
    const float windowWidth = 200.0f;
    float xPos = io.DisplaySize.x - windowWidth - rightMargin;
    
    ImGui::SetNextWindowPos(ImVec2(xPos, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, 200), ImGuiCond_FirstUseEver);
    
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
            
            // Corpse information (cast to non-const to access corpses)
            if (ImGui::CollapsingHeader("Corpses", ImGuiTreeNodeFlags_DefaultOpen)) {
                World* mutableWorld = const_cast<World*>(world);
                const auto& corpses = mutableWorld->getCorpses();
                size_t corpseCount = corpses.size();
                
                ImGui::Text("Active Corpses: %zu", corpseCount);
                ImGui::Text("Max Corpses: %zu", World::MAX_CORPSES);
                
                // Corpse usage bar
                float corpseUsage = static_cast<float>(corpseCount) / static_cast<float>(World::MAX_CORPSES);
                ImGui::Text("Usage:");
                ImGui::SameLine(80);
                ImVec4 usageColor;
                if (corpseUsage > 0.8f) {
                    usageColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);  // Red - near limit
                } else if (corpseUsage > 0.5f) {
                    usageColor = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);  // Yellow - moderate
                } else {
                    usageColor = ImVec4(0.6f, 0.4f, 0.2f, 1.0f);  // Brown - normal
                }
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, usageColor);
                ImGui::ProgressBar(corpseUsage, ImVec2(-1, 0), "");
                ImGui::PopStyleColor();
                
                // Optional: Show average decay if corpses exist
                if (corpseCount > 0) {
                    float totalDecay = 0.0f;
                    for (const auto& corpse : corpses) {
                        totalDecay += corpse->getDecayProgress();
                    }
                    float avgDecay = totalDecay / static_cast<float>(corpseCount);
                    ImGui::Text("Avg Decay: %.0f%%", avgDecay * 100.0f);
                }
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
    // Calculate right-side position dynamically based on window size
    ImGuiIO& io = ImGui::GetIO();
    const float rightMargin = 10.0f;
    const float windowWidth = 200.0f;
    float xPos = io.DisplaySize.x - windowWidth - rightMargin;
    
    ImGui::SetNextWindowPos(ImVec2(xPos, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, 330), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Creatures", &_showCreatureList, ImGuiWindowFlags_NoCollapse)) {
        if (!creatures || creatures->empty()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "No creatures available");
            ImGui::End();
            return;
        }
        
        // Population Summary (moved from World Info)
        ImGui::Text("Living: %zu", creatures->size());
        
        // Count by diet (using new emergent genetics system)
        // H=Herbivore, F=Frugivore, O=Omnivore, C=Carnivore, N=Necrovore
        int herbivores = 0, frugivores = 0, omnivores = 0, carnivores = 0, necrovores = 0;
        for (const auto& c : *creatures) {
            if (c.getPhenotype()) {
                // Use emergent diet calculation from new genetics system
                EcoSim::Genetics::DietType diet = c.getPhenotype()->calculateDietType();
                switch (diet) {
                    case EcoSim::Genetics::DietType::HERBIVORE:
                        herbivores++;
                        break;
                    case EcoSim::Genetics::DietType::FRUGIVORE:
                        frugivores++;
                        break;
                    case EcoSim::Genetics::DietType::CARNIVORE:
                        carnivores++;
                        break;
                    case EcoSim::Genetics::DietType::NECROVORE:
                        necrovores++;
                        break;
                    case EcoSim::Genetics::DietType::OMNIVORE:
                    default:
                        omnivores++;
                        break;
                }
            } else {
                // Fallback for any legacy creatures (should not exist after migration)
                omnivores++;
            }
        }
        ImGui::Text("H:%d F:%d O:%d C:%d N:%d", herbivores, frugivores, omnivores, carnivores, necrovores);
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
                
                // Double-click to center viewport on this creature
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    _pendingCenterX = creature.getX();
                    _pendingCenterY = creature.getY();
                }
                
                // Show profile on same line as selectable
                ImGui::SameLine();
                
                // Color-coded profile
                const char* profileStr = getProfileName(static_cast<int>(creature.getMotivation()));
                ImVec4 profileColor;
                
                switch (creature.getMotivation()) {
                    case Motivation::Hungry:
                        profileColor = ImVec4(1.0f, 0.5f, 0.2f, 1.0f);
                        break;
                    case Motivation::Thirsty:
                        profileColor = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
                        break;
                    case Motivation::Amorous:
                        profileColor = ImVec4(1.0f, 0.4f, 0.7f, 1.0f);
                        break;
                    case Motivation::Tired:
                        profileColor = ImVec4(0.6f, 0.6f, 0.8f, 1.0f);
                        break;
                    case Motivation::Content:
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

//==============================================================================
// Creature Inspector Window - New Design with 5 Tabs + Genetics Sub-Tabs
//==============================================================================

void ImGuiOverlay::renderCreatureInspectorWindow(const Creature* creature) {
    // Calculate right-side position dynamically based on window size
    ImGuiIO& io = ImGui::GetIO();
    const float rightMargin = 10.0f;
    const float windowWidth = 550.0f;
    float xPos = io.DisplaySize.x - windowWidth - rightMargin;
    
    ImGui::SetNextWindowPos(ImVec2(xPos, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Creature Inspector", &_showCreatureInspector, ImGuiWindowFlags_NoCollapse)) {
        if (!creature) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Creature not found");
            ImGui::End();
            return;
        }
        
        // ============================================
        // HEADER SECTION (Always visible above tabs)
        // ============================================
        
        // Common name from archetype label
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "%s", creature->getArchetypeLabel().c_str());
        
        // Scientific name
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f), "%s", creature->getScientificName().c_str());
        
        // ID and Age
        ImGui::Text("ID: #%d  Age: %d/%d", creature->getId(),
                    creature->getAge(), creature->getLifespan());
        ImGui::Separator();
        
        // ============================================
        // TABBED INTERFACE - 5 Main Tabs
        // ============================================
        if (ImGui::BeginTabBar("InspectorTabs")) {
            
            // ==========================================
            // TAB 1: OVERVIEW
            // ==========================================
            if (ImGui::BeginTabItem("Overview")) {
                ImGui::Spacing();
                
                // --- CURRENT BEHAVIOR ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "CURRENT BEHAVIOR");
                ImGui::Separator();
                
                // Motivation and Action
                const char* motivationStr = getProfileName(static_cast<int>(creature->getMotivation()));
                ImVec4 motivationColor;
                switch (creature->getMotivation()) {
                    case Motivation::Hungry: motivationColor = ImVec4(1.0f, 0.5f, 0.2f, 1.0f); break;
                    case Motivation::Thirsty: motivationColor = ImVec4(0.3f, 0.7f, 1.0f, 1.0f); break;
                    case Motivation::Amorous: motivationColor = ImVec4(1.0f, 0.4f, 0.7f, 1.0f); break;
                    case Motivation::Tired: motivationColor = ImVec4(0.6f, 0.6f, 0.8f, 1.0f); break;
                    case Motivation::Content: motivationColor = ImVec4(0.5f, 0.8f, 0.5f, 1.0f); break;
                    default: motivationColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                }
                ImGui::Text("Motivation:");
                ImGui::SameLine(100);
                ImGui::TextColored(motivationColor, "%s", motivationStr);
                
                // Action display using Action enum
                Action action = creature->getAction();
                ImGui::Text("Action:");
                ImGui::SameLine(100);
                ImGui::TextColored(getActionColor(action), "%s", getActionName(action));
                
                // Show target if in combat
                if (creature->isInCombat()) {
                    int targetId = creature->getTargetId();
                    if (targetId >= 0) {
                        ImGui::Text("Target:");
                        ImGui::SameLine(100);
                        ImGui::Text("#%d", targetId);
                    }
                }
                
                ImGui::Spacing();
                
                // --- DIET CLASSIFICATION (Emergent) ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "DIET CLASSIFICATION (Emergent)");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    EcoSim::Genetics::DietType diet = phenotype.calculateDietType();
                    ImGui::Text("Primary:");
                    ImGui::SameLine(100);
                    ImGui::Text("%s", getEmergentDietName(diet));
                    
                    float plantDigest = phenotype.getTrait(UG::PLANT_DIGESTION_EFFICIENCY);
                    float meatDigest = phenotype.getTrait(UG::MEAT_DIGESTION_EFFICIENCY);
                    
                    ImGui::Text("Plant Digest:");
                    ImGui::SameLine(100);
                    ImGui::Text("%.2f", plantDigest);
                    
                    ImGui::Text("Meat Digest:");
                    ImGui::SameLine(100);
                    ImGui::Text("%.2f", meatDigest);
                } else {
                    ImGui::Text("Diet:");
                    ImGui::SameLine(100);
                    ImGui::Text("%s", getEmergentDietName(creature->getDietType()));
                }
                
                ImGui::Spacing();
                
                // --- SIZE & POSITION ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "SIZE & POSITION");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    float maxSize = creature->getPhenotype()->getTrait(EcoSim::Genetics::UniversalGenes::MAX_SIZE);
                    ImGui::Text("Size:");
                    ImGui::SameLine(100);
                    ImGui::Text("%.1f (Max: %.1f)", maxSize * 0.8f, maxSize);
                } else {
                    ImGui::Text("Size:");
                    ImGui::SameLine(100);
                    ImGui::Text("Medium");
                }
                
                ImGui::Text("Position:");
                ImGui::SameLine(100);
                ImGui::Text("(%d, %d)", creature->tileX(), creature->tileY());
                
                ImGui::Text("World Pos:");
                ImGui::SameLine(100);
                ImGui::Text("(%.1f, %.1f)", creature->getWorldX(), creature->getWorldY());
                
                ImGui::Spacing();
                
                // --- KEY STATS SUMMARY ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "KEY STATS SUMMARY");
                ImGui::Separator();
                
                // Health bar
                float healthPercent = creature->getHealthPercent();
                char healthLabel[32];
                snprintf(healthLabel, sizeof(healthLabel), "%.0f/%.0f", creature->getHealth(), creature->getMaxHealth());
                ImGui::Text("Health:");
                ImGui::SameLine(80);
                ImVec4 healthColor = (healthPercent > 0.5f) ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) :
                                     (healthPercent > 0.25f) ? ImVec4(1.0f, 0.8f, 0.3f, 1.0f) :
                                     ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, healthColor);
                ImGui::ProgressBar(healthPercent, ImVec2(200, 0), healthLabel);
                ImGui::PopStyleColor();
                
                // Hunger (0-10 scale)
                float hunger = creature->getHunger();
                char hungerLabel[16];
                snprintf(hungerLabel, sizeof(hungerLabel), "%.1f/10", hunger);
                ImGui::Text("Hunger:");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
                ImGui::ProgressBar(hunger / 10.0f, ImVec2(200, 0), hungerLabel);
                ImGui::PopStyleColor();
                
                // Energy (inverse of fatigue)
                float energy = 10.0f - creature->getFatigue();
                char energyLabel[16];
                snprintf(energyLabel, sizeof(energyLabel), "%.1f/10", energy);
                ImGui::Text("Energy:");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
                ImGui::ProgressBar(energy / 10.0f, ImVec2(200, 0), energyLabel);
                ImGui::PopStyleColor();
                
                ImGui::Spacing();
                
                // --- NOTABLE TRAITS ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "NOTABLE TRAITS");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    int traitCount = 0;
                    float locomotion = phenotype.getTrait(UG::LOCOMOTION);
                    float sightRange = phenotype.getTrait(UG::SIGHT_RANGE);
                    float huntInstinct = phenotype.getTrait(UG::HUNT_INSTINCT);
                    float aggression = phenotype.getTrait(UG::COMBAT_AGGRESSION);
                    float teethSharp = phenotype.getTrait(UG::TEETH_SHARPNESS);
                    float clawLength = phenotype.getTrait(UG::CLAW_LENGTH);
                    float hardiness = phenotype.getTrait(UG::HARDINESS);
                    
                    if (locomotion > 0.75f) { ImGui::BulletText("Fast runner (Locomotion: %.2f)", locomotion); traitCount++; }
                    if (sightRange > 100.0f) { ImGui::BulletText("Excellent vision (Sight: %.0f)", sightRange); traitCount++; }
                    if (huntInstinct > 0.75f) { ImGui::BulletText("Strong hunter (Hunt: %.2f)", huntInstinct); traitCount++; }
                    if (aggression > 0.75f) { ImGui::BulletText("Highly aggressive (Aggression: %.2f)", aggression); traitCount++; }
                    if (teethSharp > 0.75f) { ImGui::BulletText("Sharp teeth (Sharpness: %.2f)", teethSharp); traitCount++; }
                    if (clawLength > 0.75f) { ImGui::BulletText("Long claws (Length: %.2f)", clawLength); traitCount++; }
                    if (hardiness > 0.75f) { ImGui::BulletText("Very hardy (Hardiness: %.2f)", hardiness); traitCount++; }
                    
                    if (traitCount == 0) {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No exceptional traits");
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome - no trait analysis");
                }
                
                ImGui::EndTabItem();
            }
            
            // ==========================================
            // TAB 2: STATE
            // ==========================================
            if (ImGui::BeginTabItem("State")) {
                ImGui::Spacing();
                
                // --- VITALS ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "VITALS");
                ImGui::Separator();
                
                float healthPercent = creature->getHealthPercent();
                float currentHealth = creature->getHealth();
                float maxHealth = creature->getMaxHealth();
                WoundState woundState = creature->getWoundState();
                
                ImVec4 healthBarColor;
                const char* stateText;
                switch (woundState) {
                    case WoundState::Healthy:
                        healthBarColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                        stateText = "Healthy";
                        break;
                    case WoundState::Injured:
                        healthBarColor = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);
                        stateText = "Injured";
                        break;
                    case WoundState::Wounded:
                        healthBarColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                        stateText = "Wounded";
                        break;
                    case WoundState::Critical:
                        healthBarColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                        stateText = "Critical";
                        break;
                    default:
                        healthBarColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                        stateText = "Dead";
                        break;
                }
                
                // Health bar styled like needs bars
                char healthLabel[32];
                snprintf(healthLabel, sizeof(healthLabel), "%.0f / %.0f", currentHealth, maxHealth);
                ImGui::Text("Health");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, healthBarColor);
                ImGui::ProgressBar(healthPercent, ImVec2(200, 0), healthLabel);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextColored(healthBarColor, "(%s)", stateText);
                
                float woundSeverity = creature->getWoundSeverity();
                if (woundSeverity > 0.0f) {
                    ImGui::Text("Wound Severity:");
                    ImGui::SameLine(120);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    ImGui::ProgressBar(woundSeverity, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                }
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    ImGui::Text("Regeneration Rate: %.2f", phenotype.getTrait(UG::REGENERATION_RATE));
                    ImGui::Text("Wound Tolerance: %.2f", phenotype.getTrait(UG::WOUND_TOLERANCE));
                    ImGui::Text("Bleeding Resistance: %.2f", phenotype.getTrait(UG::BLEEDING_RESISTANCE));
                } else {
                    ImGui::Text("Healing: %.3f/tick", creature->getHealingRate());
                }
                
                ImGui::Spacing();
                
                // --- NEEDS ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "NEEDS");
                ImGui::Separator();
                
                // All needs on 0-10 scale with progress bars
                float hunger = creature->getHunger();
                float thirst = creature->getThirst();
                float fatigue = creature->getFatigue();
                float mate = creature->getMate();
                
                char needLabel[32];
                
                // Hunger
                snprintf(needLabel, sizeof(needLabel), "%.1f / 10", hunger);
                ImGui::Text("Hunger");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
                ImGui::ProgressBar(hunger / 10.0f, ImVec2(200, 0), needLabel);
                ImGui::PopStyleColor();
                
                // Thirst
                snprintf(needLabel, sizeof(needLabel), "%.1f / 10", thirst);
                ImGui::Text("Thirst");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
                ImGui::ProgressBar(thirst / 10.0f, ImVec2(200, 0), needLabel);
                ImGui::PopStyleColor();
                
                // Fatigue
                snprintf(needLabel, sizeof(needLabel), "%.1f / 10", fatigue);
                ImGui::Text("Fatigue");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.6f, 0.8f, 1.0f));
                ImGui::ProgressBar(fatigue / 10.0f, ImVec2(200, 0), needLabel);
                ImGui::PopStyleColor();
                
                // Mate Drive
                snprintf(needLabel, sizeof(needLabel), "%.1f / 10", mate);
                ImGui::Text("Mate");
                ImGui::SameLine(80);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.4f, 0.7f, 1.0f));
                ImGui::ProgressBar(mate / 10.0f, ImVec2(200, 0), needLabel);
                ImGui::PopStyleColor();
                
                ImGui::Spacing();
                
                // --- ENERGY BUDGET ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "ENERGY BUDGET");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    float maintenanceCost = phenotype.getTotalMaintenanceCost();
                    float metabolism = phenotype.getTrait(EcoSim::Genetics::UniversalGenes::METABOLISM_RATE);
                    
                    ImGui::Text("Metabolism: %.3f", metabolism);
                    ImGui::Text("Maintenance Cost: %.3f/tick", maintenanceCost);
                } else {
                    ImGui::Text("Metabolism: %.4f", creature->getMetabolism());
                }
                
                ImGui::Text("Speed: %d", creature->getSpeed());
                
                ImGui::Spacing();
                
                // --- STRESS & CONDITION ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "STRESS & CONDITION");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    float hardiness = creature->getPhenotype()->getTrait(EcoSim::Genetics::UniversalGenes::HARDINESS);
                    ImGui::Text("Hardiness: %.2f", hardiness);
                }
                
                ImGui::Text("World Position: (%.1f, %.1f)", creature->getWorldX(), creature->getWorldY());
                
                ImGui::EndTabItem();
            }
            
            // ==========================================
            // TAB 3: COMBAT
            // ==========================================
            if (ImGui::BeginTabItem("Combat")) {
                ImGui::Spacing();
                
                // --- COMBAT STATUS ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "COMBAT STATUS");
                ImGui::Separator();
                
                if (creature->isInCombat()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: IN COMBAT");
                    int targetId = creature->getTargetId();
                    if (targetId >= 0) {
                        ImGui::Text("Target: #%d", targetId);
                    }
                } else if (creature->isFleeing()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Status: FLEEING");
                } else {
                    ImGui::Text("Status: Peaceful");
                }
                
                int cooldown = creature->getCombatCooldown();
                if (cooldown > 0) {
                    ImGui::Text("Cooldown: %d ticks", cooldown);
                } else {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Ready to Attack");
                }
                
                ImGui::Spacing();
                
                // --- COMBAT BEHAVIOR ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "COMBAT BEHAVIOR");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    float aggression = phenotype.getTrait(UG::COMBAT_AGGRESSION);
                    float retreat = phenotype.getTrait(UG::RETREAT_THRESHOLD);
                    float territorial = phenotype.getTrait(UG::TERRITORIAL_AGGRESSION);
                    float packCoord = phenotype.getTrait(UG::PACK_COORDINATION);
                    
                    ImGui::Text("Aggression:");
                    ImGui::SameLine(120);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                    ImGui::ProgressBar(aggression, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", aggression);
                    
                    ImGui::Text("Retreat Thresh:");
                    ImGui::SameLine(120);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
                    ImGui::ProgressBar(retreat, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", retreat);
                    
                    ImGui::Text("Territorial:");
                    ImGui::SameLine(120);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.6f, 0.3f, 1.0f));
                    ImGui::ProgressBar(territorial, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", territorial);
                    
                    ImGui::Text("Pack Coord:");
                    ImGui::SameLine(120);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.9f, 0.6f, 1.0f));
                    ImGui::ProgressBar(packCoord, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", packCoord);
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome");
                }
                
                ImGui::Spacing();
                
                // --- WEAPONS BREAKDOWN ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "WEAPONS BREAKDOWN");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    // Teeth
                    if (ImGui::TreeNode("Teeth")) {
                        ImGui::Text("%-20s: %.2f", "Sharpness", phenotype.getTrait(UG::TEETH_SHARPNESS));
                        ImGui::Text("%-20s: %.2f", "Serration", phenotype.getTrait(UG::TEETH_SERRATION));
                        ImGui::Text("%-20s: %.2f", "Size", phenotype.getTrait(UG::TEETH_SIZE));
                        ImGui::TreePop();
                    }
                    
                    // Claws
                    if (ImGui::TreeNode("Claws")) {
                        ImGui::Text("%-20s: %.2f", "Length", phenotype.getTrait(UG::CLAW_LENGTH));
                        ImGui::Text("%-20s: %.2f", "Sharpness", phenotype.getTrait(UG::CLAW_SHARPNESS));
                        ImGui::Text("%-20s: %.2f", "Curvature", phenotype.getTrait(UG::CLAW_CURVATURE));
                        ImGui::TreePop();
                    }
                    
                    // Horns
                    if (ImGui::TreeNode("Horns")) {
                        ImGui::Text("%-20s: %.2f", "Length", phenotype.getTrait(UG::HORN_LENGTH));
                        ImGui::Text("%-20s: %.2f", "Pointiness", phenotype.getTrait(UG::HORN_POINTINESS));
                        ImGui::Text("%-20s: %.2f", "Spread", phenotype.getTrait(UG::HORN_SPREAD));
                        ImGui::TreePop();
                    }
                    
                    // Tail Weapons
                    if (ImGui::TreeNode("Tail")) {
                        ImGui::Text("%-20s: %.2f", "Length", phenotype.getTrait(UG::TAIL_LENGTH));
                        ImGui::Text("%-20s: %.2f", "Mass", phenotype.getTrait(UG::TAIL_MASS));
                        ImGui::Text("%-20s: %.2f", "Spines", phenotype.getTrait(UG::TAIL_SPINES));
                        ImGui::Text("%-20s: %.2f", "Body Spines", phenotype.getTrait(UG::BODY_SPINES));
                        ImGui::TreePop();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome");
                }
                
                ImGui::Spacing();
                
                // --- DEFENSE BREAKDOWN ---
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "DEFENSE BREAKDOWN");
                ImGui::Separator();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    float hideThick = phenotype.getTrait(UG::HIDE_THICKNESS);
                    float scaleCover = phenotype.getTrait(UG::SCALE_COVERAGE);
                    float fatLayer = phenotype.getTrait(UG::FAT_LAYER_THICKNESS);
                    
                    ImGui::Text("Hide:");
                    ImGui::SameLine(100);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.4f, 0.2f, 1.0f));
                    ImGui::ProgressBar(hideThick, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", hideThick);
                    
                    ImGui::Text("Scales:");
                    ImGui::SameLine(100);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.5f, 0.3f, 1.0f));
                    ImGui::ProgressBar(scaleCover, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", scaleCover);
                    
                    ImGui::Text("Fat Layer:");
                    ImGui::SameLine(100);
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.8f, 0.5f, 1.0f));
                    ImGui::ProgressBar(fatLayer, ImVec2(200, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    ImGui::Text("%.2f", fatLayer);
                    
                    // Calculate approximate damage reduction
                    float damageReduction = (hideThick * 0.15f + scaleCover * 0.25f + fatLayer * 0.1f) * 100.0f;
                    ImGui::Spacing();
                    ImGui::Text("Est. Damage Reduction: ~%.0f%%", damageReduction);
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome");
                }
                
                ImGui::EndTabItem();
            }
            
            // ==========================================
            // TAB 4: PHYSICAL
            // ==========================================
            if (ImGui::BeginTabItem("Physical")) {
                ImGui::Spacing();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    // --- MORPHOLOGY ---
                    ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "MORPHOLOGY");
                    ImGui::Separator();
                    
                    ImGui::Columns(2, "morphology_cols", false);
                    ImGui::Text("%-20s: %.2f", "Hide Thickness", phenotype.getTrait(UG::HIDE_THICKNESS));
                    ImGui::Text("%-20s: %.2f", "Fur Density", phenotype.getTrait(UG::FUR_DENSITY));
                    ImGui::Text("%-20s: %.2f", "Scale Coverage", phenotype.getTrait(UG::SCALE_COVERAGE));
                    ImGui::Text("%-20s: %.2f", "Fat Layer", phenotype.getTrait(UG::FAT_LAYER_THICKNESS));
                    ImGui::NextColumn();
                    ImGui::Text("%-20s: %.2f", "Tooth Sharpness", phenotype.getTrait(UG::TOOTH_SHARPNESS));
                    ImGui::Text("%-20s: %.2f", "Tooth Grinding", phenotype.getTrait(UG::TOOTH_GRINDING));
                    ImGui::Text("%-20s: %.2f", "Gut Length", phenotype.getTrait(UG::GUT_LENGTH));
                    ImGui::Text("%-20s: %.2f", "Jaw Strength", phenotype.getTrait(UG::JAW_STRENGTH));
                    ImGui::Text("%-20s: %.2f", "Tail Length", phenotype.getTrait(UG::TAIL_LENGTH));
                    ImGui::Columns(1);
                    
                    ImGui::Spacing();
                    
                    // --- MOBILITY ---
                    ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "MOBILITY");
                    ImGui::Separator();
                    
                    ImGui::Text("Locomotion: %.2f", phenotype.getTrait(UG::LOCOMOTION));
                    ImGui::Text("Navigation: %.2f", phenotype.getTrait(UG::NAVIGATION_ABILITY));
                    ImGui::Text("Speed: %d", creature->getSpeed());
                    
                    ImGui::Spacing();
                    
                    // --- SENSORY ABILITIES ---
                    ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "SENSORY ABILITIES");
                    ImGui::Separator();
                    
                    ImGui::Text("Sight Range: %.1f", phenotype.getTrait(UG::SIGHT_RANGE));
                    ImGui::Text("Color Vision: %.2f", phenotype.getTrait(UG::COLOR_VISION));
                    ImGui::Text("Scent Detection: %.2f", phenotype.getTrait(UG::SCENT_DETECTION));
                    ImGui::Text("Olfactory Acuity: %.2f", phenotype.getTrait(UG::OLFACTORY_ACUITY));
                    
                    ImGui::Spacing();
                    
                    // --- APPEARANCE ---
                    ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.7f, 1.0f), "APPEARANCE");
                    ImGui::Separator();
                    
                    ImGui::Text("Color Hue: %.2f", phenotype.getTrait(UG::COLOR_HUE));
                    
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome - limited physical data");
                    ImGui::Text("Speed: %d", creature->getSpeed());
                }
                
                ImGui::EndTabItem();
            }
            
            // ==========================================
            // TAB 5: GENETICS (with 12 Sub-Tabs)
            // ==========================================
            if (ImGui::BeginTabItem("Genetics")) {
                ImGui::Spacing();
                
                if (creature->getPhenotype()) {
                    const auto& phenotype = *creature->getPhenotype();
                    using UG = EcoSim::Genetics::UniversalGenes;
                    
                    // Nested tab bar for chromosome categories
                    if (ImGui::BeginTabBar("GeneticsSubTabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
                        
                        // Sub-Tab 1: Universal (7 genes)
                        if (ImGui::BeginTabItem("Universal")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Universal Genes (7)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "LIFESPAN", phenotype.getTrait(UG::LIFESPAN));
                            ImGui::Text("%-28s: %.2f", "MAX_SIZE", phenotype.getTrait(UG::MAX_SIZE));
                            ImGui::Text("%-28s: %.2f", "METABOLISM_RATE", phenotype.getTrait(UG::METABOLISM_RATE));
                            ImGui::Text("%-28s: %.2f", "COLOR_HUE", phenotype.getTrait(UG::COLOR_HUE));
                            ImGui::Text("%-28s: %.2f", "HARDINESS", phenotype.getTrait(UG::HARDINESS));
                            ImGui::Text("%-28s: %.2f", "TEMP_TOLERANCE_LOW", phenotype.getTrait(UG::TEMP_TOLERANCE_LOW));
                            ImGui::Text("%-28s: %.2f", "TEMP_TOLERANCE_HIGH", phenotype.getTrait(UG::TEMP_TOLERANCE_HIGH));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 2: Mobility (5 genes)
                        if (ImGui::BeginTabItem("Mobility")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Mobility Genes (5)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "LOCOMOTION", phenotype.getTrait(UG::LOCOMOTION));
                            ImGui::Text("%-28s: %.2f", "SIGHT_RANGE", phenotype.getTrait(UG::SIGHT_RANGE));
                            ImGui::Text("%-28s: %.2f", "NAVIGATION_ABILITY", phenotype.getTrait(UG::NAVIGATION_ABILITY));
                            ImGui::Text("%-28s: %.2f", "FLEE_THRESHOLD", phenotype.getTrait(UG::FLEE_THRESHOLD));
                            ImGui::Text("%-28s: %.2f", "PURSUE_THRESHOLD", phenotype.getTrait(UG::PURSUE_THRESHOLD));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 3: Heterotrophy (13 genes)
                        if (ImGui::BeginTabItem("Heterotrophy")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Heterotrophy Genes (13)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "PLANT_DIGESTION_EFFICIENCY", phenotype.getTrait(UG::PLANT_DIGESTION_EFFICIENCY));
                            ImGui::Text("%-28s: %.2f", "MEAT_DIGESTION_EFFICIENCY", phenotype.getTrait(UG::MEAT_DIGESTION_EFFICIENCY));
                            ImGui::Text("%-28s: %.2f", "CELLULOSE_BREAKDOWN", phenotype.getTrait(UG::CELLULOSE_BREAKDOWN));
                            ImGui::Text("%-28s: %.2f", "TOXIN_TOLERANCE", phenotype.getTrait(UG::TOXIN_TOLERANCE));
                            ImGui::Text("%-28s: %.2f", "TOXIN_METABOLISM", phenotype.getTrait(UG::TOXIN_METABOLISM));
                            ImGui::Text("%-28s: %.2f", "SCENT_DETECTION", phenotype.getTrait(UG::SCENT_DETECTION));
                            ImGui::Text("%-28s: %.2f", "COLOR_VISION", phenotype.getTrait(UG::COLOR_VISION));
                            ImGui::Text("%-28s: %.2f", "HUNT_INSTINCT", phenotype.getTrait(UG::HUNT_INSTINCT));
                            ImGui::Text("%-28s: %.2f", "DIGESTIVE_EFFICIENCY", phenotype.getTrait(UG::DIGESTIVE_EFFICIENCY));
                            ImGui::Text("%-28s: %.2f", "NUTRIENT_VALUE", phenotype.getTrait(UG::NUTRIENT_VALUE));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 4: Morphology (9 genes)
                        if (ImGui::BeginTabItem("Morphology")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Morphology Genes (9)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "HIDE_THICKNESS", phenotype.getTrait(UG::HIDE_THICKNESS));
                            ImGui::Text("%-28s: %.2f", "FUR_DENSITY", phenotype.getTrait(UG::FUR_DENSITY));
                            ImGui::Text("%-28s: %.2f", "SCALE_COVERAGE", phenotype.getTrait(UG::SCALE_COVERAGE));
                            ImGui::Text("%-28s: %.2f", "FAT_LAYER_THICKNESS", phenotype.getTrait(UG::FAT_LAYER_THICKNESS));
                            ImGui::Text("%-28s: %.2f", "TOOTH_SHARPNESS", phenotype.getTrait(UG::TOOTH_SHARPNESS));
                            ImGui::Text("%-28s: %.2f", "TOOTH_GRINDING", phenotype.getTrait(UG::TOOTH_GRINDING));
                            ImGui::Text("%-28s: %.2f", "GUT_LENGTH", phenotype.getTrait(UG::GUT_LENGTH));
                            ImGui::Text("%-28s: %.2f", "JAW_STRENGTH", phenotype.getTrait(UG::JAW_STRENGTH));
                            ImGui::Text("%-28s: %.2f", "TAIL_LENGTH", phenotype.getTrait(UG::TAIL_LENGTH));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 5: Seed Interaction (2 genes)
                        if (ImGui::BeginTabItem("Seed")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Seed Interaction Genes (2)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "GUT_TRANSIT_TIME", phenotype.getTrait(UG::GUT_TRANSIT_TIME));
                            ImGui::Text("%-28s: %.2f", "SEED_DESTRUCTION_RATE", phenotype.getTrait(UG::SEED_DESTRUCTION_RATE));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 6: Reproduction (6 genes)
                        if (ImGui::BeginTabItem("Reproduction")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Reproduction Genes (6)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "OFFSPRING_COUNT", phenotype.getTrait(UG::OFFSPRING_COUNT));
                            ImGui::Text("%-28s: %.2f", "MATE_THRESHOLD", phenotype.getTrait(UG::MATE_THRESHOLD));
                            ImGui::Text("%-28s: %.2f", "SPREAD_DISTANCE", phenotype.getTrait(UG::SPREAD_DISTANCE));
                            ImGui::Text("%-28s: %.2f", "FATIGUE_THRESHOLD", phenotype.getTrait(UG::FATIGUE_THRESHOLD));
                            ImGui::Text("%-28s: %.2f", "COMFORT_INCREASE", phenotype.getTrait(UG::COMFORT_INCREASE));
                            ImGui::Text("%-28s: %.2f", "COMFORT_DECREASE", phenotype.getTrait(UG::COMFORT_DECREASE));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 7: Olfactory (4 genes)
                        if (ImGui::BeginTabItem("Olfactory")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Olfactory Genes (4)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "SCENT_PRODUCTION", phenotype.getTrait(UG::SCENT_PRODUCTION));
                            ImGui::Text("%-28s: %.2f", "SCENT_SIGNATURE_VARIANCE", phenotype.getTrait(UG::SCENT_SIGNATURE_VARIANCE));
                            ImGui::Text("%-28s: %.2f", "OLFACTORY_ACUITY", phenotype.getTrait(UG::OLFACTORY_ACUITY));
                            ImGui::Text("%-28s: %.2f", "SCENT_MASKING", phenotype.getTrait(UG::SCENT_MASKING));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 8: Behavior (5 genes)
                        if (ImGui::BeginTabItem("Behavior")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Behavior Genes (5)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "CACHING_INSTINCT", phenotype.getTrait(UG::CACHING_INSTINCT));
                            ImGui::Text("%-28s: %.2f", "GROOMING_FREQUENCY", phenotype.getTrait(UG::GROOMING_FREQUENCY));
                            ImGui::Text("%-28s: %.2f", "SPATIAL_MEMORY", phenotype.getTrait(UG::SPATIAL_MEMORY));
                            ImGui::Text("%-28s: %.2f", "SWEETNESS_PREFERENCE", phenotype.getTrait(UG::SWEETNESS_PREFERENCE));
                            ImGui::Text("%-28s: %.2f", "PAIN_SENSITIVITY", phenotype.getTrait(UG::PAIN_SENSITIVITY));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 9: Health (3 genes)
                        if (ImGui::BeginTabItem("Health")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Health Genes (3)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "REGENERATION_RATE", phenotype.getTrait(UG::REGENERATION_RATE));
                            ImGui::Text("%-28s: %.2f", "WOUND_TOLERANCE", phenotype.getTrait(UG::WOUND_TOLERANCE));
                            ImGui::Text("%-28s: %.2f", "BLEEDING_RESISTANCE", phenotype.getTrait(UG::BLEEDING_RESISTANCE));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 10: Combat Weapons (13 genes)
                        if (ImGui::BeginTabItem("Cbt:Weapons")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Combat Weapon Genes (13)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "TEETH_SHARPNESS", phenotype.getTrait(UG::TEETH_SHARPNESS));
                            ImGui::Text("%-28s: %.2f", "TEETH_SERRATION", phenotype.getTrait(UG::TEETH_SERRATION));
                            ImGui::Text("%-28s: %.2f", "TEETH_SIZE", phenotype.getTrait(UG::TEETH_SIZE));
                            ImGui::Text("%-28s: %.2f", "CLAW_LENGTH", phenotype.getTrait(UG::CLAW_LENGTH));
                            ImGui::Text("%-28s: %.2f", "CLAW_SHARPNESS", phenotype.getTrait(UG::CLAW_SHARPNESS));
                            ImGui::Text("%-28s: %.2f", "CLAW_CURVATURE", phenotype.getTrait(UG::CLAW_CURVATURE));
                            ImGui::Text("%-28s: %.2f", "HORN_LENGTH", phenotype.getTrait(UG::HORN_LENGTH));
                            ImGui::Text("%-28s: %.2f", "HORN_POINTINESS", phenotype.getTrait(UG::HORN_POINTINESS));
                            ImGui::Text("%-28s: %.2f", "HORN_SPREAD", phenotype.getTrait(UG::HORN_SPREAD));
                            ImGui::Text("%-28s: %.2f", "TAIL_LENGTH", phenotype.getTrait(UG::TAIL_LENGTH));
                            ImGui::Text("%-28s: %.2f", "TAIL_MASS", phenotype.getTrait(UG::TAIL_MASS));
                            ImGui::Text("%-28s: %.2f", "TAIL_SPINES", phenotype.getTrait(UG::TAIL_SPINES));
                            ImGui::Text("%-28s: %.2f", "BODY_SPINES", phenotype.getTrait(UG::BODY_SPINES));
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 11: Combat Defense (2 genes)
                        if (ImGui::BeginTabItem("Cbt:Defense")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Combat Defense Genes (2)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "SCALE_COVERAGE", phenotype.getTrait(UG::SCALE_COVERAGE));
                            ImGui::Text("%-28s: %.2f", "FAT_LAYER_THICKNESS", phenotype.getTrait(UG::FAT_LAYER_THICKNESS));
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(HIDE_THICKNESS in Morphology)");
                            ImGui::EndTabItem();
                        }
                        
                        // Sub-Tab 12: Combat Behavior (4 genes)
                        if (ImGui::BeginTabItem("Cbt:Behavior")) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "Combat Behavior Genes (4)");
                            ImGui::Separator();
                            ImGui::Text("%-28s: %.2f", "COMBAT_AGGRESSION", phenotype.getTrait(UG::COMBAT_AGGRESSION));
                            ImGui::Text("%-28s: %.2f", "RETREAT_THRESHOLD", phenotype.getTrait(UG::RETREAT_THRESHOLD));
                            ImGui::Text("%-28s: %.2f", "TERRITORIAL_AGGRESSION", phenotype.getTrait(UG::TERRITORIAL_AGGRESSION));
                            ImGui::Text("%-28s: %.2f", "PACK_COORDINATION", phenotype.getTrait(UG::PACK_COORDINATION));
                            ImGui::EndTabItem();
                        }
                        
                        ImGui::EndTabBar();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Legacy genome - genetics data not available");
                    ImGui::Spacing();
                    ImGui::Text("This creature uses the old genome system.");
                    ImGui::Text("Create new creatures to see full genetic data.");
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void ImGuiOverlay::renderControlsWindow() {
    ImGui::SetNextWindowPos(ImVec2(270, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Controls", &_showControls, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Simulation Controls");
        ImGui::Separator();
        
        ImGui::Text("Tick Rate: %d", _lastHudData.tickRate);
        ImGui::Text("Paused: %s", _lastHudData.paused ? "Yes" : "No");
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Keyboard Shortcuts:");
        ImGui::Text("Space - Pause/Resume");
        ImGui::Text("+/- - Adjust tick rate");
        ImGui::Text("F1-F6 - Toggle panels");
    }
    ImGui::End();
}

//==============================================================================
// Helper Methods
//==============================================================================

void ImGuiOverlay::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Dark theme with nature-inspired accents
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    
    ImVec4* colors = style.Colors;
    
    // Window backgrounds - dark with slight transparency
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.10f, 0.08f, 0.94f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.10f, 0.08f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.08f, 0.06f, 0.90f);
    
    // Headers - forest green tones
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.30f, 0.15f, 0.80f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.40f, 0.20f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.50f, 0.25f, 0.80f);
    
    // Buttons - earth tones
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.18f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.35f, 0.25f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.45f, 0.30f, 0.80f);
    
    // Frames - subtle borders
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.15f, 0.12f, 0.80f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.22f, 0.18f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.28f, 0.24f, 0.80f);
    
    // Title bar - darker
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.08f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.15f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.04f, 0.05f, 0.04f, 0.75f);
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.12f, 0.10f, 1.00f);
    
    // Tabs - nature tones
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.18f, 0.12f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.35f, 0.20f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.30f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.14f, 0.10f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.22f, 0.14f, 1.00f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.08f, 0.06f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.20f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.35f, 0.28f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.45f, 0.35f, 0.80f);
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.50f, 0.30f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.60f, 0.40f, 0.80f);
    
    // Check mark
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.30f, 0.25f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.45f, 0.35f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.55f, 0.45f, 1.00f);
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.92f, 0.88f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.48f, 1.00f);
    
    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.30f, 0.25f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Plot colors
    colors[ImGuiCol_PlotLines] = ImVec4(0.50f, 0.80f, 0.50f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.70f, 1.00f, 0.70f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.55f, 0.85f, 0.55f, 1.00f);
}

const char* ImGuiOverlay::getProfileName(int profile) const {
    switch (static_cast<Motivation>(profile)) {
        case Motivation::Hungry:  return "Hungry";
        case Motivation::Thirsty: return "Thirsty";
        case Motivation::Amorous: return "Amorous";
        case Motivation::Tired:   return "Tired";
        case Motivation::Content: return "Content";
        default:                  return "Unknown";
    }
}

const char* ImGuiOverlay::getDietName(EcoSim::Genetics::DietType diet) const {
    return getEmergentDietName(diet);
}

const char* ImGuiOverlay::getEmergentDietName(EcoSim::Genetics::DietType diet) const {
    switch (diet) {
        case EcoSim::Genetics::DietType::HERBIVORE:  return "Herbivore";
        case EcoSim::Genetics::DietType::FRUGIVORE:  return "Frugivore";
        case EcoSim::Genetics::DietType::OMNIVORE:   return "Omnivore";
        case EcoSim::Genetics::DietType::CARNIVORE:  return "Carnivore";
        case EcoSim::Genetics::DietType::NECROVORE:  return "Necrovore";
        default:                                      return "Unknown";
    }
}
