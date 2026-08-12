# AEGIS DOMINION roadmap

## Completed foundation (Phase 1)

- Product-facing AEGIS DOMINION identity.
- Renderer-independent V1 `MapDocument` and controlled format handling.
- Central SFML UTF-8/font service.
- Built-in data maps and foundation regression suite.

## Completed vertical slice (Phase 2)

- Stack-based `Application`/`ScreenManager` architecture.
- Extracted `MainMenuScreen`, `GameScreen` and `MapForgeScreen`.
- MAP FORGE camera, grid/snap, all required Phase 2 tools and contextual selection.
- Multi-path editing, multiple stable-ID spawns/goals and rectangular build/blocked/water zones.
- Complete create/move/delete undo/redo with branch invalidation and revision-based dirty tracking.
- Project map browser, new/open/save/save-as and unsaved-change protection.
- Object-aware error/warning validation and fatal playtest gating.
- Unsaved in-memory editor → gameplay → exact editor-state playtest round trip.
- Expanded document/editor/playtest/UTF-8 regression suite.

## Completed toolchain migration

- C++23 is the central project standard for all libraries, executables and tests.
- Shared `aegis_project_options` owns compile features and warning flags.
- GNU language extensions are disabled and unsupported requested standards fail configuration.
- A later C++26 toolchain experiment requires only `-DAEGIS_CXX_STANDARD=26`; C++26 is not yet an official project requirement.

## Completed presentation foundation (Phase 3)

- Typed logical texture/font/sound IDs, central catalog, one-owner cache and explicit missing-resource fallbacks.
- Shared DEBUG/INFO/WARNING/ERROR logger with `std::source_location` diagnostics.
- Rebindable action mappings with Menu, Gameplay, MapForge and Modal contexts plus pointer consumption/capture.
- Tactical theme tokens, stateful button logic, reusable component drawing, linear layout and reference-resolution scaling.
- Main menu, tutorial, map selection, gameplay command UI and central MAP FORGE chrome migrated to shared styling/components.
- Deterministic render-layer vocabulary, render context/queue, tween and sprite-animation playback, and parameter-driven effects.
- Settings-ready model for scale, audio, VSync, FPS limit and screen shake.

## Completed Phase-4 visual vertical slice

- Immutable map/enemy/tower/projectile render snapshots and a real fixed-pass `WorldRenderer`.
- One SFML-free Catmull-Rom curve shared by enemy movement, road visuals and build clearance.
- Eight material mappings; distinct Verdant/Frost/Ash surfaces with deterministic patches and seeded environment.
- Smooth layered road mesh, animated water, authored spawn gate/AEGIS Core, shadows, radial glow and world-only screen shake.
- Enemy state presentation and contextual health/shield UI; separate tower bases/heads with eased tracking, recoil, idle energy and differentiated projectile/VFX profiles.
- External asset and animation definition files plus authored nine-slice surfaces used by HUD, cards, buttons and editor chrome.
- MAP FORGE material/curve preview and gameplay/editor zone-visibility separation.

## MediaForge engine roadmap

- **E1:** Engine boundary, SDL3, SDL_GPU, GPU bootstrap, triangle and textured quad.
- **E2:** 2D renderer, sprite batching, camera, render targets and the first beautiful Verdant vertical slice.
- **E3:** Terrain, road, water, environment, lighting and post processing.
- **E4:** Enemies, towers, projectiles and VFX migration.
- **E5:** UI, text and MAP FORGE rendering migration.
- **E6:** Remove SFML after feature parity and regression verification.
- **E7:** 3D mesh renderer, perspective camera and depth.
- **E8:** Materials/PBR, lighting and shadows.
- **E9:** 3D terrain, heightmaps and environment.
- **E10:** 3D MAP FORGE.

The sequence may be adjusted when profiling or production evidence requires it, but dependency direction and data/renderer separation remain fixed.

## Immediate next milestone: MediaForge E2 – Beautiful Verdant Vertical Slice

E2 integrates high-quality authored Verdant terrain and road, environment, Spawn Gate, AEGIS Core, Raider, Heavy Tank, Pulse Tower, one special tower, lighting, VFX and HUD through the MediaForge GPU renderer. The user accepts E2 visually from a real screenshot/video; E1 does not claim that art milestone.
