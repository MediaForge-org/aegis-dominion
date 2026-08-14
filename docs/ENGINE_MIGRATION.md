# Engine migration strategy

E1 and E2 are additive. `aegis_dominion` still executes its complete SFML application. E2.1 makes normal `aegis_mediaforge_slice` startup an AEGIS-owned interactive MediaForge main menu; the E2 Verdant snapshot/reference runtime remains available with `--e2-showcase` without replacing the playable SFML fallback.

Migration proceeds capability by capability. E2 proves generic 2D rendering, offscreen composition and a complete Verdant presentation snapshot. E2.1 adds generic canvas/button input routing and segmented-level geometry plus AEGIS-owned runtime text/menu composition, data-driven setting descriptors, screen transitions and live performance settings. E2.2 replaces the temporary play destination with real built-in/custom map discovery, selection, validation, preview, an owned `PlayableMap` launch and a deliberately limited selected-map gameplay session. E2.3 and later milestones connect live gameplay simulation and migrate MAP FORGE. SFML is removed only after equivalent behavior and tests exist.

To extract the Engine later:

1. Move `engine/mediaforge/` to a new repository root.
2. Configure that directory directly; its CMake creates its own project, language-standard option, dependency resolution, tests and smoke target.
3. Export/install `MediaForge::Foundation` and `MediaForge::Engine` in the new repository (installation packaging is intentionally deferred in E1).
4. Replace the root `add_subdirectory` with `find_package(MediaForge)` or an exact pinned `FetchContent` reference.
5. Keep AEGIS adapters/game assets in this repository and run the architecture audit in both projects.

Known E2.2 limits are deliberate: the MediaForge gameplay session presents the selected real map but does not start waves, enemies, towers or combat; MAP FORGE still opens a migration notice; the showcase still uses animated representative AEGIS snapshot data; SPIR-V is still the first shader artifact; and the complete ordinary game/MAP FORGE remain SFML. Visual acceptance is pending Fedora review. E2 is still open, and E2.3 has not started.
