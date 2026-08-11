# AEGIS DOMINION – Codex repository instructions

## Product goal
Build a polished, highly replayable C++ tower-defense game. The current SFML version is a prototype foundation, not the visual quality bar.

## Non-negotiable architecture rules
- Keep gameplay simulation, map data, editor state, rendering and UI separated.
- `src/core/` must remain renderer-agnostic: no SFML, OpenGL or engine-specific types.
- Map files are data, never hard-coded screen coordinates inside gameplay classes.
- New map/editor features must be representable in `MapDocument` first, then rendered by the current renderer.
- Design all map data so a future 3D renderer can consume the same maps. Do not bake 2D textures into gameplay rules.
- Never remove working behavior just to simplify a refactor.
- Prefer small testable systems over adding more logic to `Game.cpp`.

## Visual quality rules
- Primitive rectangles/circles are acceptable only as editor/debug overlays or temporary fallbacks.
- Shipping UI and world art should use authored assets, nine-slice panels, sprites, particles, lighting and animation.
- Enemy silhouettes must be immediately distinguishable at gameplay zoom.
- Turret bases and rotating weapon heads should remain separate where appropriate.
- Effects must communicate damage type clearly without obscuring the battlefield.

## Text / localization
- Repository source files are UTF-8.
- German text must correctly render ä, ö, ü, Ä, Ö, Ü and ß.
- Do not replace umlauts with ae/oe/ue as a workaround.
- SFML text created from `std::string` must be converted from UTF-8 explicitly.

## MAP FORGE target
The integrated editor must eventually support terrain/biomes, paths, multiple spawns/goals, build and blocked zones, water, decorations, height information, wave sets, environment settings, validation, undo/redo, save/load and instant play-test.

## Build and verification
- C++23 is the required project standard; configure it centrally through `AEGIS_CXX_STANDARD`.
- Keep `-Wall -Wextra -Wpedantic` clean.
- Run core tests after changes to map/editor serialization.
- Never silently change `.aegismap` semantics; bump the map format version when compatibility breaks.

## Work style
For any change touching more than one subsystem, first update or create an ExecPlan in `docs/plans/` with goal, files, invariants, implementation steps and verification. Then implement it in bounded phases.
