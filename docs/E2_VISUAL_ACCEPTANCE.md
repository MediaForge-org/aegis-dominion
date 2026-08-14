# E2 visual acceptance

Technical E2 is implemented. Artistic acceptance requires the user's real Fedora display review; Codex does not self-certify it.

## Build and launch

Install the exact stable toolchain dependencies documented in `DEPENDENCIES.md`, including `glslc` 2026.1 and a working Vulkan driver. From the repository root:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc) --target aegis_dominion aegis_mediaforge_slice mediaforge_gpu_smoke
cd build-release
```

OLD transitional SFML game:

```bash
./aegis_dominion
```

NEW MediaForge E2 slice:

```bash
./aegis_mediaforge_slice --e2-showcase
```

Performance profiling and stress commands are documented in `docs/E2_PERFORMANCE.md`. High now uses a reduced-resolution HDR emissive/bloom target and an RGBA8 UI cache; acceptance requires confirming that these bandwidth/memory changes remain essentially equivalent to the accepted reference appearance.

E1 diagnostic regression:

```bash
./engine/mediaforge/mediaforge_gpu_smoke
```

The MediaForge runs must print the actual backend, expected on Fedora as `MediaForge GPU backend: vulkan`. Press `Esc` to exit.

## Manual matrix

Resize the E2 window to 1280×720, 1600×900 and 1920×1080. At all three sizes verify 16:9 framing, crisp letterboxing, stable HUD/world relationship, no stretch/crop, readable silhouettes, no atlas bleed and sensible high-DPI behavior.

Compare OLD and NEW at 1600×900 with the same monitor scaling. Confirm material improvement in terrain richness, road/shoulder blend, reduced empty space, prop composition, landmark hierarchy, Raider/Heavy Tank distinction, grounded shadows, controlled cyan/amber lighting, Pulse/Railgun separation, projectile/impact/explosion layering and HUD depth/hierarchy.

Also verify:

- Spawn Gate reads as the enemy entry landmark; Core reads as the defended objective.
- Pulse base remains stationary while its head tracks/recoils; Railgun charges, traces and impacts distinctly.
- The explosion contains flash, ring, smoke, sparks/debris and light impulse rather than one expanding circle.
- Bloom helps emissive energy but does not wash out grass/road; vignette remains subtle.
- UI is the last clean pass and is unaffected by world lighting.
- Terminal/title statistics update without draw-call explosion. Expected reference range is approximately 102–150 sprites, 0–36 particles and 21–30 batches/draws depending on effect phase. Hardware FPS/frame time must be recorded from the user's system; software/offscreen Vulkan is not a performance target.

## Capture set

Capture exactly:

1. OLD SFML Verdant gameplay at 1600×900 with a wave active, at least one tower and enemies visible.
2. NEW MediaForge E2 at 1600×900 during the Railgun impact/explosion, with Spawn Gate, Core, Raider, Heavy Tank, Pulse, Railgun and HUD visible.
3. NEW at 1280×720 and 1920×1080 to prove framing/resizing.
4. A 15–20 second NEW video including enemy motion, Pulse fire/recoil, Railgun charge/tracer/impact, smoke/particles and one visible stats update.
5. Terminal crop showing `MediaForge GPU backend: vulkan` and the stats line.
6. E1 smoke screenshot showing triangle and checker quad.

Reject E2 if the NEW scene still reads essentially like the old prototype, critical actors are obscured, road/path logic looks disconnected, bloom dominates, silhouettes are ambiguous, atlas mattes are obvious, or any required resolution breaks framing.

Final status until the post-optimization captures are reviewed: **visual acceptance pending user review**.
