# Gameplay code quality reassessment and safe refactor R1

## Goal

Reassess the current legacy gameplay layer after the interrupted session and make the smallest behavior-preserving readability pass needed for `Tower`. Replace positional `TowerStats` aggregates with named C++23 initialization, add regression coverage for every tower and its upgrade branches, and remove the clearly low-risk `Assets` facade `const_cast` without changing resource-cache behavior. Record larger renderer/simulation separation work for R2 rather than starting the MediaForge gameplay migration.

## Files in scope

- `src/Tower.hpp` and `src/Tower.cpp` for formatting and named `TowerStats` initialization.
- `tests/tower_balance_tests.cpp` and root `CMakeLists.txt` for focused balance/API regression coverage.
- `src/Assets.hpp` and `src/Assets.cpp` only for the logically mutable cache facade and removal of the existing `const_cast` calls.
- Current architecture/migration documentation only where the R1 audit or R2 recommendation needs to be recorded.

`Enemy`, `Projectile`, `Map`, `Effects`, `WaveManager`, and `MapForgeScreen` are audit-only in R1 unless verification reveals a required supporting repair.

## Invariants

- Every tower's damage, range, cooldown, projectile speed, splash, slow factor, slow duration, chain count, cost, upgrade cost, sell value, branch modifier, branch string, targeting behavior, fire timing, projectile construction, direct damage, VFX, sound trigger, and fallback factory behavior remains unchanged.
- No enemy, projectile, wave, Core-damage, map, VFX, or RNG semantics change.
- `TowerStats` field order remains compatible with existing consumers, but tower definitions identify fields by name.
- The `Assets` change remains a logical-const cache access: it may populate the same cache through a `mutable` manager, but it must not alter asset IDs, loading, fallback, or ownership.
- No new SFML dependency enters `src/core/`, simulation is not redesigned, and no E2.3 gameplay migration begins.
- C++23 and `-Wall -Wextra -Wpedantic` remain clean.

## Implementation steps

1. Capture the current balance literals, branch copy, costs, upgrade formulas, fire bodies, and recovery baseline.
2. Expand the tower hierarchy declarations and implementation to the repository's readable modern style.
3. Replace every positional base `TowerStats` aggregate with designated initialization while preserving the exact literals and modifier order.
4. Add focused regression tests for all base stats, level scaling, both level-3 branches, costs/sell values, branch gating, target-mode cycle, branch copy, and factory kind identity.
5. Replace the `Assets` facade `const_cast` calls with a documented logically mutable cache member.
6. Review the diff at token/literal level and run the R1 Debug/Release builds, all CTests, architecture audit, and `git diff --check`.

## Verification

- A dedicated tower regression test uses tolerance-aware floating-point comparison for all eight `TowerStats` fields across every current tower type.
- Both specialization branches are checked independently at the level where they become active; common level scaling and branch gating are covered.
- Costs, upgrade costs, accumulated spend, sell values, max-level behavior, target-mode order, and all branch names/descriptions are exact regression assertions.
- Debug and Release full builds complete with no project warnings.
- All configured CTests and `mediaforge_architecture_audit` pass.
- The source diff contains no gameplay literal or algorithm change beyond named initialization and formatting.

## Progress

- [x] Interrupted worktree, governing architecture, legacy gameplay code, generated-artifact risk, and fresh Debug baseline audited.
- [x] Tower formatting and named stats initialization.
- [x] Tower balance/API regression test.
- [x] Low-risk Assets logical-const cache cleanup.
- [x] R1 Debug/Release/test/audit checkpoint: fresh Debug and Release full builds, 13/13 CTests in each, architecture audit, warning review, and `git diff --check` passed.
- [x] R2 coupling recommendation and final R1 certification: keep SFML vectors, `Assets`/`Effects` fire dependencies, projectile presentation fields, and gameplay render snapshots in the bounded legacy adapter until the E2.3–E2.5 simulation/presentation extraction; no large separation belongs in R1.
