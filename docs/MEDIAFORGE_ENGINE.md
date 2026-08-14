# MediaForge Engine

MediaForge Engine is the reusable game-engine layer developed initially inside AEGIS DOMINION. Its version is independent (`0.2.0-dev`) and its directory is intentionally shaped so it can become `MediaForge-org/mediaforge-engine` later.

## E1 modules

- `foundation`: `std::expected` result/error paths, assertions, time aliases, typed handles, scope-safe cleanup, engine configuration, version and a small replaceable log sink.
- `math`: `Vec2/3/4`, column-major `Mat4`, `Transform2D/3D`, degree/radian conversion, clamp and lerp without renderer types.
- `platform`: move-only `Window` with RAII destruction, resize/title/fullscreen operations, logical/drawable sizes and high-DPI creation.
- `input`: generic event variants, SDL event translation and frame-based keyboard/mouse pressed/down/released state.
- `render`: SDL_GPU device/window claim, actual backend query, static uploads and move-only buffers, textures, samplers, shaders and pipelines.

## E2 renderer

- `Camera2D` owns a generic center, orthographic extent, zoom and viewport. World/screen transforms and aspect-preserving letterboxing are headless-tested.
- `Renderer2D` accepts textured sprites or generic triangle-list geometry with transform, pivot, UV, tint, opacity, opaque/alpha/additive blend, layer/order and world/screen coordinates.
- Submission is stable by layer/order. Opaque/additive work groups by sampler/texture/blend; alpha work preserves depth order and merges adjacent compatible items. Each batch becomes one contiguous dynamic vertex range and one draw.
- Two persistent frame resources provide reusable GPU/upload buffers. Shaders, pipelines, samplers, textures and targets are created outside the frame loop.
- `RenderTarget` is an RAII linear floating-point sampling/color target. Resize retains the resource when unchanged and replaces it only when dimensions change.
- The composite samples world, emissive and UI targets. Authored sRGB is linearized in the sprite shader; restrained bloom, grade, exposure, saturation, vignette and tone mapping produce SDR sRGB output.
- `ParticleSystem2D` provides bounded position/velocity/acceleration/lifetime/size/rotation/color/UV/drag data with burst/trail/stream vocabulary. Statistics expose sprites, geometry, particles, batches, draws, triangles and CPU render time.

Public headers expose no SDL objects. `src/mediaforge/` stays outside the Engine and translates AEGIS snapshots/asset metadata into generic submissions.

Public headers do not expose SDL declarations. E1's private smoke program records one command buffer, acquires the swapchain, clears it, draws a vertex-buffer triangle, binds a sampled checker texture and draws a six-vertex quad, then submits. Resources and uploads are created once before the loop.

## Backend policy

`automatic`, `preferVulkan` and `requireVulkan` are configuration values. Fedora smoke runs request Vulkan; `preferVulkan` logs the creation error before asking SDL to select another supported backend, while `requireVulkan` fails. The reported name always comes from `SDL_GetGPUDeviceDriver`. E1 compiles SPIR-V and therefore provides the Fedora/Vulkan artifact first; future shader outputs add DXIL and metallib without spreading backend branches through the renderer.

## Boundaries

The Engine has no tower, enemy, wave, economy, biome, MAP FORGE or game-map knowledge. Game code decides which generic resource and transform to submit. AEGIS assets remain under the repository `assets/`; E1's checker pixels are a generic in-memory smoke asset.

See `DEPENDENCIES.md`, `ENGINE_MIGRATION.md`, `E2_VISUAL_ACCEPTANCE.md` and `engine/mediaforge/README.md` for build, review and extraction details.
