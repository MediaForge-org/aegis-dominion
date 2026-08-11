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

## Recommended next bounded phase

Build the reusable UI/input and typed asset-definition foundation before expanding content: extract map selection/tutorial into dedicated screens, introduce reusable forms/widgets and input actions, and establish typed asset IDs/fallbacks/logging. This reduces risk for later settings, codex, arsenal and professional presentation work.

## Later phases

1. Extend the map format compatibly for terrain materials, elevation/height metadata, environment editing, decorations and wave sets.
2. Separate gameplay simulation from SFML render views; then make combat, waves, difficulty and economy data-driven.
3. Replace prototype visuals through a deliberate authored asset, animation, particle and lighting pipeline.
4. Add enemy/tower breadth, bosses, progression, codex/arsenal, campaigns, statistics and savegames.
5. Profile, pool high-volume objects and complete accessibility/polish before broad release content.
