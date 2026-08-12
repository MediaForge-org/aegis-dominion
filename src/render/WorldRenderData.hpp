#pragma once

#include "RenderSnapshot.hpp"

#include <string_view>

namespace aegis::render {

enum class TerrainMaterial { Grass, Dirt, Rock, Snow, Ice, Sand, Ash, Industrial };

TerrainMaterial terrainMaterialForBiome(std::string_view biome);
const char* terrainTextureId(TerrainMaterial material);
bool zoneVisibleInGameplay(core::ZoneType type, bool buildMode, bool debugOverlay);
std::vector<DecorationRenderSnapshot> generateProceduralDecorations(const MapRenderSnapshot& map, std::size_t maximum = 72);

} // namespace aegis::render
