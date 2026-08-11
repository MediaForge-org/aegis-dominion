# Phase 02 – Screen architecture and MAP FORGE vertical slice

## Goal

Replace the monolithic program-state flow with stack-based screens and deliver a usable MAP FORGE slice: document editing, camera/grid, selection, path/spawn/goal/zone tools, undo/redo, validation, map browsing, persistence and an unsaved-document playtest round trip.

## Files in scope

- Application and navigation: `src/app/`, `src/main.cpp`, `CMakeLists.txt`.
- Screens and shared UI: `src/screens/`, `src/Game.*`, focused helpers in `src/ui/`.
- Renderer-independent map/editor behavior: `src/core/MapDocument.*`, `src/editor/`.
- Current gameplay adapter: `src/Map.*` and the extracted gameplay screen.
- Regression suite: `tests/`.
- User and architecture documentation: `README.md`, `docs/ARCHITECTURE_V3.md`, `docs/MAP_FORMAT.md`, `docs/ROADMAP.md`, `docs/MAP_FORGE.md`.

## Invariants

- `src/core/` stays independent of SFML and renderer coordinates.
- Existing `AEGIS_MAP_V1` files keep their meaning and continue to load; no incompatible format change is introduced.
- `MapDocument` remains the only source of truth for editable map content.
- Existing menu, tutorial, map selection and tower-defense gameplay remain available.
- Playtest conversion copies editor data and never mutates the editor document or screen state.
- UTF-8 strings continue to enter SFML only through `TextService`.
- Debug and Release remain clean under `-Wall -Wextra -Wpedantic`.

## Implementation steps

1. Introduce `Application`, a queued stack-based `ScreenManager`, a screen interface and shared UI drawing services.
2. Extract the main-menu/map-select/tutorial flow and gameplay into `MainMenuScreen` and `GameScreen`; preserve existing behavior and add playtest return navigation.
3. Expand renderer-independent validation and `MapEditorModel` with stable-ID CRUD, selection-aware operations, revision-based dirty tracking and complete undo/redo behavior.
4. Add renderer-independent playtest conversion and adapt `GameMap` to consume an in-memory document copy.
5. Build MAP FORGE camera, grid, tools, selection/manipulation, inspector, validation/status UI, map browser, save flows and discard confirmation.
6. Add focused tests for documents, every required edit category, history branching, validation and immutable playtest conversion.
7. Update documentation, run Debug and Release builds/tests, inspect diffs and record remaining manual Fedora GUI checks.

## Verification

- Configure and build fresh Debug and Release trees.
- Run all CTest targets with failure output in both configurations.
- Confirm warning output is empty for the configured warning set.
- Verify all built-in V1 maps and save/load round trips.
- Verify history create/move/delete/branch operations and playtest conversion immutability automatically.
- On Fedora with X11, manually exercise every MAP FORGE tool, dialogs, camera controls, validation gating and editor → playtest → unchanged editor round trip.

## Progress

- [x] Repository and architecture baseline inspected; initial worktree clean.
- [x] Screen architecture and legacy screen extraction.
- [x] Renderer-independent editor/model additions.
- [x] MAP FORGE UI and interaction slice.
- [x] Automated tests and both build configurations.
- [x] Documentation and diff review.
- [ ] Phase commit (workspace `.git` is read-only; `index.lock` cannot be created).

## Result

Implemented on 2026-08-11. Fresh Debug and Release builds completed with GCC 16.1.1 and no diagnostics under `-Wall -Wextra -Wpedantic`. All four CTest targets passed in both configurations (28 named test cases total). `git diff --check` and the renderer-independence scan for `src/core/` passed. A graphical launch was attempted, but this environment has no X11 display; the executable correctly reached SFML's display initialization and manual Fedora GUI verification remains required. The requested phase commit could not be created because this managed workspace exposes `.git` read-only and rejects creation of `.git/index.lock`; no repository permissions were changed or bypassed.
