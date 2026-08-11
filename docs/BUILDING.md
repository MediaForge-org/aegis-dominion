# Building AEGIS DOMINION

## Required toolchain

AEGIS DOMINION uses C++23 as its minimum and default project standard. A supported setup needs:

- a modern GCC or Clang with C++23 language support;
- CMake 3.25 or newer;
- SFML 2.5 or newer with graphics, window, system and audio components.

The verified Fedora toolchain is GCC 16.1.1 with SFML 2.6.2. GCC and Clang builds share standard CMake compile features and avoid GNU language extensions. Clang was not installed in the verification environment, so portability is configuration-supported but not claimed as locally tested.

## Central language-standard option

`AEGIS_CXX_STANDARD` is the only project-owned C++ standard selector:

```cmake
set(AEGIS_CXX_STANDARD 23 CACHE STRING "C++ standard used by AEGIS DOMINION")
```

`aegis_project_options` propagates `cxx_std_${AEGIS_CXX_STANDARD}` and the GCC/Clang warning flags to every own library, executable and test. CMake extensions are disabled. Only values 23 and 26 are accepted, and configuration fails if the selected compiler does not advertise the requested compile feature. There is no fallback to an older standard.

Build and run scripts intentionally do not duplicate `-std=` flags; CMake remains the source of truth.

## Fedora dependencies

```bash
sudo dnf install gcc-c++ cmake SFML-devel
```

## Debug build and tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Release build and tests

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
ctest --test-dir build-release --output-on-failure
```

`./build.sh` remains the short Release-build path, and `./run.sh` starts the executable from `build/` so copied assets and maps are found.

## Verifying the selected standard

CMake exports `compile_commands.json` in each build directory. For the default configuration, every project compile command must contain `-std=c++23` and none may contain an older project standard. The shared target also keeps `-Wall -Wextra -Wpedantic` active for GCC and Clang.

## Future C++26 experiment

C++26 is not currently the official or required standard. A future compatible compiler/CMake combination can test it centrally with:

```bash
cmake -S . -B build-cxx26 -DAEGIS_CXX_STANDARD=26
cmake --build build-cxx26 -j
ctest --test-dir build-cxx26 --output-on-failure
```

Changing the default later requires one source edit: change the cached default value in the top-level `CMakeLists.txt` from `23` to `26`. No individual target or runtime source should need a separate standard flag.
