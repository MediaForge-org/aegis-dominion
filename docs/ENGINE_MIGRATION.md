# Engine migration strategy

E1 is additive. `aegis_dominion` still executes its complete SFML application while linking the new MediaForge target to establish the permitted dependency direction. No game renderer or asset was removed.

Migration proceeds capability by capability: E2 introduces a generic 2D renderer and proves one Verdant slice; later milestones migrate terrain/environment, entities/VFX, UI/text and MAP FORGE. SFML is removed only in E6 after equivalent behavior and tests exist. Map documents and simulation data remain independent throughout.

To extract the Engine later:

1. Move `engine/mediaforge/` to a new repository root.
2. Configure that directory directly; its CMake creates its own project, language-standard option, dependency resolution, tests and smoke target.
3. Export/install `MediaForge::Foundation` and `MediaForge::Engine` in the new repository (installation packaging is intentionally deferred in E1).
4. Replace the root `add_subdirectory` with `find_package(MediaForge)` or an exact pinned `FetchContent` reference.
5. Keep AEGIS adapters/game assets in this repository and run the architecture audit in both projects.

Known E1 limits are deliberate: no batched sprite renderer/camera/render targets, no font/audio/animation implementation, no portable shader artifacts beyond SPIR-V, no game presentation migration and no visual-quality claim.
