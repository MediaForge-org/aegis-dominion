#pragma once

#include "RenderContext.hpp"
#include "RenderSnapshot.hpp"

class Assets;

namespace aegis::render {

class WorldRenderer {
public:
    WorldRenderer(RenderContext& context, Assets& assets) : context_(context), assets_(assets) {}
    void draw(const WorldRenderSnapshot& snapshot);

private:
    void drawTerrain(const MapRenderSnapshot& map);
    void drawWater(const MapRenderSnapshot& map, float elapsed);
    void drawRoad(const MapRenderSnapshot& map);
    void drawEnvironment(const MapRenderSnapshot& map);
    void drawZones(const MapRenderSnapshot& map);
    void drawEndpoints(const MapRenderSnapshot& map, float elapsed);
    void drawEnemies(const std::vector<EnemyRenderSnapshot>& enemies);
    void drawTowers(const std::vector<TowerRenderSnapshot>& towers);
    void drawProjectiles(const std::vector<ProjectileRenderSnapshot>& projectiles);
    void drawLighting(const WorldRenderSnapshot& snapshot);

    RenderContext& context_;
    Assets& assets_;
};

} // namespace aegis::render
