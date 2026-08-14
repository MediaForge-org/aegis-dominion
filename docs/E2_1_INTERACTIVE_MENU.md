# E2.1 interactive MediaForge menu

## Launch

From a configured build directory:

```bash
./aegis_mediaforge_slice
```

Normal startup opens the interactive AEGIS DOMINION main menu through MediaForge → SDL_GPU → Vulkan. The preserved Verdant visual/benchmark reference is opt-in:

```bash
./aegis_mediaforge_slice --e2-showcase
```

Existing showcase configuration remains composable, for example:

```bash
./aegis_mediaforge_slice --e2-showcase --vsync on --fps-limit 120 --quality high
```

## Implemented flow

- `SPIELEN` opens a real interactive screen that clearly marks map selection as E2.2 work and provides `ZURÜCK`.
- `MAP FORGE` opens a migration-pending screen with `ZURÜCK`; the SFML editor is unchanged.
- `ANLEITUNG` opens readable German UTF-8 instructions with `ZURÜCK`.
- `EINSTELLUNGEN` applies VSync immediately, cycles 30/60/120/144/240 FPS limits immediately and cycles Low/Medium/High/Ultra quality. Quality recreates the menu world/emissive targets with the corresponding existing E2 format/bloom profile.
- All three rows are generated from AEGIS-owned setting descriptors. The descriptors expose stable IDs (`display.vsync`, `display.fps_limit`, `graphics.preset`), Display/Graphics category metadata, a distinct setting type, actual options, current index, display value, wrap behavior and optional help/disabled/restart metadata. They are read-only views derived from `RuntimeSettings`, not a second settings state.
- Every current row uses the reusable MediaForge `ui::SegmentedLevelIndicator`. VSync renders 1/2 or 2/2, FPS renders 1/5 through 5/5 from its real option list, and quality renders 1/4 through 4/4. The component draws narrow rectangle geometry; it has no text-glyph or AEGIS-setting dependency.
- A setting activation starts the same restrained 125 ms indicator pulse regardless of which row changed. Mouse release and Enter/Space both produce the same setting action and therefore the same runtime side effects, wrap behavior and animation.
- `BEENDEN` requests normal window shutdown; destructors release renderer, GPU, window and SDL resources.

Mouse buttons expose normal, hover and pressed feedback. Disabled and keyboard-focused states are supported by the generic widget contract. Tab/Down and Up move keyboard focus; Enter or Space activates the focused button. Escape navigates back and closes cleanly from the main menu.

## Coordinate and power behavior

SDL mouse positions are mapped from the current logical window size into the fixed 1600×900 menu canvas through aspect-preserving letterboxing. High-density drawable scaling remains inside the renderer's final composite, keeping pointer hit regions aligned at 1280×720, 1600×900, 1920×1080 and other aspect ratios.

The menu world, emissive and UI layers live in persistent render targets. Static world/emissive content is submitted only at creation or quality changes; UI geometry is resubmitted only when screen, setting, hover, press or focus state changes, plus the bounded 125 ms change pulse. Idle frames reuse those targets, and interaction-state buffers retain capacity instead of allocating each frame. VSync and the selected FPS limit remain active, unfocused execution is limited to 15 FPS, and minimized execution skips rendering and is limited to 15 FPS.

## Settings extension path

The AEGIS descriptor type distinguishes cyclic discrete, boolean, continuous numeric, key-binding and action settings. E2.1 presents the current boolean with two segments, while its boolean type remains intact so a generic toggle can replace that presentation later. Continuous values can use a slider, key bindings a binding control and actions a button without changing stable IDs, category ownership or runtime source-of-truth rules.

Adding a future discrete setting means adding its runtime field and real option mapping to the AEGIS model plus its descriptor. The settings screen lays out descriptor rows and obtains segment count/current index directly from that metadata; it does not require setting-specific drawing or animation code. Display, Graphics, Audio, Controls, Interface, Gameplay and Accessibility remain game-owned categories. Only Display and Graphics and the existing three settings are instantiated in this milestone.

## Scope boundary

E2 is still open. E2.1 only establishes the first functional interactive MediaForge game flow and reusable current-settings foundation. It does not add the future display/graphics/audio/control/interface/gameplay/accessibility settings, persistence storage, sliders, toggle widgets or binding controls. It does not migrate live gameplay, map selection data, combat, tower building, waves or MAP FORGE, and it does not start E2.2 or E3.
