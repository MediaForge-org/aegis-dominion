#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace aegis::ui {

enum class ElementState { Normal, Hover, Pressed, Selected, Disabled, Focused };

class ButtonInteraction {
public:
    explicit ButtonInteraction(sf::FloatRect bounds = {}) : bounds_(bounds) {}
    void setBounds(sf::FloatRect bounds) { bounds_ = bounds; }
    void setEnabled(bool enabled) { enabled_ = enabled; if (!enabled_) armed_ = false; }
    void setSelected(bool selected) { selected_ = selected; }
    void setFocused(bool focused) { focused_ = focused; }
    void pointerMove(sf::Vector2f position) { hovered_ = bounds_.contains(position); }
    void pointerPress(sf::Vector2f position);
    bool pointerRelease(sf::Vector2f position);
    ElementState state() const;

private:
    sf::FloatRect bounds_;
    bool enabled_ = true, selected_ = false, focused_ = false, hovered_ = false, armed_ = false;
};

} // namespace aegis::ui
