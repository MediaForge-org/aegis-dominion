#include "Map.hpp"
#include "core/GameLaunchConfig.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }
bool near(float actual, float expected) { return std::abs(actual - expected) < .01f; }

aegis::core::PlayableMap sentinelMap() {
    aegis::core::PlayableMap map;
    map.id = "playtest_sentinel_id"; map.name = "PLAYTEST_SENTINEL_MAP"; map.biome = "sentinel";
    map.width = 1733.f; map.height = 977.f; map.routeId = "sentinel_route";
    map.route = {{37.f, 83.f}, {419.f, 731.f}, {1691.f, 901.f}};
    map.spawn = {37.f, 83.f}; map.goal = {1691.f, 901.f};
    map.zones.push_back({aegis::core::ZoneType::Buildable, {111.f, 444.f, 222.f, 123.f}});
    map.zones.push_back({aegis::core::ZoneType::Blocked, {777.f, 55.f, 91.f, 203.f}});
    map.zones.push_back({aegis::core::ZoneType::Water, {1401.f, 501.f, 177.f, 311.f}});
    return map;
}
}

int main() {
    try {
        GameMap gameMap;
        const auto source = sentinelMap();
        gameMap.loadFromPlayableMap(source);
        const auto& projected = gameMap.data();
        const float scaleX = WORLD_W / source.width;
        const float scaleY = WORLD_H / source.height;
        require(projected.id == source.id && projected.name == source.name, "projected identity changed");
        require(projected.sourceWidth == 1733.f && projected.sourceHeight == 977.f, "source dimensions lost");
        require(!projected.authoredBackground && gameMap.index() == -1, "custom map selected a built-in visual map");
        require(projected.path.size() == source.route.size() && near(projected.path[1].x, 419.f * scaleX) && near(projected.path[1].y, 731.f * scaleY), "path projection differs");
        require(near(projected.spawn.x, 37.f * scaleX) && near(projected.spawn.y, 83.f * scaleY), "spawn projection differs");
        require(near(projected.goal.x, 1691.f * scaleX) && near(projected.goal.y, 901.f * scaleY), "goal projection differs");
        require(projected.zones.size() == 3 && near(projected.zones[0].rect.left, 111.f * scaleX) && near(projected.zones[0].rect.height, 123.f * scaleY), "zone projection differs");
        require(projected.zones[0].type == aegis::core::ZoneType::Buildable && projected.zones[1].type == aegis::core::ZoneType::Blocked && projected.zones[2].type == aegis::core::ZoneType::Water, "zone types changed");
        require(gameMap.canBuild({85.f, 500.f}, {}), "projected build zone is not buildable");
        require(!gameMap.canBuild({500.f, 500.f}, {}), "custom map allowed building outside explicit build zones");
        require(!gameMap.canBuild({570.f, 130.f}, {}), "projected blocked zone allows building");
        require(!gameMap.canBuild({1030.f, 600.f}, {}), "projected water zone allows building");
        std::cout << "12 GameMap projection/identity/mechanics checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
