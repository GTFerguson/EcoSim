---
title: UI & Rendering Improvements
created: 2025-12-24
updated: 2025-12-24
status: planned
tags: [ui, rendering, improvements, future]
---

# UI & Rendering Improvements

User interface and rendering improvements for EcoSim.

---

## High Priority

Issues significantly impacting user experience.

### Inspector Improvements

- [ ] **Click to select creature** - No way to select creatures by clicking in the world view
  - Add click detection, world-to-screen coordinate mapping
  - Should work in both ncurses and SDL2 backends

- [ ] **Stress not shown in inspector** - Missing stress display in creature panel
  - Add stress display to [`renderCreatureInspectorWindow()`](src/rendering/backends/sdl2/ImGuiOverlay.cpp:186)

### Window Management

- [ ] **Fullscreen right pane positioning** - Inspector panel positioned incorrectly in fullscreen
  - Calculate positions from `ImGui::GetIO().DisplaySize`

- [ ] **Mac fullscreen menu bar collision** - Menu bar overlaps with UI on macOS
  - Use `SDL_WINDOW_FULLSCREEN_DESKTOP` instead of true fullscreen

---

## Medium Priority

Feature enhancements and usability improvements.

### Camera & Navigation

- [ ] **Zoom center point** - Zoom doesn't center on viewport
  - Calculate viewport center before zoom, adjust camera position after

### Debug Visualization

- [ ] **Vegetation map layer** - No separate visualization layer for plant distribution
  - Add toggle in UI to show/hide vegetation density overlay

---

## Low Priority

Nice-to-have features for later consideration.

### Visual Enhancements

- [ ] **Size affecting visual appearance** - SIZE gene doesn't affect rendering
  - Scale sprite based on SIZE gene (SDL2 only feature)
  - Could use different characters in ncurses based on size threshold

- [ ] **Scent/sound layer visualization toggles** - No way to visualize sensory layers
  - Toggle buttons in UI
  - Render colored overlays for scent gradients
  - Sound visualization (when implemented)

### Menu System

- [ ] **ESC menu popup** - No escape key menu for pause/settings/quit
  - Add ESC handler for modal dialog with options
  - Options: Resume, Settings, Save, Quit

---

## Implementation Notes

### Backend Considerations

Most UI improvements should be implemented in the SDL2 backend first:
- [`ImGuiOverlay.hpp`](include/rendering/backends/sdl2/ImGuiOverlay.hpp)
- [`ImGuiOverlay.cpp`](src/rendering/backends/sdl2/ImGuiOverlay.cpp)

For click-to-select and camera features, consider:
- [`SDL2InputHandler`](include/rendering/backends/sdl2/SDL2InputHandler.hpp)
- [`SDL2Renderer`](include/rendering/backends/sdl2/SDL2Renderer.hpp)

NCurses backend has more limited UI capabilities but should maintain parity where possible.

---

## See Also

- [[blocking-issues]] - Critical blocking issues (includes inspector selection shift bug)
- [[genetics/improvements]] - Genetics-specific improvements
- [[environment/ecosystem-improvements]] - Creature behavior and world mechanics
