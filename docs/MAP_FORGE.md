# MAP FORGE

MAP FORGE is available from the main menu and edits `.aegismap` documents directly. Phase 2 is a functional vertical slice; terrain painting, height sculpting and decoration placement are shown only as inactive future tools.

## Layout

- Top: file/actions, history, grid/snap, camera fit and **KARTE TESTEN**.
- Left: active editing tools.
- Center: zoomable map workspace.
- Right: contextual inspector and live validation.
- Bottom: tool, world coordinate, zoom, grid/snap, save state and status message.

## Controls

| Input | Action |
|---|---|
| Left click | Use active tool / select / begin drag |
| Middle mouse or right-drag | Pan camera |
| Mouse wheel | Zoom around mouse position |
| `F` | Fit/reset map view |
| `1`–`8` | Select tool |
| `G` | Toggle grid |
| `H` | Toggle snapping |
| Arrow keys | Move selected point, spawn, goal or zone by snap/grid step |
| `Delete` | Delete selection |
| `Ctrl+Z` / `Ctrl+Y` | Undo / redo |
| `Ctrl+N` / `Ctrl+O` | New / open |
| `Ctrl+S` / `Ctrl+Shift+S` | Save / save as |
| `Escape` | Cancel field/selection, then leave with dirty warning |

Inspector text fields commit with `Enter` and cancel with `Escape`. Unicode input is stored as UTF-8.

## Tools

- **SELECT:** selects path lines/nodes, spawns, goals and zones. Drag or use arrow keys to move; `Delete` removes.
- **PATH:** click to append nodes to the active path. Drag existing nodes. `Ctrl`-click a segment to insert between nodes. Select a path/node to expose **NEUER PFAD** and **PFAD LÖSCHEN** in the inspector.
- **SPAWN / GOAL:** each click creates another stable-ID endpoint for the active route. Endpoints remain selectable, movable and deletable.
- **BUILD ZONE / BLOCKED ZONE / WATER:** left-drag creates a rectangle. Select it to move, resize and cycle its type in the inspector.
- **ERASER:** click any supported object to delete it through editor history.

Path visuals distinguish start, intermediate and end nodes. Spawn (`S`) and goal (`G`) markers use different colors. Zone colors remain editor overlays and are not baked into gameplay data.

## Save/load and dirty state

The built-in browser lists every `.aegismap` in `maps/`; no single filename is hard-coded. **SPEICHERN UNTER** accepts a filename and enforces the extension. New/open/leave/close operations detect unsaved revisions and offer save, discard or cancel. Incomplete maps can be saved and reopened for repair because syntax parsing and gameplay validation are separate.

## Validation

The right panel updates from `MapDocument::validateDetailed()`. Errors are fatal; warnings are advisory. Clicking a listed issue selects its referenced path/node, spawn, goal or zone when available. Validation includes required objects, route length/references, stable-ID duplicates, bounds, zones and decorations. Asset-catalog existence checks await the typed asset service.

## Playtest flow

**KARTE TESTEN** validates the current in-memory document; saving is not required. A renderer-independent converter copies the first route with both spawn and goal into a `MapForgePlaytestLaunch`. `GameScreen` receives this explicit launch value—never a map index or filename—and `GameMap` consumes its `PlayableMap` directly without standard-map fallback.

The current 2D adapter maps world coordinates to the 1200×900 gameplay plane with independent deterministic X/Y scales (`1200 / mapWidth`, `900 / mapHeight`). Path, spawn, goal and all zone rectangles use exactly those scales. Custom terrain, road and zone overlays are rendered from the same projected data used by simulation; built-in background PNGs are used only by normal game launches.

The screen manager pushes `GameScreen` above MAP FORGE. Use the pause/end overlay's **ZURÜCK ZU MAP FORGE** action to pop gameplay and reveal the exact same editor screen—document, dirty state, selection, tool, camera and history are preserved.

The current gameplay engine follows one route per match and scales arbitrary map dimensions to its 1200×900 world viewport. Blocked and water zones prevent tower placement. Multiple routes remain stored/editable for the future multi-route simulation.

## Known Phase 2 limits

- Terrain, height and decoration authoring are not active.
- No rotation, scale, duplicate or multi-selection yet.
- Map selection/tutorial remain sub-pages of `MainMenuScreen`.
- The inspector has focused Phase 2 fields/steppers rather than a general form widget library.
- There is no native OS file dialog; the project-local `maps/` browser is intentional.
- GUI automation is not available in the headless development environment; final interaction checks are manual on Fedora/X11.
