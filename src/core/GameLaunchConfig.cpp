#include "GameLaunchConfig.hpp"

#include <utility>

namespace aegis::core {

std::optional<GameLaunchConfig> makeMapForgePlaytestLaunch(const MapDocument& document, std::string* error) {
    auto playable = buildPlayableMap(document, error);
    if (!playable) return std::nullopt;
    return GameLaunchConfig{MapForgePlaytestLaunch{std::move(*playable)}};
}

} // namespace aegis::core
