# AEGIS DOMINION architecture

## Runtime composition

The Phase 2 runtime is screen-based:

```text
Application
  ├─ RenderWindow + shared Assets/TextService
  └─ ScreenManager (queued stack navigation)
       ├─ MainMenuScreen (menu, map selection, tutorial)
       ├─ GameScreen (match orchestration and current SFML gameplay view)
       └─ MapForgeScreen (editor UI and interaction)
```

`Application` owns only the window lifecycle, event loop, common services and screen dispatch. Navigation requests are queued so a screen is never destroyed while handling its own event. The stack is important for playtesting: MAP FORGE pushes a `GameScreen`; popping that screen exposes the same editor instance, including camera, selection, tool, undo/redo and unsaved document.

The request/factory boundary is ready to add `MapSelectScreen`, `SettingsScreen`, `TutorialScreen`, `CodexScreen`, `ArsenalScreen` and `ResultsScreen` without reintroducing a central state switch. Map selection and tutorial still live as bounded pages inside `MainMenuScreen` in this phase.

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
- `GameScreen` still coordinates current prototype simulation and rendering. Entity simulation/render separation remains later work.

## Validation and playtest flow

`MapDocument::validateDetailed()` returns severity plus object identity/index. MAP FORGE shows errors and warnings and can select referenced path nodes, endpoints and zones. Fatal errors block playtest; warnings do not.

`buildPlayableMap()` selects the first route that has both a spawn and goal, copies route/endpoints/zones and never mutates the source document. `makeMapForgePlaytestLaunch()` wraps that complete value in the `GameLaunchConfig` variant. A normal launch contains `StandardGameLaunch{mapIndex}`; a MAP FORGE launch contains `MapForgePlaytestLaunch{PlayableMap}`. The alternatives cannot be confused through a boolean or missing optional document.

`GameMap::loadFromPlayableMap()` consumes the playtest value directly and never calls the standard-map loader. Its SFML-facing adapter applies one deterministic projection from document world space into the current 1200×900 gameplay plane:

```text
gameX = worldX × 1200 / mapWidth
gameY = worldY ×  900 / mapHeight
```

The same scale pair is applied to path nodes, spawn, goal and every zone rectangle. Original dimensions remain in session metadata and are displayed in the playtest HUD. This projection is isolated in the current 2D adapter; `MapDocument` and `PlayableMap` retain semantic world coordinates for future renderers.

Built-in games retain their authored background PNG. MAP FORGE playtests have no built-in visual index: `GameScreen` renders their terrain surface, route, spawn, goal and build/blocked/water overlays from projected session data. Thus simulation and visible geometry share one `GameMap` instance.

## Toolchain boundary

AEGIS DOMINION uses C++23 for every project library, executable and test. `AEGIS_CXX_STANDARD` is the single selector, and the `aegis_project_options` INTERFACE target propagates `cxx_std_23` plus the common warning policy. GNU language extensions are disabled globally. CMake rejects values other than 23/26 and rejects a requested standard that the selected compiler does not advertise.

C++26 is not the current project standard. The build structure merely reserves `-DAEGIS_CXX_STANDARD=26` as a future toolchain experiment without requiring runtime-source or per-target CMake edits. See `BUILDING.md`.

## Text boundary

Repository/map strings are UTF-8 `std::string`. `ui::TextService` converts them using `sf::String::fromUtf8`. MAP FORGE text entry encodes SFML Unicode events back to UTF-8, including German umlauts and ß.

## Remaining architecture debt

- Gameplay entities still use SFML vectors and draw themselves.
- Current gameplay supports one active enemy route per match; maps may store several routes and MAP FORGE edits all of them.
- Map selection/tutorial are menu pages rather than independent screens, although navigation supports extracting them.
- The inspector uses focused fields and bounded controls; a reusable retained widget/form system belongs to a later UI phase.
- Terrain, elevation, decorations and environment authoring are deliberately not active tools yet.
- Waves, economy and balance remain hard-coded.
