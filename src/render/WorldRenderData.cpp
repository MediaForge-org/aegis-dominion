#include "WorldRenderData.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace aegis::render {
namespace {

float distanceToSegment(RenderVec2 p, RenderVec2 a, RenderVec2 b) {
    const float dx = b.x - a.x, dy = b.y - a.y;
    const float denominator = dx * dx + dy * dy;
    if (denominator < .001f) return std::hypot(p.x - a.x, p.y - a.y);
    const float t = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / denominator, 0.f, 1.f);
    return std::hypot(p.x - (a.x + dx * t), p.y - (a.y + dy * t));
}

bool safePosition(const MapRenderSnapshot& map, RenderVec2 point) {
    if (point.x < 36.f || point.y < 36.f || point.x > 1164.f || point.y > 864.f) return false;
    if (std::hypot(point.x - map.spawn.x, point.y - map.spawn.y) < 120.f ||
        std::hypot(point.x - map.goal.x, point.y - map.goal.y) < 135.f) return false;
    for (std::size_t i = 0; i + 1 < map.path.size(); ++i)
        if (distanceToSegment(point, map.path[i], map.path[i + 1]) < 78.f) return false;
    for (const auto& zone : map.zones)
        if ((zone.type == core::ZoneType::Water || zone.type == core::ZoneType::Blocked) &&
            point.x >= zone.rect.x && point.y >= zone.rect.y && point.x <= zone.rect.x + zone.rect.width && point.y <= zone.rect.y + zone.rect.height)
            return false;
    return true;
}

} // namespace

TerrainMaterial terrainMaterialForBiome(std::string_view biome) {
    if (biome == "frost" || biome == "snow") return TerrainMaterial::Snow;
    if (biome == "ice") return TerrainMaterial::Ice;
    if (biome == "ember" || biome == "ash" || biome == "volcanic") return TerrainMaterial::Ash;
    if (biome == "dirt") return TerrainMaterial::Dirt;
    if (biome == "rock") return TerrainMaterial::Rock;
    if (biome == "sand") return TerrainMaterial::Sand;
    if (biome == "industrial") return TerrainMaterial::Industrial;
    return TerrainMaterial::Grass;
}

const char* terrainTextureId(TerrainMaterial material) {
    switch (material) {
        case TerrainMaterial::Grass: return "terrain.grass.surface";
        case TerrainMaterial::Dirt: return "terrain.dirt.surface";
        case TerrainMaterial::Rock: return "terrain.rock.surface";
        case TerrainMaterial::Snow: return "terrain.snow.surface";
        case TerrainMaterial::Ice: return "terrain.ice.surface";
        case TerrainMaterial::Sand: return "terrain.sand.surface";
        case TerrainMaterial::Ash: return "terrain.ash.surface";
        case TerrainMaterial::Industrial: return "terrain.industrial.surface";
    }
    return "terrain.grass.surface";
}

bool zoneVisibleInGameplay(core::ZoneType type, bool buildMode, bool debugOverlay) {
    if (type == core::ZoneType::Water) return true;
    if (debugOverlay) return true;
    return buildMode && type == core::ZoneType::Buildable;
}

std::vector<DecorationRenderSnapshot> generateProceduralDecorations(const MapRenderSnapshot& map, std::size_t maximum) {
    std::vector<DecorationRenderSnapshot> result = map.decorations;
    if (!map.decorations.empty() || maximum == 0) return result;
    const auto material = terrainMaterialForBiome(map.biome);
    const std::array grassIds{"environment.tree", "environment.bush", "environment.rock_small", "environment.rock_large", "environment.crate", "environment.antenna"};
    const std::array frostIds{"environment.rock_small", "environment.rock_large", "environment.ruin", "environment.antenna", "environment.barricade", "environment.crate"};
    const std::array ashIds{"environment.rock_large", "environment.scrap", "environment.ruin", "environment.lamp", "environment.barricade", "environment.rock_small"};
    const auto& ids = material == TerrainMaterial::Grass ? grassIds : (material == TerrainMaterial::Snow || material == TerrainMaterial::Ice ? frostIds : ashIds);
    std::seed_seq sequence{map.terrainSeed, static_cast<unsigned>(std::hash<std::string>{}(map.id)), 0xA3E615u};
    std::mt19937 random(sequence);
    std::uniform_real_distribution<float> x(28.f, 1172.f), y(30.f, 870.f), rotation(0.f, 360.f), scale(.62f, 1.18f);
    std::uniform_int_distribution<std::size_t> pick(0, ids.size() - 1);
    for (std::size_t attempt = 0; attempt < maximum * 12 && result.size() < maximum; ++attempt) {
        const RenderVec2 position{x(random), y(random)};
        if (!safePosition(map, position)) continue;
        const auto selected = pick(random);
        result.push_back({ids[selected], position, rotation(random), scale(random), static_cast<int>(selected), Layer::Environment});
    }
    return result;
}

} // namespace aegis::render
