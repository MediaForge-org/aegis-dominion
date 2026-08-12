#pragma once

#include "core/GameLaunchConfig.hpp"
#include "input/InputSystem.hpp"

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

class Assets;

namespace aegis::app {

class ScreenManager;

struct ScreenContext {
    sf::RenderWindow& window;
    Assets& assets;
    ScreenManager& screens;
    input::InputSystem& input;
};

class Screen {
public:
    explicit Screen(ScreenContext context) : context_(context) {}
    virtual ~Screen() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaSeconds) = 0;
    virtual void render() = 0;
    virtual void onResume() {}

protected:
    ScreenContext context_;
};

enum class ScreenType { MainMenu, Game, MapForge };

struct ScreenRequest {
    ScreenRequest(ScreenType requestedType = ScreenType::MainMenu) : type(requestedType) {}
    ScreenType type = ScreenType::MainMenu;
    bool openMapSelect = false;
    std::optional<core::GameLaunchConfig> gameLaunch;
};

} // namespace aegis::app
