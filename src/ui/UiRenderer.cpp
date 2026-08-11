#include "UiRenderer.hpp"

#include "Common.hpp"

#include <sstream>

namespace aegis::ui {

void UiRenderer::panel(const sf::FloatRect& rect, sf::Color fill, sf::Color outline, float thickness) {
    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition(rect.left, rect.top);
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(thickness);
    window_.draw(shape);
}

void UiRenderer::text(const std::string& value, unsigned size, sf::Vector2f position, sf::Color color, bool bold, bool centered) {
    if (!assets_.text().loaded()) return;
    auto drawable = assets_.text().makeText(value, size);
    drawable.setFillColor(color);
    if (bold) drawable.setStyle(sf::Text::Bold);
    drawable.setOutlineColor(sf::Color(0, 0, 0, 100));
    drawable.setOutlineThickness(size >= 28 ? 1.5f : 0.f);
    if (centered) {
        const auto bounds = drawable.getLocalBounds();
        drawable.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    }
    drawable.setPosition(position);
    window_.draw(drawable);
}

void UiRenderer::wrapped(const std::string& value, unsigned size, sf::FloatRect box, sf::Color color, float lineGap, bool bold) {
    if (!assets_.text().loaded()) return;
    std::istringstream words(value);
    std::string word;
    std::string line;
    float y = box.top;
    while (words >> word) {
        const auto candidate = line.empty() ? word : line + " " + word;
        const auto measure = assets_.text().makeText(candidate, size);
        if (measure.getLocalBounds().width > box.width && !line.empty()) {
            text(line, size, {box.left, y}, color, bold);
            y += static_cast<float>(size) + lineGap;
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) text(line, size, {box.left, y}, color, bold);
}

bool UiRenderer::button(const sf::FloatRect& rect, const std::string& label, sf::Color accent, bool active, unsigned size) {
    const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    const bool hovered = rect.contains(mouse);
    panel(rect, hovered ? sf::Color(26, 42, 57, 248) : sf::Color(17, 29, 41, 245),
          active ? accent : withAlpha(accent, hovered ? 220 : 120), active ? 2.5f : 1.5f);
    sf::RectangleShape bar({4.f, rect.height - 12.f});
    bar.setPosition(rect.left + 6.f, rect.top + 6.f);
    bar.setFillColor(withAlpha(accent, hovered ? 255 : 180));
    window_.draw(bar);
    text(label, size, {rect.left + rect.width / 2.f + 4.f, rect.top + rect.height / 2.f - 1.f},
         hovered ? sf::Color::White : Text, true, true);
    return hovered;
}

} // namespace aegis::ui
