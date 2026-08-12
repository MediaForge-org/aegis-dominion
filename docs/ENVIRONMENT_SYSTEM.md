# Environment system

Stable logical IDs currently cover Tree, Bush, RockSmall, RockLarge, Crate, Ruin, Lamp, Antenna, Scrap and Barricade. File access is exclusively through `AssetManager`; snapshots store only the logical ID and transform.

Authored `MapDocument::decorations` survive playtest conversion and are projected by `GameMap`. If none exist, `generateProceduralDecorations()` chooses a biome-specific palette using map ID plus `terrainSeed`. It rejects candidates near the sampled road, spawn, AEGIS Core, water, blocked areas and world edges. The same seed produces equivalent placement records; decorative results never change build/collision rules.

The generated PNGs are replaceable intermediate art. MAP FORGE decoration placement is not active yet; once added it must mutate `MapDocument` through `MapEditorModel`, after which authored objects automatically suppress procedural generation.
