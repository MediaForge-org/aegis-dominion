#include "GameLaunchConfig.hpp"

#include <utility>

namespace aegis::core {

std::optional<GameLaunchConfig> makeMapForgePlaytestLaunch(const MapDocument& document, std::string* error) {
    auto playable = buildPlayableMap(document, error);
    if (!playable) return std::nullopt;
    return GameLaunchConfig{MapForgePlaytestLaunch{std::move(*playable)}};
}

const PlayableMap& GameSession::map() const noexcept {
    return std::visit([](const auto& launch) -> const PlayableMap& { return launch.map; }, launch_);
}

bool GameSession::isMapForgePlaytest() const noexcept {
    return std::holds_alternative<MapForgePlaytestLaunch>(launch_);
}

} // namespace aegis::core
