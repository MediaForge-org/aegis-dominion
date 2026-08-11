# C++23 toolchain migration

## Goal

Move AEGIS DOMINION from C++17 to a centrally configured C++23 project standard without changing runtime behavior, game data, rendering, MAP FORGE or the playtest flow. Keep a future C++26 experiment to one cache-variable override.

## Files in scope

- Build configuration: `CMakeLists.txt`, audit of `build.sh`, `run.sh` and repository CMake/scripts.
- Standard declarations visible to contributors/users: `AGENTS.md`, `README.md`, `docs/ARCHITECTURE_V3.md`, `docs/ROADMAP.md`, new `docs/BUILDING.md`.
- Stale visible version label: `src/screens/MainMenuScreen.cpp`.
- Historical wording that incorrectly appears current: `docs/plans/PHASE_01_DOMINION_FOUNDATION.md`.

## Invariants

- No runtime/game/editor/map-format behavior changes.
- `AEGIS_CXX_STANDARD` is the only project-owned language-standard selector.
- All project libraries, executables and tests consume one INTERFACE configuration target.
- GNU language extensions remain disabled.
- Unsupported values or compiler standards fail configuration; no fallback occurs.
- Warning flags remain `-Wall -Wextra -Wpedantic` for GCC and Clang.
- Existing dirty worktree content and generated `build/` data are not reverted or treated as source.

## Implementation steps

1. Add cached `AEGIS_CXX_STANDARD=23`, allow only 23/26, validate compiler feature availability and create `aegis_project_options`.
2. Link every own compiled target to the shared project options and remove repeated target warning blocks/global C++17 settings.
3. Update active and historical C++17 references plus build/toolchain documentation.
4. Re-audit source files while excluding generated build directories.
5. Perform fresh Debug and Release builds/tests outside the repository and inspect generated compile commands for C++23 on every target.

## Verification

- Fresh CMake configure/build with GCC and `AEGIS_CXX_STANDARD` default.
- `compile_commands.json` contains `-std=c++23` and no older `-std` flags for all translation units.
- Five CTest targets and the existing 43 reported domain checks remain green.
- Fresh Debug and Release build logs contain no warnings.
- Repository audit finds no stale C++17/C++20 selector outside documentation describing the migration mechanism.

## Progress

- [x] Central C++23/C++26 cache option and shared project-options target added.
- [x] All eight project build targets connected to the shared configuration.
- [x] Active repository standard references and build documentation updated.
- [x] Fresh Debug and Release builds completed outside the dirty repository build tree.
- [x] All five CTest targets and all 43 reported domain checks passed.

## Result (2026-08-12)

GCC 16.1.1 and SFML 2.6.2 configured and built all 26 translation units in both Debug and Release. Both exported compile-command sets contain exactly 26 `-std=c++23` entries and the common `-Wall -Wextra -Wpedantic` flags; all eight target flag files use the same configuration. Both CTest runs passed 5/5 targets, and the test executables reported 43 domain checks. No compiler warnings were emitted. Clang was not installed in the environment, so the Clang-compatible configuration was audited but not compiled locally.

The repository's tracked `build/` directory was not removed or reused because it already contained user/Phase-2 changes and an untracked saved map. Clean verification instead used isolated directories below `/tmp`.
