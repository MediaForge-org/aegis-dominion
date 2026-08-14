# MediaForge E2.1 – Interactive start screen

## Goal

Replace the normal startup behavior of `aegis_mediaforge_slice` with the first real interactive AEGIS DOMINION flow: a MediaForge/SDL_GPU/Vulkan main menu, functional secondary screens, runtime performance settings and clean quit. Preserve the accepted Verdant E2 scene unchanged as an explicit `--e2-showcase` reference and do not migrate gameplay or MAP FORGE in this milestone.

## Files in scope

- Generic MediaForge UI input and layout contracts under `engine/mediaforge/include/mediaforge/ui/` and `engine/mediaforge/src/ui/`, plus their headless engine tests and build registration.
- AEGIS-owned MediaForge application state, menu presentation and runtime entry point under `src/mediaforge/`.
- Existing E2 run configuration and Verdant executable entry point only where needed to select menu versus showcase and apply live settings.
- AEGIS E2 UI text/texture assets and logical manifest entries under `assets/e2/`.
- Root build/test registration and E2.1 launch/migration documentation.

## Invariants

- Dependency direction remains AEGIS DOMINION → MediaForge Engine → SDL3/SDL_GPU. Engine UI contains only generic rectangles, canvas mapping, widget states and input routing; no AEGIS names, copy, screens or assets enter `engine/mediaforge/`.
- `src/core/`, gameplay simulation, map data and `.aegismap` semantics remain unchanged and renderer-agnostic.
- Normal `aegis_mediaforge_slice` startup enters the interactive menu. The Verdant scripted reference remains available only through `--e2-showcase` and retains its benchmark/stress flags.
- All visible AEGIS screen composition, German UTF-8 copy, logical asset IDs and screen transitions remain under `src/mediaforge/` and `assets/e2/`.
- The menu uses the existing authored E2 surfaces and art direction. Primitive fills are limited to restrained tint/accent layers and interaction feedback, not developer-style shipping panels.
- UI hit testing maps SDL logical window coordinates into the fixed 1600×900 UI canvas through aspect-preserving letterboxing, including resized and high-density windows.
- UI state supports normal, hover, pressed, disabled and keyboard focus preparation; pointer activation consumes input and preserves press capture semantics.
- VSync and FPS changes affect the running application immediately. Quality changes update the active MediaForge presentation configuration without building a larger persistent settings system.
- Static screen geometry is rebuilt only when the screen, setting or interaction state changes. VSync/FPS limiting and minimized/unfocused throttling remain active.
- C++23 and `-Wall -Wextra -Wpedantic` remain clean. Existing SFML application, MAP FORGE, E1 smoke and all E2 renderer behavior remain available.

## Implementation phases

1. Add and headless-test generic `UiCanvas`, `UiRect`, `UiContext`, `ButtonResult` and widget-state behavior in MediaForge.
2. Add an AEGIS-owned `MediaForgeAppModel` with bounded screen stack/back navigation, menu actions, settings values and quit request state; cover it with deterministic tests including UTF-8 copy.
3. Add a reusable game-owned E2 text atlas/presentation helper and compose Main Menu, Play placeholder, MAP FORGE placeholder, Anleitung and Einstellungen with existing authored E2 surfaces.
4. Split the executable entry behavior so normal startup runs the interactive application and `--e2-showcase` runs the preserved Verdant reference.
5. Wire SDL3 mouse/keyboard input, accurate canvas mapping, screen transitions, live present mode/FPS/quality updates, clean window close and cached static UI rendering.
6. Update launch/migration documentation and run Debug, Release, standalone engine, E1 smoke build, all CTests, warning scan and architecture audit.

## Verification

- Headless tests cover resized-canvas mapping, inclusive button hit testing, normal/hover/pressed/disabled/focused states, pointer capture/input consumption and mouse/keyboard activation.
- AEGIS tests cover every main-menu transition, back behavior, VSync/FPS/quality values, clean quit request and the exact UTF-8 German strings `ANLEITUNG`, `EINSTELLUNGEN`, `Zurück`, `Qualität`, `Überblick` and `Schließen`.
- `./aegis_mediaforge_slice` visibly starts at the main menu and supports the full E2.1 manual acceptance route.
- `./aegis_mediaforge_slice --e2-showcase` visibly starts the preserved Verdant reference and accepts existing E2 performance/stress options.
- Debug and Release root builds, standalone MediaForge build, `mediaforge_gpu_smoke` build, all CTests and `mediaforge_architecture_audit` pass without project warnings.

## Progress

- [x] Existing E2 executable, renderer/input APIs, performance configuration, assets, tests and dirty worktree audited.
- [x] Generic MediaForge UI foundation and tests.
- [x] AEGIS screen/application model and tests.
- [x] E2 menu presentation and interaction wiring.
- [x] Menu/showcase startup split.
- [x] Documentation.
- [x] Debug/Release/standalone/test/audit verification. Desktop visual/input acceptance remains pending because the managed environment cannot access the X11 video device; offscreen SDL_GPU/Vulkan startup and rendering passed.
