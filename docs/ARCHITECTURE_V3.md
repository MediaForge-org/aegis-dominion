# AEGIS DOMINION architecture

## Current boundary

The repository is a playable SFML prototype in transition. The current stable boundary is:

```text
renderer-independent                 SFML-facing

core/MapDocument  <──  Map/GameMap  ──>  Game, sprites and effects
       ↑
editor/MapEditorModel                     ui/TextService
```

`src/core/` contains no SFML, OpenGL or renderer types. `MapDocument` uses semantic world coordinates and is the persistence boundary for maps. `GameMap` is currently an SFML adapter: it projects the first document path into the legacy gameplay representation while keeping the complete document available.

## Implemented layers

- `src/core/`: versioned map data, serialization and structural validation.
- `src/editor/`: renderer-independent editor model and bounded undo/redo history.
- `src/ui/`: centralized font discovery, German glyph validation and UTF-8-to-SFML conversion.
- Legacy root `src/` files: playable simulation, SFML rendering, assets and screen flow. These remain functional but are not the final architecture.

The same `MapDocument` is intended for the current 2D renderer, MAP FORGE, tests/tooling and a future 2.5D/3D renderer.

## Text boundary

Repository and map strings are UTF-8 `std::string`. `ui::TextService` is the only current constructor path for `sf::Text`; it converts with `sf::String::fromUtf8` and rejects fonts that lack ä, ö, ü, Ä, Ö, Ü or ß. It first checks `assets/fonts/`, then known system-font paths and finally other installed fonts.

## Map boundary

`.aegismap` V1 stores metadata, dimensions, environment, paths, spawns, goals, rectangular zones and decorations. Unknown headers are rejected. Saving a document with an unsupported declared version is rejected rather than silently writing V1. See `MAP_FORMAT.md`.

World positions are map-space coordinates. Current maps use a 1200×900 plane, but `MapDocument` does not expose screen/pixel or SFML types. Elevation, terrain layers and 3D transforms require a future compatible format extension before MAP FORGE exposes those tools.

## Known architecture debt

- `Game` still owns state transitions, input, rendering and match orchestration; screen extraction is required before adding MAP FORGE.
- Gameplay entities still use SFML vectors and draw themselves. Simulation/render views must be separated in a later phase.
- `GameMap` consumes only the first path for legacy gameplay and still uses fixed world-panel constants.
- Asset loading is centralized in `Assets`, but stable typed asset IDs, fallbacks, logging and definitions are not yet implemented.
- Waves and balance values remain hard-coded.
- Editor history uses general document checkpoints. It is functional for current actions but should evolve into explicit commands as editing operations become richer.

These constraints are deliberate follow-up work; no unimplemented screen or system is represented as complete.
