# Asset pipeline

All SFML texture, font and sound-buffer file loading lives in `assets::AssetManager`. Callers use `TextureId`, `FontId` or `SoundId` with logical names such as `ui.logo`, `tower.pulse.base`, `enemy.raider.idle` and `environment.verdant.background`.

`AssetCatalog` maps each logical ID to type, relative file path and texture sampling metadata. Runtime definitions now come from `assets/manifest.aegis` (`AEGIS_ASSETS_V1`); the compiled catalog is only the failure-safe fallback. Each line stores type, quoted logical ID, quoted relative path, smooth and repeat flags. Duplicate IDs, paths masquerading as IDs, malformed rows and type-mismatched lookup are rejected.

`ResourceCache` owns each successfully loaded resource once and returns the same object for duplicate requests. `AssetManager` owns all SFML resources through RAII. `Assets` is a compatibility/domain facade only; it translates tower/enemy/map names to typed IDs and never loads files.

Missing textures use a conspicuous magenta/black checkerboard, missing fonts use a logged system font with verified German glyph coverage, and missing sounds use a silent buffer. Each missing logical asset is logged once. Shipping packages should include a licensed UTF-8-capable font in `assets/fonts/`; system font discovery is a development fallback.

## Production flow

```text
source art / procedural source
  → lossless export (PNG/WAV/TTF)
  → assets/<domain>/<snake_case_name>.<ext>
  → logical dotted ID in manifest.aegis
  → AssetManager validation + cache
  → snapshot visual ID
  → renderer
```

Runtime code never opens texture/sound/font files. New IDs use `domain.subject.variant`, for example `terrain.grass.surface`, `world.aegis_core` or `tower.pulse.turret`. Source art is kept separate from exported runtime files when introduced; `tools/generate_phase4_assets.py` is the reproducible source for current terrain/world/environment intermediates.

`assets/animations.aegis` uses `AEGIS_ANIMATIONS_V1`. A clip row defines logical clip ID, sprite-sheet asset ID, loop/once, sequential frame count, first frame rectangle and duration. `AnimationCatalog` rejects malformed and duplicate definitions; the WorldRenderer consumes state timing without owning gameplay state.

## E2 MediaForge manifest

`assets/e2/manifest.mfassets` records each logical ID's path, atlas/source dimensions, filter/wrap, pivot, scale, material and optional emissive/normal/animation references. The AEGIS catalog validates it, loads each PNG once through MediaForge and uploads eight resident textures; renderer code contains no individual file paths.

Sources remain under `assets/e2/source/`: a 1672×941 battlefield, 1254–1536-class structures/units/environment/VFX/HUD, and a 1024×1536 separate tower-parts atlas. IDs/metadata allow commissioned replacements without renderer changes.
