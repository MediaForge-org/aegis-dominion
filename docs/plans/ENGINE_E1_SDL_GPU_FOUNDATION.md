# MediaForge E1 – SDL3 / SDL_GPU foundation

## Goal

Add an independently buildable, reusable MediaForge Engine foundation beside the existing AEGIS DOMINION SFML runtime. E1 establishes the dependency boundary, renderer-neutral foundation/math APIs, SDL3 platform/input services, an SDL_GPU device/resource layer, and a graphical smoke executable that clears, draws a GPU triangle, and draws a textured quad.

## Files in scope

- Root integration: `CMakeLists.txt`, `AGENTS.md`, `README.md`.
- Standalone engine subtree: `engine/mediaforge/` (CMake, public headers, sources, shaders, tests, test assets and documentation).
- Architecture/dependency/migration documentation: `docs/ARCHITECTURE_V3.md`, `docs/ROADMAP.md`, `docs/MEDIAFORGE_ENGINE.md`, `docs/DEPENDENCIES.md`, `docs/ENGINE_MIGRATION.md`.
- No gameplay, map-format, balance, asset-art or existing SFML renderer changes.

## Invariants

- Dependency direction is AEGIS → MediaForge → SDL3/SDL_GPU; MediaForge never includes or links AEGIS.
- `src/core/` remains renderer- and engine-agnostic.
- Existing AEGIS/SFML targets, assets, behavior and seven CTests remain present.
- Engine public APIs do not expose SDL types; SDL handles stay in implementation/private native-access boundaries.
- Engine content is generic and contains no AEGIS gameplay types, strings or assets.
- SDL is pinned centrally to the latest official stable release, never an unversioned branch.
- GPU objects and SDL lifecycle are RAII-owned; static GPU resources are not recreated per frame.
- Shader source is external and compiled reproducibly at build time; no embedded source strings or per-frame compilation.
- C++23 and project warning policy remain central and C++26-ready.

## Implementation phases

1. Add standalone engine CMake structure, version/dependency pins, strict target direction and architecture audit.
2. Implement foundation, math, configuration, logging and headless unit tests.
3. Implement SDL3 Window, generic Event/EventPump and frame-based InputState.
4. Implement SDL_GPU device and move-only RAII wrappers for buffers, textures, samplers, shaders and graphics pipelines.
5. Add build-time GLSL → SPIR-V pipeline and smoke executable with clear, triangle and textured checker quad passes.
6. Integrate root build, sanitizers and build identity output without replacing SFML.
7. Update required architecture, dependency, migration, engine README and roadmap documents.
8. Run architecture scans, Debug/Release builds, CTests and warning/diff audits; report graphical verification separately when no display/compiler is available.

## Verification

- Configure identity prints MediaForge version, C++ standard, SDL version, compiler and build type.
- Fresh Debug and Release builds with `-Wall -Wextra -Wpedantic` contain no diagnostics from project targets.
- All existing AEGIS tests plus MediaForge foundation/math/audit tests pass.
- Audit rejects AEGIS includes/terms, SFML references and AEGIS target links below `engine/mediaforge/`.
- `mediaforge_gpu_smoke` has external compiled shader assets and contains the complete SDL_GPU upload/render/submit path.
- Manual Fedora display run logs the actual backend from `SDL_GetGPUDeviceDriver`; Vulkan preference is requested, never fabricated.

## Progress

- [x] Repository/branch/worktree, existing targets/tests and official dependency releases audited.
- [x] Engine boundary and foundation/math.
- [x] SDL platform/input and GPU resource layer.
- [x] Shader pipeline and GPU smoke executable.
- [x] Root integration and documentation.
- [x] Debug/Release/test/audit verification.

## Result (2026-08-12)

MediaForge 0.1.0-dev now builds both as a standalone subtree and as an additive dependency of the unchanged AEGIS SFML executable. The public API is SDL-free; SDL3 3.4.14 owns platform/events and SDL_GPU device/resources privately. External GLSL sources compile once through exactly shaderc/glslc 2026.1 when available. The full smoke code and shader branch compile cleanly and implement clear, vertex-buffer triangle and uploaded checker-texture quad rendering.

Fresh GCC 16.1.1 Debug and Release root builds completed without project diagnostics. Nine CTests passed in each configuration: seven retained AEGIS tests, 25 new headless Engine checks and the architecture audit. The standalone Engine build and its two CTests also passed. Across test executables there are 89 domain assertions plus the architecture audit.

Visual GPU acceptance is not claimed. The managed process cannot open the existing X11 session (`SDL initialization failed: x11 not available`). The installed shaderc runtime was sufficient to verify the real compiled shader/smoke branch through a temporary out-of-repository frontend, but the environment lacks the supported `glslc` executable. The documented Fedora run remains required to observe the triangle/quad and confirm the actual backend line.
