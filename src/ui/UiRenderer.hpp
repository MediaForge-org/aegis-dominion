#pragma once

#include "Assets.hpp"

#include <SFML/Graphics.hpp>
#include <string>

namespace aegis::ui {

inline const sf::Color Background(7, 13, 21);
inline const sf::Color Panel(12, 21, 31, 242);
inline const sf::Color PanelRaised(18, 30, 43, 238);
inline const sf::Color Text(224, 235, 244);
inline const sf::Color Muted(137, 158, 176);
inline const sf::Color Cyan(91, 210, 255);
inline const sf::Color Green(92, 220, 143);
inline const sf::Color Red(255, 92, 107);
inline const sf::Color Gold(255, 197, 76);
inline const sf::Color Purple(203, 139, 255);
inline const sf::Color Orange(255, 140, 78);

class UiRenderer {
public:
    UiRenderer(sf::RenderWindow& window, Assets& assets) : window_(window), assets_(assets) {}

    void panel(const sf::FloatRect& rect, sf::Color fill, sf::Color outline = sf::Color::Transparent, float thickness = 1.f);
    void text(const std::string& value, unsigned size, sf::Vector2f position, sf::Color color = Text,
              bool bold = false, bool centered = false);
    void wrapped(const std::string& value, unsigned size, sf::FloatRect box, sf::Color color = Text,
                 float lineGap = 5.f, bool bold = false);
    bool button(const sf::FloatRect& rect, const std::string& label, sf::Color accent,
                bool active = false, unsigned size = 18);

private:
    sf::RenderWindow& window_;
    Assets& assets_;
};

} // namespace aegis::ui
