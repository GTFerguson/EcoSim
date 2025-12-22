# EcoSim ImGui UI Layout Redesign Plan

**Author:** Design Phase  
**Date:** December 2024  
**Status:** Draft - Awaiting Review

---

## 1. Executive Summary

This document outlines a redesign of the ImGui panel layout to address user feedback regarding scattered panels, unnecessary clutter, and inefficient use of screen space.

### Goals
- Organize panels into logical left/right columns
- Maximize world view area in the center
- Reduce default visual clutter
- Eliminate SDL2 HUD redundancy with ImGui panels

---

## 2. Current vs Proposed Layout

### 2.1 ASCII Diagram - Current Layout (Issues)

```
+------------------------------- 1280 x 720 ---------------------------------+
| [Menu Bar: Windows | View | Help]                           FPS | Pop     |
+--------------------+------------------------------------------------------+
|                    |                                                       |
| [Statistics]       |  [World Info]                                         |
| (10,30)            |  (340,30)                                             |
| 400x550            |  300x350        <-- Overlapping/floating              |
|                    |                                                       |
|                    |                      WORLD VIEW                       |
|                    |                      (partially obscured)             |
|                    |                                                       |
+--------------------+-------------------+----------------------------------+
| [Performance]      | [Creature List]   |                                   |
| (10,490)           | (10,500)          |  [Creature Inspector]            |
| 320x180            | 380x350           |  (400,500)                       |
|                    | <-- overlapping!  |  350x400                         |
+--------------------+-------------------+----------------------------------+
| TIME: 12:00:00  | Date: Day 1, Spring | World: 500x500 |       [PAUSED]   |
+----------------------------------------------------------------------------+
                          ^^^ SDL2 HUD (redundant info)
```

**Problems Identified:**
1. World Info panel floats in center, obscuring world view
2. Performance panel open by default (unnecessary)
3. Creature List overlaps with Performance panel
4. Creature Inspector positioned awkwardly
5. SDL2 HUD duplicates info already in ImGui panels
6. World view doesn't maximize available space

---

### 2.2 ASCII Diagram - Proposed Layout

```
+------------------------------- 1280 x 720 ---------------------------------+
| [Menu Bar: Windows | View | Help]              FPS: 60.0 | PAUSED | Pop: X |
+------------+-----------------------------------------------+---------------+
|            |                                               |               |
| STATISTICS |                                               | WORLD INFO    |
| PANEL      |                                               | (collapsed    |
|            |                                               |  sections)    |
| - Time     |                                               | 280x180       |
| - Date     |              WORLD VIEW                       |               |
| - Pop      |                                               +---------------+
| - Deaths   |              (MAXIMIZED)                      |               |
| - Births   |                                               | CREATURE LIST |
|            |              ~700px width                     | (default      |
| - Graphs   |              ~680px height                    |  closed)      |
|            |                                               | 280x300       |
| 260x680    |                                               |               |
|            |                                               +---------------+
|            |                                               | CREATURE      |
|            |                                               | INSPECTOR     |
|            |                                               | (on select)   |
|            |                                               | 280x220       |
+------------+-----------------------------------------------+---------------+
```

**Key Changes:**
1. ✅ Statistics panel stays LEFT (as requested)
2. ✅ World Info moves to RIGHT (top of right column)
3. ✅ Creature List moves to RIGHT (middle, default closed)
4. ✅ Creature Inspector on RIGHT (bottom, appears on selection)
5. ✅ Performance panel closed by default
6. ✅ SDL2 HUD hidden completely (info in ImGui)
7. ✅ World view maximized in center

---

## 3. Panel Specifications

### 3.1 Screen Layout Summary (1280x720)

| Region | X Start | X End | Width | Purpose |
|--------|---------|-------|-------|---------|
| Left Column | 0 | 260 | 260px | Statistics |
| Center | 260 | 980 | 720px | World View |
| Right Column | 980 | 1280 | 300px | Info/List/Inspector |
| Menu Bar | 0 | 1280 | 1280px | 20px height |

### 3.2 Panel Specifications Table

| Panel | Default State | Position (x, y) | Size (w x h) | Shortcut |
|-------|--------------|-----------------|--------------|----------|
| **Statistics** | Open | (5, 25) | 250 x 690 | F1 |
| **World Info** | Open | (975, 25) | 300 x 180 | F2 |
| **Performance** | **Closed** | (975, 25)* | 300 x 180 | F3 |
| **Creature List** | **Closed** | (975, 210) | 300 x 280 | F4 |
| **Creature Inspector** | Closed | (975, 495) | 300 x 220 | F5 |
| **Controls** | Closed | (975, 25)* | 300 x 280 | F6 |

*Note: Performance and Controls share position with World Info (only one visible at a time in top-right slot, or they stack)

### 3.3 Default Visibility Changes

| Panel | Current Default | New Default | Reason |
|-------|----------------|-------------|--------|
| Statistics | Open | Open | Primary info - keep visible |
| World Info | Open | Open | Essential world data |
| Performance | **Open** | **Closed** | Rarely needed, reduces clutter |
| Creature List | Closed | Closed | On-demand usage |
| Creature Inspector | Closed | Closed | Opens on creature selection |
| Controls | Closed | Closed | On-demand usage |

---

## 4. Behavioral Specifications

### 4.1 Panel Stacking on Right Side

When multiple right-side panels are open, they stack vertically:

```
Position Priority (top to bottom):
1. World Info     (Y = 25)
2. Creature List  (Y = 210, or below World Info if open)
3. Creature Inspector (Y = 495, or below Creature List)
```

**Dynamic Positioning Logic:**
```cpp
// Pseudo-code for right column positioning
int rightColumnX = screenWidth - 305;  // 5px margin from edge
int nextY = 25;  // Start below menu bar

if (_showWorldInfo) {
    // Position World Info
    SetNextWindowPos(rightColumnX, nextY);
    nextY += worldInfoHeight + 5;
}

if (_showCreatureList) {
    // Position Creature List below World Info
    SetNextWindowPos(rightColumnX, nextY);
    nextY += creatureListHeight + 5;
}

if (_showCreatureInspector) {
    // Position Inspector below Creature List
    SetNextWindowPos(rightColumnX, nextY);
}
```

### 4.2 Creature Inspector Auto-Open Behavior

When a creature is selected (from list or world click):
1. Set `_selectedCreatureId = creatureIndex`
2. Set `_showCreatureInspector = true` (auto-open)
3. Inspector appears in right column stack

When creature is deselected or dies:
1. Set `_selectedCreatureId = -1`
2. Keep `_showCreatureInspector` state (user can close manually)

### 4.3 Keyboard Shortcuts

| Key | Action |
|-----|--------|
| F1 | Toggle Statistics panel |
| F2 | Toggle World Info panel |
| F3 | Toggle Performance panel |
| F4 | Toggle Creature List panel |
| F5 | Toggle Creature Inspector panel |
| F6 | Toggle Controls panel |
| H | Toggle ALL panels (existing behavior) |

---

## 5. SDL2 HUD Hiding

### 5.1 Current HUD Implementation

Location: [`SDL2Renderer::renderHUD()`](src/rendering/backends/sdl2/SDL2Renderer.cpp:387)

Currently renders at bottom of screen:
- Population statistics
- Death breakdown
- Time and date
- World dimensions
- Pause indicator

### 5.2 Proposed Change

Add a flag to conditionally hide the SDL2 text HUD when ImGui is active:

```cpp
// In SDL2Renderer.hpp
bool _hideTextHUD = true;  // Hide when ImGui active

// In SDL2Renderer::renderHUD()
void SDL2Renderer::renderHUD(const HUDData& data) {
    // Always render ImGui overlay (has all the info)
    renderImGuiOverlay(data, _currentWorld);
    
    // Only render text HUD if ImGui is not available or explicitly requested
    #ifdef ECOSIM_HAS_IMGUI
    if (_imguiOverlay != nullptr && _hideTextHUD) {
        return;  // Skip text HUD - ImGui has everything
    }
    #endif
    
    // ... existing text HUD code (for non-ImGui builds)
}
```

### 5.3 Information Coverage Check

Ensure ImGui panels cover all SDL2 HUD info:

| SDL2 HUD Info | ImGui Location | Status |
|---------------|----------------|--------|
| Population count | Statistics panel + Menu bar | ✅ Covered |
| Births | Statistics panel | ✅ Covered |
| Food Eaten | Statistics panel | ✅ Covered |
| Deaths (all types) | Statistics panel | ✅ Covered |
| Time | Statistics panel | ✅ Covered |
| Date | Statistics panel | ✅ Covered |
| World dimensions | World Info panel | ✅ Covered |
| Pause indicator | Menu bar + Controls | ✅ Covered |
| Tick rate | Statistics panel | ✅ Covered |

**All information is covered by ImGui panels - safe to hide SDL2 HUD.**

---

## 6. Implementation Changes

### 6.1 Files to Modify

| File | Changes Required |
|------|------------------|
| [`src/rendering/backends/sdl2/ImGuiOverlay.cpp`](src/rendering/backends/sdl2/ImGuiOverlay.cpp) | Update default states, window positions, add dynamic stacking |
| [`include/rendering/backends/sdl2/ImGuiOverlay.hpp`](include/rendering/backends/sdl2/ImGuiOverlay.hpp) | Add any new member variables if needed |
| [`src/rendering/backends/sdl2/SDL2Renderer.cpp`](src/rendering/backends/sdl2/SDL2Renderer.cpp) | Conditionally hide text HUD |
| [`include/rendering/backends/sdl2/SDL2Renderer.hpp`](include/rendering/backends/sdl2/SDL2Renderer.hpp) | Add `_hideTextHUD` flag |

### 6.2 Specific Code Changes

#### 6.2.1 Default States (ImGuiOverlay.cpp constructor)

```cpp
// BEFORE (line 31-36)
, _showStatistics(true)
, _showWorldInfo(true)
, _showCreatureList(false)
, _showPerformance(true)    // <-- CHANGE THIS
, _showCreatureInspector(false)
, _showControls(false)

// AFTER
, _showStatistics(true)
, _showWorldInfo(true)
, _showCreatureList(false)
, _showPerformance(false)   // <-- Closed by default
, _showCreatureInspector(false)
, _showControls(false)
```

#### 6.2.2 Statistics Panel Position (line ~286)

```cpp
// BEFORE
ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(400, 550), ImGuiCond_FirstUseEver);

// AFTER - Narrower, full height on left
ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(250, 690), ImGuiCond_FirstUseEver);
```

#### 6.2.3 World Info Panel Position (line ~434-436)

```cpp
// BEFORE
ImGui::SetNextWindowPos(ImVec2(340, 30), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);

// AFTER - Right side, top
ImGui::SetNextWindowPos(ImVec2(975, 25), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
```

#### 6.2.4 Creature List Panel Position (line ~551-553)

```cpp
// BEFORE
ImGui::SetNextWindowPos(ImVec2(10, 500), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(380, 350), ImGuiCond_FirstUseEver);

// AFTER - Right side, below World Info
ImGui::SetNextWindowPos(ImVec2(975, 210), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(300, 280), ImGuiCond_FirstUseEver);
```

#### 6.2.5 Creature Inspector Position (line ~713-715)

```cpp
// BEFORE
ImGui::SetNextWindowPos(ImVec2(400, 500), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

// AFTER - Right side, bottom
ImGui::SetNextWindowPos(ImVec2(975, 495), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(300, 220), ImGuiCond_FirstUseEver);
```

#### 6.2.6 Performance Panel Position (line ~510-512)

```cpp
// BEFORE
ImGui::SetNextWindowPos(ImVec2(10, 490), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(320, 180), ImGuiCond_FirstUseEver);

// AFTER - Right side (optional position when opened)
ImGui::SetNextWindowPos(ImVec2(975, 25), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
```

#### 6.2.7 SDL2 HUD Hiding (SDL2Renderer.cpp)

```cpp
void SDL2Renderer::renderHUD(const HUDData& data) {
    if (!_initialized) {
        return;
    }
    
    // Render ImGui overlay panels with creature data
    renderImGuiOverlay(data, _currentWorld);
    
    // Skip text HUD when ImGui is active - it has all the information
#ifdef ECOSIM_HAS_IMGUI
    if (_imguiOverlay != nullptr) {
        return;  // ImGui provides all HUD info
    }
#endif
    
    // ... rest of text HUD code (only for non-ImGui builds)
}
```

---

## 7. Visual Mockup

### 7.1 Default View (Statistics + World Info open)

```
+------------------------------- 1280 x 720 ---------------------------------+
| Windows | View | Help                          FPS: 60.0 | Pop: 150        |
+------------+-----------------------------------------------+---------------+
|            |                                               |               |
| Statistics |                                               | World Info    |
| ──────────|                                               | ────────────  |
| Time: 12:00|                                               | Dimensions    |
| Day 1, Spr |                                               | 500 x 500     |
|            |                                               |               |
| Population |              [World View]                     | Generation    |
| ──────────|                                               | Seed: 42.0    |
| Count: 150 |              Terrain, creatures,              | Scale: 0.01   |
| Dynamics:  |              food rendered here               |               |
|  +3 births |                                               | Population    |
|  -1 deaths |              Maximum visibility               | Herbivores: 80|
|            |                                               | Carnivores: 20|
| [Graphs]   |                                               +---------------+
| ▁▂▃▄▅▆▇█   |                                               |               |
|            |                                               |               |
| Lifetime   |                                               |               |
| ──────────|                                               |               |
|            |                                               |               |
+------------+-----------------------------------------------+---------------+
```

### 7.2 Creature Selected View (Inspector auto-opens)

```
+------------------------------- 1280 x 720 ---------------------------------+
| Windows | View | Help                          FPS: 60.0 | Pop: 150        |
+------------+-----------------------------------------------+---------------+
|            |                                               | World Info    |
| Statistics |                                               | (collapsed)   |
|            |                                               +---------------+
|            |                                               | Creature List |
|            |              [World View]                     | #0 [12,34]    |
|            |                                               | #1 [56,78] <- |
|            |              Creature #1 highlighted          | #2 [90,12]    |
|            |              with yellow border               |               |
|            |                                               +---------------+
|            |                                               | Inspector #1  |
|            |                                               | ────────────  |
|            |                                               | Pos: (56,78)  |
|            |                                               | Age: 45/100   |
|            |                                               | Hunger: ████░ |
|            |                                               | Thirst: ███░░ |
|            |                                               | [Deselect]    |
+------------+-----------------------------------------------+---------------+
```

---

## 8. Testing Checklist

After implementation, verify:

- [ ] Statistics panel appears on left at startup
- [ ] World Info panel appears on right at startup
- [ ] Performance panel is closed by default
- [ ] Creature List is closed by default
- [ ] F1-F6 shortcuts toggle correct panels
- [ ] Creature selection opens Inspector automatically
- [ ] Inspector shows in right column stack
- [ ] SDL2 text HUD is not visible
- [ ] World view has maximum screen space
- [ ] Panels don't overlap world view significantly
- [ ] All original HUD info accessible in ImGui panels

---

## 9. Future Enhancements (Out of Scope)

- Dockable panels (ImGui docking branch)
- Save/load panel layouts
- Resizable column widths
- Multiple monitor support
- Theme customization

---

## 10. Approval

**Ready for Review**

Please confirm this layout plan before implementation proceeds.

- [ ] Layout positions approved
- [ ] Default states approved  
- [ ] SDL2 HUD hiding approved
- [ ] Ready to implement