#include "Application.hpp"

#include "screens/GameScreen.hpp"
#include "screens/MainMenuScreen.hpp"
#include "editor/MapForgeScreen.hpp"
#include "logging/Logger.hpp"

#include <algorithm>
#include <stdexcept>

namespace aegis::app {

Application::Application()
    : window_(sf::VideoMode(WINDOW_W, WINDOW_H), "AEGIS DOMINION — Tactical Tower Defense", sf::Style::Default),
      screens_([this](const ScreenRequest& request) { return createScreen(request); }) {
    window_.setFramerateLimit(120);
    window_.setVerticalSyncEnabled(true);
    if (!assets_.load("assets")) logging::log().warning("Some assets are missing; explicit fallbacks are active");
    screens_.replace({ScreenType::MainMenu});
    screens_.applyPending();
}

std::unique_ptr<Screen> Application::createScreen(const ScreenRequest& request) {
    ScreenContext context{window_, assets_, screens_, input_};
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
        input_.beginFrame();
        const float deltaSeconds = std::min(0.035f, clock.restart().asSeconds());
        sf::Event event{};
        while (window_.pollEvent(event)) {
            input_.handleEvent(event);
            if (event.type == sf::Event::Resized) {
                const float windowAspect = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
                const float referenceAspect = static_cast<float>(WINDOW_W) / static_cast<float>(WINDOW_H);
                sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
                if (windowAspect > referenceAspect) { viewport.width = referenceAspect / windowAspect; viewport.left = (1.f - viewport.width) * .5f; }
                else { viewport.height = windowAspect / referenceAspect; viewport.top = (1.f - viewport.height) * .5f; }
                sf::View view({0.f, 0.f, static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H)});
                view.setViewport(viewport);
                window_.setView(view);
            }
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
