#include "Interaction.hpp"

namespace aegis::ui {

void ButtonInteraction::pointerPress(sf::Vector2f position) {
    hovered_ = bounds_.contains(position);
    armed_ = enabled_ && hovered_;
}

bool ButtonInteraction::pointerRelease(sf::Vector2f position) {
    hovered_ = bounds_.contains(position);
    const bool clicked = enabled_ && armed_ && hovered_;
    armed_ = false;
    return clicked;
}

ElementState ButtonInteraction::state() const {
    if (!enabled_) return ElementState::Disabled;
    if (armed_) return ElementState::Pressed;
    if (selected_) return ElementState::Selected;
    if (focused_) return ElementState::Focused;
    if (hovered_) return ElementState::Hover;
    return ElementState::Normal;
}

} // namespace aegis::ui
