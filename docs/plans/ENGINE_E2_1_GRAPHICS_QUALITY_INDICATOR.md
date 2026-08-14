# MediaForge E2.1 – Graphics quality indicator polish

> Historical plan: the quality-only implementation described here was generalized by
> `ENGINE_E2_1_DATA_DRIVEN_SETTINGS.md`. Quality, FPS Limit and VSync now share the
> engine-generic segmented indicator and AEGIS descriptor path.

## Goal

Add a compact four-segment level indicator to the existing MediaForge graphics-quality selector while preserving the Low → Medium → High → Ultra → Low runtime cycle and idle UI caching.

## Files in scope

- `src/mediaforge/MediaForgeAppModel.hpp` for the deterministic quality-to-segment mapping.
- `src/mediaforge/MediaForgeMenu.cpp` for MediaForge geometry rendering and the bounded change animation.
- `tests/e2_slice_tests.cpp` for focused segment-count and wrap coverage.

## Invariants

- The actual `GraphicsQuality` value is the sole source of the active segment count; no duplicate quality state is introduced.
- Mouse and keyboard activations continue through the same application command and update runtime quality exactly as before.
- The indicator uses MediaForge rectangle geometry, restrained cyan/teal active segments and subdued but visible inactive segments; it uses no font glyphs.
- The change pulse lasts 125 ms and only rebuilds the overlay during the animation plus its final settled frame. Idle world and overlay caching remain intact.
- No E2.2, gameplay, map, SFML, engine-boundary or asset changes are included.

## Implementation steps

1. Add a constexpr quality-to-segment helper beside the E2.1 application model.
2. Draw four framed segments beneath the quality label and apply a short data-driven active-segment pulse after changes.
3. Extend the E2 slice test with all four mappings and an explicit Ultra → Low activation assertion.
4. Build Debug and Release and run the existing CTest suites.

## Verification

- Focused E2 tests assert Low=1, Medium=2, High=3, Ultra=4 and Ultra → Low restores one active segment.
- Debug and Release builds complete with the project warning flags.
- Existing Debug and Release CTest suites pass.

## Progress

- [x] Existing selector, unified activation path and cached overlay flow audited.
- [x] Deterministic mapping and indicator presentation implemented.
- [x] Focused tests added.
- [x] Debug/Release builds and tests passed. Offscreen SDL_GPU/Vulkan menu startup also passed; desktop visual review remains a manual acceptance step.
