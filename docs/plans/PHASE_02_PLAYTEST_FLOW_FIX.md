# Phase 02 bug fix – Map Forge playtest identity

## Goal

Guarantee that MAP FORGE launches gameplay from the current in-memory document and that simulation and rendering consume the same projected `PlayableMap`, without any standard-map fallback.

## Files in scope

- Launch contract: `src/core/GameLaunchConfig.*`, `src/app/Screen.*`, `src/app/Application.cpp`.
- Playtest producer/consumer: `src/editor/MapForgeScreen.cpp`, `src/screens/GameScreen.*`.
- Gameplay map adapter/render data: `src/Map.*`.
- Conversion and regression tests: `src/core/PlayableMap.*`, `tests/playtest_conversion_tests.cpp`, `CMakeLists.txt`.
- Focused documentation: `docs/ARCHITECTURE_V3.md`, `docs/MAP_FORGE.md`.

## Root cause

The editor document reaches `GameScreen` and its path reaches simulation, but `GameMap::loadFromDocument()` assigns a built-in visual index from the biome. `GameScreen::drawWorld()` then always draws `assets.mapTexture(map_.index())`, a baked standard-map PNG containing its own terrain and road. Custom paths/zones are not rendered. The launch request also combines an optional document, map index and boolean playtest flag, which permits ambiguous construction and silent fallback.

## Invariants

- No file path is required for a MAP FORGE playtest.
- A playtest launch owns a complete renderer-independent `PlayableMap`; it cannot represent “playtest with no map”.
- Standard and MAP FORGE launches are distinct variant alternatives, not inferred from flags.
- One deterministic world-to-gameplay transform is applied to paths, spawn, goal and all zones.
- A valid playtest never calls the built-in map loader or chooses a standard visual index.
- The editor screen remains alive below the gameplay screen and its document is never mutated.

## Implementation steps

1. Add an SFML-free variant launch configuration and in-memory document-to-launch factory.
2. Pass that configuration through `ScreenRequest` and construct `GameScreen` through one unambiguous API.
3. Make `GameMap` consume `PlayableMap` directly and expose consistently projected path/endpoints/zones.
4. Render custom playtest terrain, path, endpoints and all zone types from that projected data; retain authored PNGs only for normal built-in games.
5. Add sentinel, unsaved-map, saved-map and no-fallback launch regression tests.
6. Run clean Debug/Release builds and all CTests outside the repository build tree.

## Progress

- [x] Root cause reproduced by tracing launch, map adapter and rendering.
- [x] Variant launch contract and direct in-memory conversion implemented.
- [x] Custom simulation/render data unified with deterministic projection.
- [x] Sentinel, unsaved, saved and no-fallback regression coverage added.
- [x] Final clean Debug/Release verification.

## Result

Completed on 2026-08-11. Fresh Debug and Release builds in separate `/tmp` trees completed with GCC 16.1.1 and no warnings under `-Wall -Wextra -Wpedantic`. All five CTest targets passed in both configurations. The regression suite now includes seven conversion/launch cases plus a dedicated GameMap projection/identity/mechanics target. GUI verification remains manual because the managed environment has no X11 display.

## Verification

- `-Wall -Wextra -Wpedantic` clean in Debug and Release.
- All CTest targets pass.
- Sentinel launch preserves ID/name/dimensions/path/spawn/goal/build/blocked/water zones.
- Unsaved and save/load documents produce equivalent playtest geometry.
- MAP FORGE launch has no built-in map-index alternative and cannot silently fall back.
