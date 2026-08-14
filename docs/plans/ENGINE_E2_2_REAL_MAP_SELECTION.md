# E2.2 real map selection and launch handoff

## Goal

Replace the normal MediaForge `SPIELEN` placeholder with an AEGIS-owned, interactive map library backed by the real `.aegismap` sources. A valid selection must create the existing AEGIS launch contract, enter a limited E2.2 gameplay session that owns the selected `PlayableMap`, render that map's actual identity and geometry, and return without restarting. The explicit `--e2-showcase` path remains separate. E2.3 gameplay migration is out of scope.

## Files and bounded phases

1. Map discovery and launch contract
   - Add `src/core/MapCatalog.*` for ordered built-in discovery plus safe custom `.aegismap` enumeration.
   - Generalize `src/core/GameLaunchConfig.*` so a standard launch owns a validated `PlayableMap` instead of a lossy integer index, and add a small renderer-independent `GameSession` owner.
   - Adapt the transitional SFML `GameScreen`, `GameMap`, and main-menu launch call site without removing existing behavior.
2. AEGIS MediaForge model and presentation
   - Extend `src/mediaforge/MediaForgeAppModel.*` with map-selection, scrolling, validity, preview identity, launch, gameplay, and back-navigation state.
   - Replace the placeholder composition in `src/mediaforge/MediaForgeMenu.cpp` with real map cards, metadata, authored built-in previews plus geometry overlays, and a selected-map gameplay presentation.
   - Register the existing authored map images in `assets/e2/manifest.mfassets`; no new fake map artwork or engine-owned AEGIS types.
3. Regression coverage and documentation
   - Extend focused core/E2 tests for discovery, invalid entries, stable IDs, selection, scroll visibility, metadata/preview changes, launch gating, sentinel-map separation, full geometry handoff, and navigation.
   - Update E2 migration/acceptance documentation and CMake source/copy wiring.

## Invariants

- `MapDocument` remains the authoritative serialized model; `PlayableMap` is derived only through `buildPlayableMap()`.
- Built-in maps are `verdant.aegismap`, `frost.aegismap`, and `ember.aegismap`, in their existing order. Other `.aegismap` files in the same directory are custom/MAP FORGE maps, never fabricated entries.
- Parse errors, fatal validation errors, and duplicate stable map IDs are visible catalog failures and cannot create a launch.
- `StandardGameLaunch` and `MapForgePlaytestLaunch` each own their exact `PlayableMap`; there is no index-zero, Verdant, previous-selection, or showcase fallback in the launch/session path.
- `src/core/` stays renderer-agnostic. SDL/GPU types remain behind MediaForge. No AEGIS vocabulary or includes enter `engine/mediaforge/`.
- Existing settings descriptors, immediate runtime application, UI caching, persistent targets, throttling, profiling, UTF-8, tutorial, quit, SFML game, MAP FORGE, and `--e2-showcase` behavior remain intact.
- E2.2 gameplay is deliberately limited to selected-map session presentation and back navigation. Waves, enemies, towers, combat, full HUD, editor migration, E3, and 3D are not implemented.

## Implementation steps

1. Implement deterministic catalog discovery and owned standard launches; convert transitional callers and add core tests.
2. Implement model-level selection/scroll/start/session behavior with no implicit selection and deterministic back history.
3. Compose real cards and cached previews from catalog data, then compose the gameplay screen from the session's real map geometry and authored built-in background where available.
4. Add interaction wiring for mouse cards/buttons, arrows/tab/enter, wheel scrolling, Escape, and selection visibility.
5. Update documentation and run the full verification matrix.

## Verification

- Configure and build Debug and Release with C++23 and the repository warning flags.
- Run all CTests in both builds.
- Run `aegis_e2_slice_tests`, `mediaforge_foundation_tests`, `mediaforge_render_tests`, and `mediaforge_architecture_audit` explicitly.
- Confirm the E1 GPU smoke target and the explicit `--e2-showcase` executable path still build/run as far as the available display permits.
- Inspect compiler output for `-Wall -Wextra -Wpedantic` warnings and report any environment-only limitations.
- Perform the documented Fedora manual sequence: normal launch, map A/B selection and preview change, keyboard navigation, back/re-entry, START, selected gameplay identity/geometry, and return without process restart.

## Result

- Debug full build: passed in `/tmp/aegis-recovery-debug` with C++23 and no AEGIS/MediaForge `-Wall -Wextra -Wpedantic` warnings.
- Release full build: passed in `/tmp/aegis-recovery-release` with the same warning policy.
- Debug and Release CTest: 13/13 passed in each configuration, including R1 tower balance coverage, the E2.2 sentinel/catalog flow and architecture audit.
- Standalone `engine/mediaforge/` Debug build: passed; 3/3 standalone tests passed (foundation, render, architecture).
- Normal E2.2 startup, explicit `--e2-showcase`, and E1 GPU smoke reached the Vulkan backend with SDL's offscreen video driver. The showcase retained 4,194,304 frame-resource bytes and zero frame-storage growth. Desktop visual/input acceptance still requires the documented Fedora run.
- The existing workspace build directory was not deleted or reset when its fetched-dependency generator cache conflicted. Fresh isolated build directories were used instead.
