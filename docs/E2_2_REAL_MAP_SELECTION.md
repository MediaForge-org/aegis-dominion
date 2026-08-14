# E2.2 real map selection and launch handoff

## Runtime flow

Configure and build on Fedora from the repository root:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc) --target aegis_mediaforge_slice mediaforge_gpu_smoke
ctest --test-dir build-release --output-on-failure
cd build-release
./aegis_mediaforge_slice
```

`SPIELEN` opens the AEGIS-owned `MAPAUSWAHL`. The first three entries are loaded in the established order from `maps/verdant.aegismap`, `maps/frost.aegismap`, and `maps/ember.aegismap`. Other `.aegismap` files in the same runtime directory are sorted by filename and shown as `CUSTOM / MAP FORGE`. The post-build step copies the repository maps beside the MediaForge executable; when run from the repository root, the source `maps/` directory is used directly.

Each file is parsed by `MapDocument::load()` and converted by `buildPlayableMap()`. There is no MediaForge parser or second map database. Parse failures, fatal validation errors and every entry sharing a duplicate stable metadata ID remain visible as red `UNGÜLTIG` cards with a concise reason. They cannot be selected and never enable `START`.

No map is selected implicitly. Mouse release on a valid card selects it. Up/Down or Left/Right moves through valid maps and skips invalid entries; Tab moves button focus and Enter/Space activates the focused card or action. The wheel scrolls the four-row viewport. Model selection keeps a keyboard-selected entry visible. `ZURÜCK` or Escape returns to the main menu.

The selected view shows real `MapDocument`/`PlayableMap` name, stable ID, biome, dimensions, authored difficulty and route-node count. Existing authored `assets/maps/*.png` imagery is used for the three built-ins. Real zones, route nodes, spawn and goal are projected over that image. Custom maps use a biome field plus the same real geometry. The persistent UI target is only regenerated when screen/selection/hover/focus changes, so an idle preview is cached.

`START` is enabled only for a selected valid entry. It creates `GameLaunchConfig{StandardGameLaunch{selected PlayableMap}}`, then a `core::GameSession` owns that launch. The gameplay screen reads only `session.map()` and presents its real map image/geometry plus ID, dimensions, path, spawn and goal metadata. There is no map index, file reparse, Verdant fallback or `--e2-showcase` transition. Back/Escape destroys the temporary session and returns to the same selection.

## Fedora manual acceptance

1. Run the build commands above and start `./aegis_mediaforge_slice` from `build-release/`.
2. Click `SPIELEN`; confirm `Grüne Grenze`, `Frostpass`, and `Aschefeld` are present and no placeholder copy appears.
3. Hover each card and select `Grüne Grenze`; confirm its selected surface, ID `verdant_frontier`, Verdant preview, `1200 x 900`, and route overlay.
4. Select `Frostpass`; confirm the image, biome/metadata, route, spawn and goal visibly change.
5. Use Up/Down (and Left/Right), Tab, Enter/Space and the mouse wheel; confirm selection remains visible and invalid cards, if present, remain disabled.
6. Click `ZURÜCK`, confirm the main menu, click `SPIELEN` again, and confirm the process did not restart.
7. Select `Aschefeld` and click `START`.
8. Confirm the E2.2 gameplay session shows `ember_field`, `1200 x 900`, the Aschefeld path, spawn `(0, 240)` and goal `(1200, 690)`.
9. Confirm no enemies/towers from the scripted E2 reference appear and the screen explicitly says E2.3 has not started.
10. Click `ZURÜCK` or press Escape and confirm return to map selection, then return to the main menu without restarting.
11. Separately run `./aegis_mediaforge_slice --e2-showcase` and confirm the opt-in reference still starts only through that flag.

E2 remains open. E2.3 was not started.
