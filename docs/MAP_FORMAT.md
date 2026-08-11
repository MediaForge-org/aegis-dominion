# AEGIS DOMINION map format

## Status

The current extension remains `.aegismap` and the only supported header is `AEGIS_MAP_V1`. Files are UTF-8 text. Unknown versions, unknown record types, malformed rows and structurally invalid documents fail with an explanatory error.

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

Quoted values may contain spaces and German UTF-8 text. Coordinates are semantic world coordinates, not UI positions. The current renderer treats them as a 2D plane, while future versions may add elevation and full transforms.

## Compatibility rules

- V1 semantics must not change silently.
- Additive records need a parser strategy for older readers; the current strict V1 parser rejects unknown records.
- Any incompatible change requires a new header and an explicit migration path.
- `MapDocument::metadata.formatVersion` must match the writer version; unsupported versions are not downgraded implicitly.

## Current validation

Validation checks required metadata, map dimensions, paths and route references; point, spawn, goal, zone and decoration bounds; positive transformations; and duplicate path IDs. Gameplay checks such as route-to-goal connectivity, build-area sufficiency, wave validity and asset-catalog lookup belong to later validation layers.
