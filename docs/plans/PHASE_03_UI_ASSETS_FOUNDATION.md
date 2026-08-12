# Phase 03 – UI, assets, input and rendering foundation

## Goal

Deliver an integrated technical vertical slice for AEGIS DOMINION's presentation layer: typed cached assets with explicit fallbacks, shared logging and input actions, themed/stateful UI with layout and scaling, deterministic render layers, and reusable animation/effect definitions. Migrate the existing main-menu pages, gameplay controls and central MAP FORGE chrome without changing simulation or map semantics.

## Files in scope

- Application services and composition: `src/app/`, `src/main.cpp`.
- Typed resources and compatibility facade: `src/assets/`, `src/Assets.*`.
- Input and logging: `src/input/`, `src/logging/`.
- UI/theme/layout/interaction: `src/ui/` and the migrated screens in `src/screens/` and `src/editor/MapForgeScreen.*`.
- Rendering/animation/effects foundations: `src/render/`, `src/animation/`, focused integration in `src/Effects.*` and screens.
- Settings model: `src/settings/`.
- Build and regression coverage: `CMakeLists.txt`, `tests/`.
- Contributor/user documentation: `README.md`, `AGENTS.md`, `docs/ARCHITECTURE_V3.md`, `docs/ROADMAP.md`, and new subsystem documentation.

## Invariants

- `src/core/` remains free of SFML and any dependency on UI, assets, input or rendering.
- `AEGIS_MAP_V1`, `MapDocument`, editor history, validation and playtest conversion semantics do not change.
- Gameplay simulation never depends on UI widgets; compatibility APIs do not reload assets per frame.
- UTF-8 strings continue through `TextService`, including German umlauts and ß; icons remain independent of the text font.
- Existing authored assets remain usable by towers, enemies, maps and gameplay while all file loading is centralized.
- Modal UI consumes input before world/editor actions, and one active input context prevents cross-screen action leakage.
- The reference UI coordinate system remains 1600×900 while the window/view scales and letterboxes consistently.
- Warning flags remain clean and C++23 remains centrally configured through `AEGIS_CXX_STANDARD`.

## Implementation steps

1. Add logger, typed asset IDs/catalog/manager, checkerboard texture fallback and font/sound fallbacks; retain `Assets` as a narrow domain facade.
2. Add action bindings/contexts, pointer capture/consumption and settings-ready bindings; route Application events and migrate menu/game/editor actions.
3. Add theme tokens, widget interaction state, layout calculations, UI scaling, lightweight transitions and reusable component drawing; migrate visible menu/tutorial/map-select and MAP FORGE chrome.
4. Add render context/layer ordering plus sprite animation and parameter-driven effect definitions; integrate fixed world/UI/modal passes without coupling core data to SFML.
5. Add focused tests for caching/fallbacks/IDs, input contexts/conflicts, UI state/layout/icons/theme, logging levels, animation and render-layer order.
6. Update architecture and subsystem documentation, perform fresh Debug and Release builds/tests, audit warnings/direct loads/direct keys and report manual Fedora GUI checks.

## Verification

- Every resource `loadFromFile` call is confined to the central asset manager.
- Existing five CTest targets stay green and new subsystem tests cover meaningful behavior.
- Fresh Debug and Release configurations build under `-Wall -Wextra -Wpedantic` with no diagnostics.
- `src/core/` contains no SFML or new presentation dependency; `.aegismap` fixtures remain unchanged.
- Source audits confirm action use in migrated screens, typed IDs in the resource facade and no font-based reserved icons.
- Manual Fedora checks cover menu/tutorial/map selection, normal game, MAP FORGE/playtest return, modal capture, scaling, hover/press/tooltips and UTF-8.

## Progress

- [x] Repository, documentation, prior plans and current architecture audited; branch/worktree verified.
- [x] Shared services and typed assets.
- [x] Input/actions and migrated controls.
- [x] UI/theme/layout/scaling and visible migration.
- [x] Rendering/animation/effect foundations.
- [x] Tests, documentation and Debug/Release verification.
- [ ] Phase commit (managed workspace exposes `.git` read-only; commit attempt recorded below).

## Result (2026-08-12)

Phase 3 delivered the integrated foundation and migrated real runtime paths. The six CTest targets pass in fresh Debug and Release trees; the suite reports 54 meaningful checks (44 retained plus 10 new foundation cases). GCC 16.1.1 compiled 38 translation units as C++23 under `-Wall -Wextra -Wpedantic` without warnings. `git diff --check`, the `src/core` renderer-dependency scan and resource-loading/runtime-console audits passed. GUI verification remains manual because this environment has no X11 display. The requested commit could not be created: the managed workspace mounts `.git` read-only and rejected `.git/index.lock`; no permission bypass was attempted.
