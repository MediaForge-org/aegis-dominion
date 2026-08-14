# E2 performance and resource acceptance

E2 performance acceptance is measurement-driven and is not complete until the same Release build, resolution and gameplay state have been captured on the Fedora desktop. Visual acceptance is a separate gate. E3 must not begin while either gate is pending.

## Measurement status

| Path | Status | Evidence |
|---|---|---|
| Transitional SFML game | Pending matched capture | No repository benchmark mode yet produces split update/render/present timings for a deterministic gameplay state. |
| E2 reference before optimization | User-observed baseline | The supplied E2 screenshot showed about 57 FPS, 16.25 ms labelled CPU, 105 sprites, 0 particles, 23 batches/draws and 212 triangles. The old label included the whole renderer interval and could include swapchain wait, so 16.25 ms must not be treated as CPU rendering cost. RAM and texture/load timings were not captured. |
| E2 reference after optimization | Pending desktop run | The executable now emits a machine-readable `BENCHMARK` line with split CPU/wait/resource counters, but the managed command environment cannot open the user's X11 video device (`SDL initialization failed: No available video device`). No FPS result is invented here. |
| Playable MediaForge integration | Not implemented yet | The current E2 producer remains deterministic scripted reference data. Final performance acceptance requires the real AEGIS simulation and the same snapshot/presentation path. |

## Implemented instrumentation and resource changes

- `cpu_work_ms` is update + sprite submission + particle update + batch construction + command/upload/submission CPU work. It explicitly excludes `present_wait_ms` and `limiter_wait_ms`.
- Reusable event, pass, item, state, ordering, batch and vertex storage replaces avoidable per-frame container construction. `frame_storage_growths=0` after warmup is the steady-state expectation.
- The batch planner uses allocation-free `std::sort` over reserved indices. Alpha sprites remain sequence ordered; only contiguous non-alpha runs may be state-grouped.
- Conservative rotated sprite bounds are culled against the camera viewport before six vertices and a submission item are generated.
- The static reference HUD is rendered into its persistent overlay target only when dirty. Its 53 cached sprites remain included in the reported visible sprite count.
- World/detail/entity work now shares one ordered pass, and emissive/light work shares one ordered pass, eliminating three avoidable target load/transitions.
- High quality keeps the world and emissive paths in RGBA16F, keeps bloom and visible lighting, and evaluates bloom from a half-resolution emissive target. The UI target uses RGBA8 because the clean SDR overlay does not need HDR range.
- Render-target allocation at 1600×900 High changed from 34,560,000 bytes (three full-resolution RGBA16F targets) to 20,160,000 bytes (full RGBA16F world, 800×450 RGBA16F emissive, full RGBA8 UI), a 41.7% reduction. Post-optimization screenshot comparison is still required.
- E2 frame vertex/upload capacity is derived from the requested stress load with a 32,768-vertex floor. The normal scene therefore uses 4,194,304 bytes across two persistent GPU vertex and two upload buffers instead of 76,800,000 bytes at the former 600,000-vertex default, a 94.5% reduction.
- The asset catalog reports unique texture count, approximate RGBA8 residency, load duration and duplicate logical IDs/paths. The current eight unique E2 runtime images total approximately 44,032,192 bytes. Runtime compression/downscaling is not applied until a visual comparison can establish equivalence; source art remains untouched.
- SDL_GPU VSync, immediate and mailbox present modes are supported. Default operation uses VSync plus a 120 FPS ceiling; `--profile` requests immediate uncapped presentation. Losing focus throttles the explicit cap to 15 FPS, and minimized windows skip simulation/render work while continuing low-rate event handling.
- Stress counts are renderer-only diagnostics and do not alter game balance. Repeated enemies, environment sprites and particles use existing asset, queue and pooled-particle paths.

## Exact Fedora commands

Configure and build once:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc) --target aegis_dominion aegis_mediaforge_slice mediaforge_gpu_smoke
ctest --test-dir build-release --output-on-failure
cd build-release
```

Normal power-bounded E2 run:

```bash
./aegis_mediaforge_slice --e2-showcase --vsync on --fps-limit 120 --quality high
```

Uncapped reference benchmark (two-second warmup, ten-second sample):

```bash
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10
```

Stress matrix using the real renderer:

```bash
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-enemies 100
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-enemies 500
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-enemies 1000
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-particles 100
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-particles 1000
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-environment 1000
./aegis_mediaforge_slice --e2-showcase --profile --quality high --warmup-seconds 2 --benchmark-seconds 10 --stress-enemies 1000 --stress-environment 1000 --stress-particles 1000
```

Run each command at the same desktop resolution/scaling with no other benchmark load. Preserve the complete terminal output. For the visual regression gate, capture High at 1600×900 during the same Railgun impact phase and compare terrain, road, lighting, bloom, actors, environment and HUD against the accepted E2 reference.

The old SFML and playable MediaForge commands will be added only after both expose the same deterministic live-gameplay benchmark state. Until then, the final 24-item performance report and the statement that MediaForge approaches SFML efficiency remain pending.

## Non-acceptance functional validation

`SDL_VIDEODRIVER=offscreen` was used only to exercise the completed GPU/benchmark paths in the managed environment. The normal scene reported zero frame-storage growth, and the combined 1,000-enemy + 1,000-environment + 1,000-particle submission also reported zero growth and 33 batches/draws for 3,111 visible sprites. The offscreen backend spent hundreds of milliseconds in command submission and is explicitly not a desktop performance result; its FPS/RSS values are excluded from acceptance.
