# AEGIS DOMINION architecture

## MediaForge engine boundary

The long-term runtime boundary is now explicit:

```text
AEGIS DOMINION game layer
        │ uses
        ▼
MediaForge Engine (generic, extractable)
        │ uses
        ▼
SDL3 platform + SDL_GPU
```

`engine/mediaforge/` is an independent CMake subtree with public headers rooted at `include/mediaforge/`. It never includes `src/`, never links an AEGIS target and contains no game concepts or game assets. The inverse link is represented concretely by `aegis_dominion` linking `mediaforge_engine`. A CTest scans Engine source/include files for forbidden AEGIS/SFML dependencies and CMake also owns only a private SDL3 link.

MediaForge foundation/math and typed GPU resource descriptions are SDL-free. Window uses a private implementation, events translate SDL data into generic variants, and GPU resources are move-only RAII objects. The private E1 smoke implementation is the only code that uses native command encoding directly. This preserves a generic public boundary while the fuller renderer API is designed in E2.

**SFML IS TRANSITIONAL.** The current AEGIS application, UI and world renderer remain operational and are not rewritten in E1. No new SFML dependency may enter gameplay simulation, map data or `src/core/`; long-term presentation work targets MediaForge.

## Runtime composition

The Phase 3 runtime remains screen-based and composes presentation services explicitly:

```text
Application
  ├─ RenderWindow + reference-resolution view
  ├─ AssetManager/Catalog + TextService facade
  ├─ InputSystem + Logger + Settings model
  └─ ScreenManager (queued stack navigation)
       ├─ MainMenuScreen (menu, map selection, tutorial)
       ├─ GameScreen (match orchestration and current SFML gameplay view)
       └─ MapForgeScreen (editor UI and interaction)
```

`Application` owns only the window lifecycle, event loop, common services and screen dispatch. Navigation requests are queued so a screen is never destroyed while handling its own event. The stack is important for playtesting: MAP FORGE pushes a `GameScreen`; popping that screen exposes the same editor instance, including camera, selection, tool, undo/redo and unsaved document.

The request/factory boundary is ready to add `MapSelectScreen`, `SettingsScreen`, `TutorialScreen`, `CodexScreen`, `ArsenalScreen` and `ResultsScreen` without reintroducing a central state switch. Map selection and tutorial still live as bounded pages inside `MainMenuScreen` in this phase.

## Presentation boundaries

`AssetManager` is the only resource file-loading boundary. Strong `TextureId`, `FontId` and `SoundId` values resolve logical catalog names, cache owned SFML resources and return explicit checkerboard/font/silent fallbacks. The legacy `Assets` class is now only a domain-friendly adapter for existing tower, enemy, map and sound call sites.

`InputSystem` owns default/rebindable action mappings for Menu, Gameplay, MapForge and Modal contexts. Application records pointer events once; UI/modal regions consume presses before world logic. Text editing and directional point nudging retain direct low-level key interpretation because they are text/editor primitives rather than global commands.

`ui::Theme`, `ButtonInteraction`, `LinearLayout`, `UiScale` and `UiRenderer` form the UI boundary. Application renders a 1600×900 virtual UI through an aspect-preserving letterboxed view, so coordinates, text and icons scale together without modifying simulation coordinates.

`render::Layer` defines Terrain, Water, Road, Environment, Zones, Enemies, Towers, Projectiles, Effects, WorldUi, ScreenUi and ModalUi. `GameScreen` extracts one immutable `WorldRenderSnapshot` each frame. `WorldRenderer` owns the ordered world passes; `Enemy`, `Tower` and `Projectile` no longer issue arbitrary SFML draw calls. Their snapshots contain transforms, logical visual IDs, animation state, health/shield presentation, selection and effect profiles, but no targeting, damage or wave logic.

`core::samplePathCurve()` is SFML-free. `GameMap` projects logical route nodes once, samples the Catmull-Rom curve and exposes that same point list to enemy movement, build-distance checks and road rendering. The editor keeps storing logical nodes and previews the sampled curve without changing the document.

## Data and rendering boundaries

```text
renderer-independent                         SFML-facing

core/MapDocument ──> core/PlayableMap ──> Map/GameMap ──> GameScreen
       ↑
editor/MapEditorModel                    MapForgeScreen + EditorCamera
```

- `src/core/` contains versioned map data, structural parsing, detailed semantic validation and playtest conversion. It has no SFML/OpenGL types.
- `MapEditorModel` is the only mutation/history gateway for editable content. Its bounded checkpoint history tracks revisions, dirty state and redo branching.
- `MapForgeScreen` owns transient UI state only: tool, selection, drag preview, camera, grid/snap and dialogs. It renders the current `MapDocument`; there is no second editable path or zone copy.
- `EditorCamera` converts screen/world coordinates without changing map data.
- `GameMap` is the current SFML/gameplay adapter. It projects a selected playable route to the fixed 1200×900 gameplay viewport and blocks building in blocked/water zones.
- `GameScreen` still coordinates match simulation, input and HUD, but delegates all world presentation to `WorldRenderer` through snapshots.

## Validation and playtest flow

`MapDocument::validateDetailed()` returns severity plus object identity/index. MAP FORGE shows errors and warnings and can select referenced path nodes, endpoints and zones. Fatal errors block playtest; warnings do not.

`buildPlayableMap()` selects the first route that has both a spawn and goal, copies route/endpoints/zones and never mutates the source document. `makeMapForgePlaytestLaunch()` wraps that complete value in the `GameLaunchConfig` variant. A normal launch contains `StandardGameLaunch{mapIndex}`; a MAP FORGE launch contains `MapForgePlaytestLaunch{PlayableMap}`. The alternatives cannot be confused through a boolean or missing optional document.

`GameMap::loadFromPlayableMap()` consumes the playtest value directly and never calls the standard-map loader. Its SFML-facing adapter applies one deterministic projection from document world space into the current 1200×900 gameplay plane:

```text
gameX = worldX × 1200 / mapWidth
gameY = worldY ×  900 / mapHeight
```

The same scale pair is applied to path nodes, spawn, goal and every zone rectangle. Original dimensions remain in session metadata and are displayed in the playtest HUD. This projection is isolated in the current 2D adapter; `MapDocument` and `PlayableMap` retain semantic world coordinates for future renderers.

Built-in games retain their authored background PNG as an optional terrain input but then use the same road/endpoints/environment/entity passes as custom maps. MAP FORGE playtests choose a material from biome data, use `terrainSeed` for deterministic patches/decorations and render water as an animated surface. Build/blocked rectangles remain mechanics/editor data: gameplay hides them unless building/debugging.

## Toolchain boundary

AEGIS DOMINION uses C++23 for every project library, executable and test. `AEGIS_CXX_STANDARD` is the single selector, and the `aegis_project_options` INTERFACE target propagates `cxx_std_23` plus the common warning policy. GNU language extensions are disabled globally. CMake rejects values other than 23/26 and rejects a requested standard that the selected compiler does not advertise.

C++26 is not the current project standard. The build structure merely reserves `-DAEGIS_CXX_STANDARD=26` as a future toolchain experiment without requiring runtime-source or per-target CMake edits. See `BUILDING.md`.

## Text boundary

Repository/map strings are UTF-8 `std::string`. `ui::TextService` converts them using `sf::String::fromUtf8`. MAP FORGE text entry encodes SFML Unicode events back to UTF-8, including German umlauts and ß.

## Remaining architecture debt

- Gameplay entities still use SFML vectors internally; snapshots isolate rendering, but complete renderer-agnostic combat simulation remains future work.
- Current gameplay supports one active enemy route per match; maps may store several routes and MAP FORGE edits all of them.
- Map selection/tutorial are menu pages rather than independent screens, although navigation supports extracting them.
- The inspector uses shared panels/buttons/labels but does not yet provide reflection or complete numeric/text widget editing.
- MAP FORGE previews terrain and stored decorations flow into playtest, but terrain painting, height sculpting and decoration placement tools are not active yet.
- Current enemy sheets are single-pose transitional art; external clip definitions and state playback are ready for multi-frame replacements.
- Waves, economy and balance remain hard-coded.
- AEGIS presentation still runs through SFML; E1 adds the parallel SDL_GPU engine path but does not migrate gameplay rendering.
- E1 ships a Vulkan/SPIR-V smoke shader path. Portable D3D12/Metal shader artifacts and the public batched renderer arrive in later MediaForge milestones without changing game data.
