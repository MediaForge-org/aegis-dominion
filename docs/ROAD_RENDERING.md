# Road rendering

MAP FORGE stores logical route nodes only. `core::samplePathCurve()` deterministically samples endpoint-clamped Catmull-Rom segments at a target spacing. `GameMap` projects nodes, samples once and uses the resulting points for enemy motion, normalized progress, tower clearance and rendering. This prevents the visual road and simulation from diverging.

`WorldRenderer` builds triangle strips from averaged curve normals. Passes are shadow, biome shoulder, dark base and surface, followed by deterministic low-alpha markings/wear. Curves therefore have continuous joins instead of overlapping rectangles or polygon zigzags. Material selection currently changes shoulders; the structure supports asphalt, dirt, snow, industrial and energy profiles without changing map paths.

Logical nodes remain visible/editable in MAP FORGE, while its preview connects the sampled curve. No smoothed samples are serialized, so V1 files remain stable and future renderers can choose their own tessellation density.
