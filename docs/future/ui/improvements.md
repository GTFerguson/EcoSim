---
title: UI & Rendering Polish Backlog
created: 2025-12-24
updated: 2026-04-14
status: active
tags: [ui, rendering, polish, backlog]
---

# UI & Rendering Polish Backlog

Narrow polish items for the UI and rendering layers. **This is a polish backlog, not the visibility-systems plan** — see [[visibility-systems]] for the real-time world-visibility framework (inspection, population graphs, gene-pool views, family trees, event feeds, map overlays, procedural creature appearance) which is the MVP-blocking scope.

> [!NOTE]
> **Scope clarification (2026-04-14):** Several items previously on this list have been **promoted to the visibility-systems plan** because they are feature-scope rather than polish-scope: scent/sound layer visualisation toggles (map overlays), size affecting visual appearance (creature appearance), click-to-select creature (inspection framework). This doc is now focused only on narrow tactical polish.

---

## Already shipped (audit 2026-04-14)

- ✅ **Fullscreen right pane positioning** — `io.DisplaySize`-based math at `src/rendering/backends/sdl2/ImGuiOverlay.cpp:852-853`
- ✅ **Mac fullscreen menu bar collision** — `SDL_WINDOW_FULLSCREEN_DESKTOP` in use at `src/rendering/backends/sdl2/SDL2Renderer.cpp:81`

---

## High priority

- [ ] **Stress not shown in inspector** — Missing stress display in creature panel. Add to `renderCreatureInspectorWindow()` in `src/rendering/backends/sdl2/ImGuiOverlay.cpp`. Note: `Creature._currentEnvironmentalStress` exists at `include/objects/creature/creature.hpp:190` but is not surfaced.

---

## Medium priority

- [ ] **Zoom center point** — Zoom doesn't center on the viewport. Calculate viewport center before zoom, adjust camera position after.

---

## Low priority

- [ ] **Vegetation map layer** — A separate visualization layer for plant distribution. This is a narrower sibling of the map overlays in the visibility-systems plan; probably belongs there once that plan starts, but listed here as a polish tracker for now.
- [ ] **ESC menu popup** — No escape key menu for pause/settings/quit. Add ESC handler for modal dialog with Resume / Settings / Save / Quit options.

---

## Promoted to visibility-systems plan

These items have been moved out of this polish backlog because they are feature-scope, not polish-scope. They are now part of [[visibility-systems]]:

- **Click to select creature** — part of the real-time inspection framework (visibility-systems cluster 1)
- **Scent / sound layer visualisation toggles** — part of map overlays (visibility-systems cluster 6)
- **Size affecting visual appearance / creature appearance generally** — part of the procedural creature appearance feature (visibility-systems cross-cutting foundation)

---

## Implementation Notes

### Backend Considerations

Most UI polish items should be implemented in the SDL2 backend first:

- [`ImGuiOverlay.hpp`](include/rendering/backends/sdl2/ImGuiOverlay.hpp)
- [`ImGuiOverlay.cpp`](src/rendering/backends/sdl2/ImGuiOverlay.cpp)

NCurses backend has more limited UI capabilities but should maintain parity where possible.

---

## See Also

- [[visibility-systems]] - **MVP-blocking visibility framework** (distinct from this polish backlog)
- [[genetics/improvements]] - Genetics-specific improvements
- [[environment/ecosystem-improvements]] - Creature behavior and world mechanics
