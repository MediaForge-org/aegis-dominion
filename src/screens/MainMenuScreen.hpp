#pragma once

#include "app/Screen.hpp"
#include "Map.hpp"
#include "ui/UiRenderer.hpp"
#include "animation/Tween.hpp"
#include "core/MapCatalog.hpp"

#include <vector>

namespace aegis::screens {

class MainMenuScreen final : public app::Screen {
public:
    explicit MainMenuScreen(app::ScreenContext context, bool openMapSelect = false);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render() override;
    void onResume() override { context_.input.setContext(input::Context::Menu); }

private:
    enum class Page { Menu, MapSelect, Tutorial };
    struct Particle { sf::Vector2f position; sf::Vector2f velocity; float radius; sf::Color color; };

    void click(sf::Vector2f position);
    void drawMenu();
    void drawMapSelect();
    void drawTutorial();

    ui::UiRenderer ui_;
    GameMap map_;
    std::vector<core::MapCatalogEntry> mapCatalog_;
    Page page_ = Page::Menu;
    int tutorialPage_ = 0;
    std::vector<Particle> particles_;
    animation::Tween pageAppear_{0.f, 1.f, .28f, animation::Easing::SmoothStep};
};

} // namespace aegis::screens
