# Phase 01 – AEGIS DOMINION foundation

## Goal

Complete the first bounded foundation phase: adopt the AEGIS DOMINION product name, make every current SFML text path explicitly UTF-8 safe, and turn the existing map/editor smoke test into a useful regression suite without removing playable prototype behavior.

## Files in scope

- Product/build surface: `CMakeLists.txt`, `build.sh`, `run.sh`, visible strings in `src/`, map metadata, asset generator and generated logo.
- Text infrastructure: focused files below `src/ui/`, `Assets.*`, `Game.*`, `Effects.*`.
- Foundation tests: `tests/`, map serialization/parser behavior where a test exposes a defect.
- Documentation: `AGENTS.md`, `README.md`, `ANLEITUNG.md`, `CHANGELOG.md`, `docs/`.

## Invariants

- `src/core/` stays renderer-agnostic and contains no SFML types.
- `.aegismap` remains `AEGIS_MAP_V1`; no compatibility-breaking semantic change is made.
- Existing maps, towers, enemies, waves, tutorial and gameplay stay available.
- All `std::string` text passed to SFML is converted explicitly from UTF-8.
- The configured C++17 warning set remains clean.
- No Map Forge UI, asset-pipeline rewrite, or combat redesign is claimed in this phase.

## Implementation steps

1. Rename product-facing build targets, launch scripts, window/menu strings, metadata and documentation.
2. Add a central text service that owns font discovery/loading and UTF-8 conversion/text construction.
3. Route game UI and floating effect text through the service.
4. Expand foundation tests for UTF-8 data, save/load round trips, malformed input, unknown versions, validation and editor undo/redo.
5. Build and test Debug and Release configurations, inspect warning output, and run a headless launch smoke test where supported.
6. Update architecture and phase documentation to match the delivered implementation.

## Verification

- Configure/build with CMake and `-Wall -Wextra -Wpedantic` in Debug and Release.
- Run all tests through CTest with failure output enabled.
- Verify the generated logo dimensions/content and confirm no old product names remain outside historical context.
- Launch the executable under a virtual display when available; otherwise document the manual visual checks.

## Result

Completed on 2026-08-11. Clean Debug and Release builds completed with GCC 16.1.1 and the configured warning flags. Both CTest targets passed in both configurations. The automated graphical launch smoke test could not run because no usable X11 display or `xvfb-run` was available; visual menu/gameplay checks remain manual.
