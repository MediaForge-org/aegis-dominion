# Art direction

AEGIS DOMINION uses a restrained tactical sci-fi language: dark military materials, readable silhouettes, cyan system energy, amber warnings and damage-type accents. The battlefield stays lower-contrast than gameplay actors and the command UI. Glow communicates energy; it is not a universal outline.

Verdant uses deep mineral green and organic clusters, Frost uses pale blue-grey surfaces and sparse hard props, and Ash uses charcoal/brown volcanic material with ruin/scrap silhouettes. Roads remain neutral enough for every enemy silhouette. Spawn is a luminous deployment gate; the AEGIS Core is a heavier technical plinth with a separate energy element.

Current terrain, environment, spawn/core and panel files are polished intermediate assets with stable IDs. They are intentionally replaceable without renderer changes. Existing enemy/tower sprites remain transitional and should be replaced first with cohesive multi-frame authored sheets while preserving silhouette, origin and logical ID contracts.

## E2 source-art status

The E2 slice uses newly AI-authored high-resolution raster sources: composed Verdant battlefield/road, Spawn Gate, Core, separate Pulse/Railgun parts, Raider, Heavy Tank, grove/checkpoint, VFX and tactical HUD surfaces. These are not the old Python-generated 96–256 px placeholder set. No procedural raster-art generator produced E2 art; ImageMagick produced only the deterministic text overlay and generic shadow mask.

They are polished vertical-slice art, not claimed final production art. Limitations include imperfect atlas gutters/mattes, single poses, baked terrain/road composition, no normal maps, and baked overlay text. Artist-owned sources, animation sheets, separate emissive maps and runtime fonts should replace them through the manifest.
