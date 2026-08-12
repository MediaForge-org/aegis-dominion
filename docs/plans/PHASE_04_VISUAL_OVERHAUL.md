# Phase 04 – visual overhaul vertical slice

## Goal

Deliver a visibly higher-quality world and HUD for built-in and MAP FORGE playtests: deterministic material terrain, smooth themed roads, animated water, authored spawn/core/environment visuals, snapshot-driven entities, shadows/glow/VFX and scalable authored UI panels. The running game—not only its architecture—must show the improvement.

## Files in scope

- Renderer-facing extraction and world systems: `src/render/`, focused adapters in `Map.*`, `Enemy.*`, `Tower.*`, `Projectile.*`, `Effects.*`, `screens/GameScreen.*`.
- Renderer-independent shared curve/data: `src/core/PathCurve.*`, `PlayableMap.*`; existing V1 map semantics remain compatible.
- Assets and definitions: `src/assets/`, `src/animation/`, `assets/manifest.aegis`, `assets/animations.aegis`, generated runtime art below `assets/terrain/`, `assets/world/`, `assets/environment/` and `assets/ui/`.
- Scalable UI surface: `src/ui/UiRenderer.*` and gameplay HUD integration.
- Tests/build: `tests/phase4_render_tests.cpp`, `CMakeLists.txt`.
- Documentation requested by Phase 4.

## Invariants

- `src/core/` remains free of SFML, OpenGL and engine-specific types.
- `MapDocument` stays the source of truth; `AEGIS_MAP_V1` syntax and meaning remain compatible.
- Simulation and rendered roads consume the same sampled curve, so enemies do not leave the road.
- Build/blocked zones stay mechanically active but are hidden in ordinary gameplay; only build-mode validity and water are visible.
- Built-in and custom maps share one world-render submission path; authored backgrounds are an optional terrain input, not a separate entity renderer.
- Resource file loads remain exclusive to `AssetManager`; manifest IDs are stable and missing resources retain explicit fallbacks.
- Fixed render layers remain authoritative and modal UI remains last.
- No combat balance or wave behavior is removed or changed.

## Implementation phases

1. Add SFML-free Catmull-Rom path sampling and propagate seed/environment/decorations through `PlayableMap` and `GameMap`.
2. Add immutable render snapshots and extraction from simulation entities; replace entity-owned drawing in gameplay with `WorldRenderer` submissions.
3. Implement layered terrain, smooth shoulder/base/marking roads, animated water, deterministic decals/decorations, spawn gate, AEGIS Core, shadows, health/shield rules and projectile visuals.
4. Load typed assets from an external lightweight manifest and animation clips from an external definition file; add authored runtime art and a nine-slice UI surface.
5. Integrate build-mode zone feedback, HUD polish and upgraded effect presentation.
6. Add regression tests for curves, snapshots/layers, deterministic decorations, materials, zone visibility, manifest/animation parsing and isolation.
7. Update architecture/art/rendering documentation; run fresh Debug/Release builds, all CTests, warning/diff/dependency audits and record Fedora visual checks.

## Verification

- Fresh Debug and Release configure/build with C++23 and `-Wall -Wextra -Wpedantic`.
- All prior 6 CTest targets plus the Phase-4 target pass; report named check count.
- `src/core/` dependency scan contains no SFML/render/UI/assets includes.
- Asset loading audit finds runtime file loads only in `AssetManager` (map/document I/O excluded).
- Repeated generation with identical map seed yields identical decoration records; different seeds yield a different layout.
- Curve endpoints are exact and distance sampling is continuous enough for simulation/render agreement.
- Manual Fedora matrix covers all requested screens/maps/towers/enemies/effects and 1280×720, 1600×900, 1920×1080.

## Progress

- [x] Repository status, required documentation, phase plans, render path, entity draw path, map projection, assets and tests audited.
- [x] Shared curve and map presentation data.
- [x] Snapshot extraction and WorldRenderer.
- [x] Runtime assets, external manifests and nine-slice UI.
- [x] Tests and documentation.
- [x] Debug/Release verification.
- [ ] Phase commit (managed workspace exposes `.git` read-only).

## Result (2026-08-12)

The vertical slice routes built-in and custom playtests through one snapshot-driven WorldRenderer. Custom maps now receive distinct material terrain, a smooth shared simulation/render road, animated water, deterministic environment, an animated deployment gate and AEGIS Core; gameplay debug rectangles are hidden. Entities gain consistent shadows, state animation, contextual bars, eased tower tracking, differentiated projectiles/VFX, additive lighting and world-only shake. Authored nine-slice art is active across shared UI cards/buttons/editor chrome.

Fresh GCC 16.1.1 Debug and Release builds completed as C++23 without diagnostics under `-Wall -Wextra -Wpedantic`. All seven CTest targets passed in both configurations and the executables report 64 domain checks. `git diff --check`, renderer-independence/resource-loading/entity-draw audits passed. GUI launch remains unavailable because the managed environment has no reachable X11 server or `xvfb-run`; `docs/PHASE_04_MANUAL_TESTS.md` contains the Fedora matrix.
