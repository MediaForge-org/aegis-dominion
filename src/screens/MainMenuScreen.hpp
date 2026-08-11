#pragma once

#include "app/Screen.hpp"
#include "Map.hpp"
#include "ui/UiRenderer.hpp"

#include <vector>

namespace aegis::screens {

class MainMenuScreen final : public app::Screen {
public:
    explicit MainMenuScreen(app::ScreenContext context, bool openMapSelect = false);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render() override;

private:
    enum class Page { Menu, MapSelect, Tutorial };
    struct Particle { sf::Vector2f position; sf::Vector2f velocity; float radius; sf::Color color; };

    void click(sf::Vector2f position);
    void drawMenu();
    void drawMapSelect();
    void drawTutorial();

    ui::UiRenderer ui_;
    GameMap map_;
    Page page_ = Page::Menu;
    int tutorialPage_ = 0;
    std::vector<Particle> particles_;
};

} // namespace aegis::screens
