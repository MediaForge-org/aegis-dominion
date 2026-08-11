# AEGIS DOMINION map format

## Status

The extension is `.aegismap`; the supported header remains `AEGIS_MAP_V1`. Phase 2 does not change V1 persistence semantics. Files are UTF-8 text. Unknown versions, unknown record types and malformed rows fail with an explanatory parser error.

Semantic validation is intentionally separate from parsing. This lets MAP FORGE reopen and repair an incomplete draft (for example, one without a goal). A parsed document must pass fatal validation before gameplay conversion.

## V1 records

```text
AEGIS_MAP_V1
meta "id" "name" "subtitle" "description" "biome" "author" "difficulty"
size width height
environment "weather" timeOfDay ambientIntensity terrainSeed
waves "preset_id"
path "path_id" nodeCount x0 y0 x1 y1 ...
spawn "spawn_id" "path_id" x y
goal "goal_id" "path_id" x y
zone buildable|blocked|water|decoration_only x y width height
deco "asset_id" x y rotationDegrees scale layer
```

Quoted values may contain spaces and German UTF-8 text. Coordinates are semantic map-world coordinates, not UI pixels. `size` defines that coordinate plane. The 2D gameplay adapter scales it into its current viewport; a future 3D renderer can consume the same world data.

## Identity and layers

Path, spawn and goal IDs are stable strings and must be unique within their object category. Spawn/goal `path_id` references must resolve. Rectangular zones are ordered map layers in V1 and are addressed by stable order during an editor session. `biome`, decoration `asset_id` and the wave preset are semantic IDs, not texture paths.

V1 already carries future-facing height-adjacent metadata through world dimensions, decoration layer/transform and terrain seed, but it does not persist per-point Z, height fields or terrain layers. Those additions require a compatible new record strategy or a version bump; MAP FORGE does not pretend they are available in Phase 2.

## Validation

Fatal errors include:

- missing map ID/name, path, spawn or goal;
- paths with fewer than two nodes;
- duplicate path/spawn/goal IDs;
- unresolved spawn/goal route references;
- non-finite or out-of-bounds points/endpoints/decorations;
- invalid/out-of-bounds zone rectangles;
- empty decoration asset IDs or invalid decoration transforms;
- unsupported declared format versions.

No explicit build zone is a warning, because free non-path space remains buildable. Playtest accepts warnings but rejects any fatal error. The current asset catalog has no renderer-independent lookup service, so non-empty decoration IDs cannot yet be verified against installed art.

## Compatibility rules

- V1 meanings must not change silently.
- Existing V1 files continue to load.
- Incompatible changes require a new header and explicit migration.
- Unsupported `metadata.formatVersion` values are never written as V1 implicitly.
