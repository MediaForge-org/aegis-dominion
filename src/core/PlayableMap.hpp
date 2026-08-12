#pragma once

#include "MapDocument.hpp"

#include <optional>
#include <string>
#include <vector>

namespace aegis::core {

struct PlayableMap {
    std::string id;
    std::string name;
    std::string subtitle;
    std::string description;
    std::string biome;
    float width = 1200.f;
    float height = 900.f;
    std::string routeId;
    std::vector<Vec2> route;
    std::vector<Zone> zones;
    std::vector<Decoration> decorations;
    Vec2 spawn;
    Vec2 goal;
    EnvironmentSettings environment;
};

std::optional<PlayableMap> buildPlayableMap(const MapDocument& document, std::string* error = nullptr);

} // namespace aegis::core
