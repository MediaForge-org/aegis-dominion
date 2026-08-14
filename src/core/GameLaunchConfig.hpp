#pragma once

#include "PlayableMap.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace aegis::core {

struct StandardGameLaunch {
    PlayableMap map;
};

struct MapForgePlaytestLaunch {
    PlayableMap map;
};

using GameLaunchConfig = std::variant<StandardGameLaunch, MapForgePlaytestLaunch>;

std::optional<GameLaunchConfig> makeMapForgePlaytestLaunch(const MapDocument& document, std::string* error = nullptr);

class GameSession {
public:
    explicit GameSession(GameLaunchConfig launch) : launch_(std::move(launch)) {}

    [[nodiscard]] const GameLaunchConfig& launch() const noexcept { return launch_; }
    [[nodiscard]] const PlayableMap& map() const noexcept;
    [[nodiscard]] bool isMapForgePlaytest() const noexcept;

private:
    GameLaunchConfig launch_;
};

} // namespace aegis::core
