# MediaForge Engine

MediaForge Engine is the reusable game-engine layer developed initially inside AEGIS DOMINION. Its version is independent (`0.1.0-dev`) and its directory is intentionally shaped so it can become `MediaForge-org/mediaforge-engine` later.

## E1 modules

- `foundation`: `std::expected` result/error paths, assertions, time aliases, typed handles, scope-safe cleanup, engine configuration, version and a small replaceable log sink.
- `math`: `Vec2/3/4`, column-major `Mat4`, `Transform2D/3D`, degree/radian conversion, clamp and lerp without renderer types.
- `platform`: move-only `Window` with RAII destruction, resize/title/fullscreen operations, logical/drawable sizes and high-DPI creation.
- `input`: generic event variants, SDL event translation and frame-based keyboard/mouse pressed/down/released state.
- `render`: SDL_GPU device/window claim, actual backend query, static uploads and move-only buffers, textures, samplers, shaders and pipelines.

Public headers do not expose SDL declarations. E1's private smoke program records one command buffer, acquires the swapchain, clears it, draws a vertex-buffer triangle, binds a sampled checker texture and draws a six-vertex quad, then submits. Resources and uploads are created once before the loop.

## Backend policy

`automatic`, `preferVulkan` and `requireVulkan` are configuration values. Fedora smoke runs request Vulkan; `preferVulkan` logs the creation error before asking SDL to select another supported backend, while `requireVulkan` fails. The reported name always comes from `SDL_GetGPUDeviceDriver`. E1 compiles SPIR-V and therefore provides the Fedora/Vulkan artifact first; future shader outputs add DXIL and metallib without spreading backend branches through the renderer.

## Boundaries

The Engine has no tower, enemy, wave, economy, biome, MAP FORGE or game-map knowledge. Game code decides which generic resource and transform to submit. AEGIS assets remain under the repository `assets/`; E1's checker pixels are a generic in-memory smoke asset.

See `DEPENDENCIES.md`, `ENGINE_MIGRATION.md` and `engine/mediaforge/README.md` for build and extraction details.
