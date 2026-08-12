# Dependency versions

Versions are centralized in `engine/mediaforge/CMakeLists.txt`. Upgrade `MEDIAFORGE_SDL_VERSION` and `MEDIAFORGE_SHADERC_VERSION` there after checking official stable releases, then update this file and run both build configurations.

| Dependency | Exact version | Purpose | Official source |
|---|---:|---|---|
| SDL | 3.4.14 | Window, events, input and SDL_GPU | `https://github.com/libsdl-org/SDL/releases/tag/release-3.4.14` |
| shaderc / `glslc` | 2026.1 | Offline GLSL → SPIR-V build tool only | `https://github.com/google/shaderc/releases/tag/v2026.1` |
| SFML | system 2.5+ (verified 2.6.2) | Transitional AEGIS runtime only; never an Engine dependency | distribution package |
| CMake | 3.25+ | Project and standalone Engine build | `https://cmake.org/` |

SDL 3.4.14 and shaderc 2026.1 were the latest official stable tags checked on 2026-08-12. `SDL_shadercross` is deliberately not used because its official repository has no stable releases. No preview, RC, nightly or unpinned branch is used.

Resolution order for SDL is an exact matching system CMake package, then official `FetchContent` from `release-3.4.14`. Offline/package-less verification may pass explicit include/library cache paths; these are toolchain inputs and are never embedded in source. Third-party targets are external/system includes.

The shader sources stay under `engine/mediaforge/shaders/`. CMake accepts only a `glslc` whose version output contains 2026.1 and compiles each source once into the build tree. If the tool is absent, the smoke executable builds as an honest diagnostic stub; engine/headless tests remain buildable and the configure step warns explicitly.
