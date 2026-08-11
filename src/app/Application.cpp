#include "Application.hpp"

#include "screens/GameScreen.hpp"
#include "screens/MainMenuScreen.hpp"
#include "editor/MapForgeScreen.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace aegis::app {

Application::Application()
    : window_(sf::VideoMode(WINDOW_W, WINDOW_H), "AEGIS DOMINION — Tactical Tower Defense", sf::Style::Titlebar | sf::Style::Close),
      screens_([this](const ScreenRequest& request) { return createScreen(request); }) {
    window_.setFramerateLimit(120);
    window_.setVerticalSyncEnabled(true);
    if (!assets_.load("assets")) std::cerr << "Einige Assets konnten nicht geladen werden. Starte das Spiel aus dem Build-Ordner.\n";
    screens_.replace({ScreenType::MainMenu});
    screens_.applyPending();
}

std::unique_ptr<Screen> Application::createScreen(const ScreenRequest& request) {
    ScreenContext context{window_, assets_, screens_};
    switch (request.type) {
        case ScreenType::MainMenu: return std::make_unique<screens::MainMenuScreen>(context, request.openMapSelect);
        case ScreenType::MapForge: return std::make_unique<editor::MapForgeScreen>(context);
        case ScreenType::Game:
            if (!request.gameLaunch) throw std::runtime_error("GameScreen benötigt eine explizite Launch-Konfiguration");
            return std::make_unique<screens::GameScreen>(context, *request.gameLaunch);
    }
    throw std::runtime_error("Unbekannter Screen-Typ");
}

int Application::run() {
    sf::Clock clock;
    while (window_.isOpen() && !screens_.quitRequested()) {
        const float deltaSeconds = std::min(0.035f, clock.restart().asSeconds());
        sf::Event event{};
        while (window_.pollEvent(event)) {
            if (auto* screen = screens_.current()) screen->handleEvent(event);
            screens_.applyPending();
        }
        if (auto* screen = screens_.current()) screen->update(deltaSeconds);
        screens_.applyPending();
        if (!screens_.current()) break;
        window_.clear(sf::Color(7, 13, 21));
        screens_.current()->render();
        window_.display();
        screens_.applyPending();
    }
    window_.close();
    return 0;
}

} // namespace aegis::app
