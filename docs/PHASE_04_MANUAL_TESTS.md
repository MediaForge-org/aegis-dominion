# Phase-4 manual Fedora visual tests

Run from the Release build through `./run.sh` or from its directory so `assets/manifest.aegis` is found. For every case watch for checkerboard fallbacks, frame hitches, path/actor divergence and UI overlap.

## Window matrix

Repeat the main menu → gameplay → pause → return flow at 1280×720, 1600×900 and 1920×1080. Confirm aspect-preserving letterboxing, crisp nine-slice corners, stable world/UI separation, correct pointer hit regions and no stretched fonts/icons.

## Built-in maps

1. Start Verdant: verify deep green authored terrain, smooth overlaid road, green environment palette, animated spawn gate and readable AEGIS Core.
2. Start Frost: verify cold surface/shoulder treatment and sparse rock/ruin/antenna palette.
3. Start Ash: verify warm ash shoulder/material accents and rock/scrap/ruin palette.
4. On each map start a wave and confirm enemies follow the road center through every curve.

## MAP FORGE custom map

1. Create a new valid map with at least five nodes forming two strong curves; set biome to `verdant`, `frost`, then `ash` in separate runs.
2. Add a Build Zone, Blocked Zone and Water rectangle away from the path plus spawn and goal; save, reopen and playtest.
3. Confirm material texture and seeded decorations appear automatically, do not cover road/spawn/core and remain identical after restarting the same playtest.
4. Confirm Water is an animated surface with edge treatment, not a translucent debug rectangle.
5. Confirm Build/Blocked overlays are absent during normal combat. Select a tower and confirm buildable space appears subtly; valid preview is green and invalid path/water/blocked preview is red.
6. Return to MAP FORGE and confirm its terrain preview, curve preview, grid, logical nodes and explicit zone overlays remain readable.

## Combat matrix

Build all six towers and verify separate base/head sprites and eased tracking:

- Pulsar: cyan bolt, muzzle tracer and visible recoil.
- Mörser: ballistic shell, warm muzzle sparks, explosive impact/ring and restrained shake.
- Kryo: ice shard projectile, frost muzzle particles/trail and pale impact.
- Railgun: purple charge ring, long tracer, impact sparks and small impulse.
- Tesla: green chained arcs that remain distinct from rail/energy.
- Raketen: missile body, smoke trail, orange explosion and stronger short shake.

Spawn mixed enemy waves and confirm Raider, Runner, Tank, Shield, Regenerator, Splitter and Boss silhouettes remain distinct; spawn scaling/movement bob/hit flash work; ordinary full-health enemies have no bar; damaged enemies show health; Shield shows a separate blue bar/glow; Boss remains clearly larger with a bar. Confirm deaths emit particles instead of silently disappearing.

## UI/screens

Open Main Menu, Tutorial, Map Select, Gameplay, pause/end modals and MAP FORGE. Confirm authored nine-slice surfaces appear on cards/buttons/modals, selected states retain clear accents, German umlauts/ß render, tool selection and inspector grouping remain readable, tooltips stay on top, and modal UI is always the last pass.

## Performance/fallback

Play a late wave at 2× with all tower types. Watch for increasing frame time or particle accumulation; pause/resume twice. Temporarily rename one copied environment PNG in an expendable build directory and confirm one logged missing-ID error plus checkerboard fallback, then restore it. Confirm textures and shaders are not reloaded/created per frame in logs/profiling.
