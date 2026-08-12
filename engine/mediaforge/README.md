# MediaForge Engine 0.1.0-dev

MediaForge is a reusable C++23 game engine built on SDL3 and SDL_GPU. AEGIS DOMINION is its first consumer, not part of the Engine. Game rules, content names, maps, UI copy and game art must never enter this directory.

## Dependencies

- SDL 3.4.14, exact official stable release.
- shaderc/glslc 2026.1, exact stable offline shader compiler for the smoke build.
- CMake 3.25+ and a C++23 compiler.

The version pins and upgrade location are at the top of `CMakeLists.txt`. There is no SFML dependency.

## Build standalone

```bash
cmake -S engine/mediaforge -B build-mediaforge -DCMAKE_BUILD_TYPE=Debug
cmake --build build-mediaforge -j
ctest --test-dir build-mediaforge --output-on-failure
```

If an exact system SDL package is unavailable, CMake fetches the official stable tag. Set `MEDIAFORGE_FETCH_DEPENDENCIES=OFF` for strictly offline package-only builds.

## GPU smoke test

```bash
cmake --build build-mediaforge --target mediaforge_gpu_smoke
./build-mediaforge/mediaforge_gpu_smoke
```

From the AEGIS root build the executable is under `build/engine/mediaforge/`. On Fedora install the SDL3 development package, shaderc/glslc 2026.1, Vulkan loader/driver and validation layers. A successful default development run logs the actual line `MediaForge GPU backend: vulkan` and displays a dark clear, colored triangle and checker-textured translucent quad. Escape or window close exits cleanly.

## Repository extraction

This directory already contains its public include root, sources, shaders, tests, docs, dependency resolution and standalone `project()` path. Moving it to a new repository does not require copying AEGIS source or assets. The future consumer switches from `add_subdirectory(engine/mediaforge)` to an installed package or exact pinned repository dependency.
