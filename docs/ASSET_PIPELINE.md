# Asset pipeline

All SFML texture, font and sound-buffer file loading lives in `assets::AssetManager`. Callers use `TextureId`, `FontId` or `SoundId` with logical names such as `ui.logo`, `tower.pulse.base`, `enemy.raider.idle` and `environment.verdant.background`.

`AssetCatalog` maps each logical ID to type, relative file path and texture sampling metadata. The built-in catalog is C++ data today and deliberately has a small API that can later be populated by JSON, TOML or a custom manifest without changing callers. Duplicate IDs, paths masquerading as IDs and type-mismatched lookup are rejected.

`ResourceCache` owns each successfully loaded resource once and returns the same object for duplicate requests. `AssetManager` owns all SFML resources through RAII. `Assets` is a compatibility/domain facade only; it translates tower/enemy/map names to typed IDs and never loads files.

Missing textures use a conspicuous magenta/black checkerboard, missing fonts use a logged system font with verified German glyph coverage, and missing sounds use a silent buffer. Each missing logical asset is logged once. Shipping packages should include a licensed UTF-8-capable font in `assets/fonts/`; system font discovery is a development fallback.
