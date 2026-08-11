#pragma once

#include "PlayableMap.hpp"

#include <optional>
#include <string>
#include <variant>

namespace aegis::core {

struct StandardGameLaunch {
    int mapIndex = 0;
};

struct MapForgePlaytestLaunch {
    PlayableMap map;
};

using GameLaunchConfig = std::variant<StandardGameLaunch, MapForgePlaytestLaunch>;

std::optional<GameLaunchConfig> makeMapForgePlaytestLaunch(const MapDocument& document, std::string* error = nullptr);

} // namespace aegis::core
