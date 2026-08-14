# MediaForge E2.1 – Data-driven settings

## Goal

Finish E2.1 by replacing the Graphics Quality-only indicator path with a small data-driven AEGIS settings model and a reusable MediaForge segmented-level UI primitive. Current runtime VSync, FPS-limit and quality behavior must remain immediate and authoritative.

## Files in scope

- `engine/mediaforge/include/mediaforge/ui/SegmentedLevelIndicator.hpp`
- `engine/mediaforge/src/ui/SegmentedLevelIndicator.cpp`
- `engine/mediaforge/CMakeLists.txt`
- `engine/mediaforge/tests/RenderTests.cpp`
- `src/mediaforge/MediaForgeAppModel.hpp`
- `src/mediaforge/MediaForgeAppModel.cpp`
- `src/mediaforge/MediaForgeMenu.cpp`
- `tests/e2_slice_tests.cpp`
- `docs/E2_1_INTERACTIVE_MENU.md`
- `engine/mediaforge/README.md`

## Invariants

- Dependency direction stays AEGIS DOMINION → MediaForge Engine → SDL3/SDL_GPU. MediaForge contains no AEGIS setting IDs, labels, enums or dependencies.
- `RuntimeSettings` remains the sole source of current setting values. Descriptors are derived views and hold no independent mutable selection state.
- Stable IDs and category metadata are consumer-owned and independent of localized display strings.
- Boolean, discrete/cyclic, continuous, key-binding and action types are distinguishable even though only current cyclic presentation is implemented in E2.1.
- Segment count comes from the setting's actual option list; active count is the bounded current index plus one.
- Mouse and keyboard activations continue through the same setting-action path.
- VSync changes present mode, FPS changes the frame pacer and quality changes the live render-target profile exactly as before.
- The generic change pulse is bounded to 125 ms. Static UI does not animate or rebuild while idle.
- No E2.2, E3, gameplay, MAP FORGE migration, map-selection implementation or unrelated content is included.

## Implementation steps

1. Add an engine-generic segmented indicator model/style/geometry renderer and focused standalone MediaForge tests.
2. Add AEGIS-owned setting IDs, categories, setting types, value metadata and derived descriptor lookup over `RuntimeSettings`.
3. Route all current setting rows, labels and indicators through the descriptors; replace the quality-only timer with a generic changed-setting pulse.
4. Add focused AEGIS tests for IDs, categories, types, mappings, derived state and wrap behavior.
5. Update E2.1 and MediaForge UI documentation.
6. Configure/build Debug and Release, run full CTest and standalone MediaForge tests, and run the architecture audit plus available smoke/showcase startup checks.

## Verification

- MediaForge tests cover zero/one/many segment geometry, current-index clamping, active/inactive state and visual pulse inputs.
- AEGIS tests cover VSync 1/2–2/2, FPS 1/5–5/5, quality 1/4–4/4, all last-to-first wraps, stable IDs, category metadata and descriptor derivation from changed runtime settings.
- Debug and Release compile as C++23 with `-Wall -Wextra -Wpedantic` and no project warnings.
- All configured CTests and the MediaForge architecture audit pass.

## Progress

- [x] Existing E2.1 settings, runtime side effects, interaction path and cache behavior audited.
- [x] Generic MediaForge segmented indicator implemented and tested.
- [x] AEGIS data-driven setting descriptors implemented and tested.
- [x] Menu rows and generic pulse migrated.
- [x] Documentation updated.
- [x] Debug/Release builds, tests, audit and offscreen Vulkan smoke/menu/showcase checks completed. Desktop visual/input acceptance remains a manual Fedora review.
