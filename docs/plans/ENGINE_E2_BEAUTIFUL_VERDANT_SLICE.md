# MediaForge E2 – Beautiful Verdant vertical slice

## Goal

Deliver a directly launchable AEGIS Verdant reference battle rendered through MediaForge, SDL3 and SDL_GPU. E2 adds a reusable generic 2D/2.5D renderer and proves it with materially richer authored world art, layered roads and terrain, readable combat silhouettes, grounded shadows, restrained lighting/post effects, particles/decals and a command HUD. The existing SFML game and E1 GPU smoke remain available for side-by-side comparison and regression testing.

Technical completion and artistic acceptance are separate: this plan may close technical work after clean builds/tests and a functioning slice, while visual acceptance remains pending the user's Fedora screenshot/video review.

E2 is not complete at a visually acceptable screenshot. It also includes the playable integration and measured resource-efficiency work described below. The target is the accepted MediaForge appearance with efficiency approaching the transitional SFML renderer at matched settings. E3 must not begin until the playable path and this performance acceptance are complete.

## Files in scope

- Engine API and implementation: `engine/mediaforge/include/mediaforge/render/`, `engine/mediaforge/src/render/`, `engine/mediaforge/shaders/`, `engine/mediaforge/tests/`, `engine/mediaforge/CMakeLists.txt`.
- AEGIS MediaForge adapter/reference scene: `src/mediaforge/`, root `CMakeLists.txt`.
- Replaceable E2 art and metadata: `assets/e2/`, `assets/e2/manifest.mfassets`.
- Existing renderer-agnostic snapshot contract under `src/render/` only where the adapter needs generic translation helpers; no simulation rewrite.
- Required repository/engine/asset/render/migration/roadmap documentation and `docs/E2_VISUAL_ACCEPTANCE.md`.
- Reusable frame pacing, profiling, culling, persistent submission storage and resource-memory accounting in MediaForge.
- AEGIS-owned live gameplay/presentation adapter and a diagnostic stress configuration which consumes the same renderer without changing game balance.
- `docs/E2_PERFORMANCE.md` containing measured local evidence, exact Fedora benchmark commands and explicitly pending hardware fields where this environment cannot produce them.

## Invariants

- Dependency direction remains AEGIS → MediaForge → SDL3/SDL_GPU. Nothing under `engine/mediaforge/` may contain AEGIS concepts, includes, targets or assets.
- Engine public headers expose no SDL types. GPU/native objects and command encoding remain private implementation details with RAII ownership.
- `src/core/`, gameplay simulation and `MapDocument` remain renderer-agnostic; `.aegismap` semantics do not change.
- The SFML executable and E1 triangle/checker smoke remain supported and unchanged in behavior.
- Runtime assets are addressed through logical metadata, not hard-coded renderer paths. E2 authored source art is distinct from debug/procedural fallback content.
- Shaders, pipelines, textures and render targets are persistent; sprite/particle submission uses bounded reusable per-frame uploads and batching rather than resource creation per item.
- Steady-state frame paths perform no avoidable GPU-resource creation and reuse hot CPU containers. Profiling separates CPU work from swapchain/present wait.
- Culling occurs before expensive vertex/submission construction. Static and dynamic presentation data have separate lifetimes and dirty invalidation.
- Default execution is power bounded; VSync, an FPS limit and an explicitly uncapped profiling mode are configurable.
- World presentation uses explicit ordered passes, with modal/screen UI outside world post-processing.
- Color handling is explicit: sampled color art is treated as sRGB-authored, lighting/composition operates in linearized shader space, and the final output is encoded for SDR presentation.
- C++23 and `-Wall -Wextra -Wpedantic` remain central and clean.

## Implementation phases

1. Add and test generic camera, render submission, sorting/batch construction, layer/pass vocabulary, render-target descriptions, statistics, particle and decal data contracts.
2. Extend private SDL_GPU resource support for renderable textures, dynamic frame uploads, alpha/additive pipelines, viewport/scissor state and offscreen pass encoding.
3. Add reusable world/emissive/composite flow with radial lights, soft projected shadow sprites, restrained bloom, vignette and tone/color adjustment.
4. Integrate high-resolution E2 authored assets and a manifest carrying logical ID, path, dimensions, filtering/wrapping, pivot, scale, material and optional map/animation references.
5. Build the AEGIS-owned Verdant reference snapshot/translation and `aegis_mediaforge_slice` runtime: layered terrain/road/details, curated environment, landmarks, Raider/Heavy Tank, Pulse/Railgun, projectiles, particles/decals and HUD.
6. Preserve E1 smoke; add headless engine tests for camera transforms, sorting/batching, layer order, render-target validation, particle evolution and lifecycle/configuration logic, plus AEGIS translation/determinism coverage.
7. Update required docs with exact Fedora launch/side-by-side commands, stats, color-space policy, asset honesty and manual visual acceptance checklist.
8. Run clean Debug and Release builds, standalone Engine build, all CTests, architecture audit and warning scan. Record observed headless limits and leave visual acceptance explicitly pending user review.
9. Establish a reproducible measurement protocol for the SFML executable, the pre-optimization E2 reference and the eventual playable MediaForge path. Never substitute present/VSync wait for CPU rendering time or report unavailable desktop measurements.
10. Add renderer phase timings (sprite submission, batch construction, command encoding/submission and swapchain wait), frame/resource counters, asset-load duration, texture/render-target memory estimates and process RSS reporting where supported.
11. Remove avoidable steady-state allocations; reuse pass, item, ordering, batch and event storage; retain all GPU objects across frames; add tests/statistics that make regressions visible.
12. Add conservative viewport culling, static world submission caching/dirty invalidation and pooled/batched particle stress coverage. Preserve alpha ordering and optimize batching only where profiling supports it.
13. Audit target formats and bloom bandwidth. Keep HDR precision for world/emissive data where it affects the accepted look, use a cheaper UI target, and run emissive/bloom at a quality-controlled reduced resolution only when the default appearance remains essentially equivalent.
14. Add settings/configuration for VSync, capped and uncapped profiling operation, graphics quality preparation, minimized/unfocused throttling, and real-renderer stress loads at 100/500/1000 enemies, 1000 environment sprites and 100/1000 particles.
15. Replace the scripted snapshot producer with the real AEGIS simulation feeding `RenderSnapshot` and an AEGIS presentation adapter. There must be one gameplay simulation, not a parallel scripted simulation.
16. Re-run Release comparisons on identical hardware/resolution/scene settings, capture the required before/after counters, and complete the 24-item final report. User Fedora/Core Ultra 7 256V acceptance remains pending until those commands are run there.

## Verification

- `aegis_mediaforge_slice` launches independently, logs the actual SDL_GPU backend and keeps a responsive 1280×720, 1600×900 and 1920×1080 composition.
- The scene visibly contains all eight ordered E2 stages: world base, details, entities, emissive/effects, lighting, post, UI and final composite.
- Renderer statistics report frame time/FPS, submitted sprites, particles, batches, draw calls and triangles; compatible decoration/particle submissions batch together.
- Camera world/screen round trips and aspect-preserving viewport calculations pass headless tests.
- Batch tests prove stable layer order and state grouping; target tests reject invalid sizes and preserve allocation across unchanged frames.
- The engine architecture audit finds no AEGIS/SFML/game concepts or links below `engine/mediaforge/`.
- `mediaforge_gpu_smoke` still builds with the existing triangle/checker path.
- Fresh Debug and Release root builds and standalone Engine builds complete without project warnings; every CTest passes.
- Fedora visual review captures the old SFML scene and new MediaForge slice at the same reference resolutions. Final status is worded exactly: **visual acceptance pending user review**.
- Release benchmark mode supports deterministic duration/warmup/output, disables VSync/frame caps only when explicitly requested, and reports CPU work separately from wait/synchronization.
- The diagnostic scene reports representative 100/500/1000 enemy and 1000-particle results without altering live gameplay balance.
- A post-optimization screenshot/video is compared with the accepted E2 reference; noticeable regressions are reverted or exposed only as optional lower-quality modes.

## Progress

- [x] Branch/worktree, required documentation, E1 plan, current SFML snapshot renderer, Engine GPU foundation, assets and tests audited.
- [x] Generic renderer contracts and headless tests.
- [x] SDL_GPU batched/offscreen renderer and shaders.
- [x] Authored Verdant E2 art and metadata.
- [x] AEGIS reference slice and presentation translation.
- [x] Documentation and visual acceptance guide.
- [x] Debug/Release/standalone/test/audit verification.
- [ ] Comparable SFML / E2 reference / playable MediaForge baseline protocol and measurements.
- [x] Split phase instrumentation and resource-memory/load accounting.
- [x] Persistent hot-path storage, allocation regression visibility and zero steady-state GPU creation.
- [ ] Culling, static presentation caching and measured batching improvements.
- [ ] Target-format/bloom audit with equivalent default visuals.
- [x] VSync/FPS-cap/uncapped profile settings and background power throttling.
- [ ] Renderer stress mode and particle scaling evidence.
- [ ] Real AEGIS simulation → RenderSnapshot → MediaForge playable integration.
- [ ] Final Fedora benchmark and visual-regression acceptance report.
