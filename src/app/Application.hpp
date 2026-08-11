#pragma once

#include "Assets.hpp"
#include "ScreenManager.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

namespace aegis::app {

class Application {
public:
    Application();
    int run();

private:
    std::unique_ptr<Screen> createScreen(const ScreenRequest& request);

    sf::RenderWindow window_;
    Assets assets_;
    ScreenManager screens_;
};

} // namespace aegis::app
