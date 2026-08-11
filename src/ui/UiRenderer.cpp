#include "UiRenderer.hpp"

#include "Common.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace aegis::ui {
namespace {

void segment(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to, float thickness, sf::Color color) {
    const auto delta = to - from;
    const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length <= 0.f) return;
    sf::RectangleShape line({length, thickness});
    line.setOrigin(0.f, thickness / 2.f);
    line.setPosition(from);
    line.setRotation(std::atan2(delta.y, delta.x) * 180.f / PI_F);
    line.setFillColor(color);
    window.draw(line);
}

void triangle(sf::RenderWindow& window, const std::array<sf::Vector2f, 3>& points, sf::Color color) {
    sf::ConvexShape shape(3);
    for (std::size_t i = 0; i < points.size(); ++i) shape.setPoint(i, points[i]);
    shape.setFillColor(color);
    window.draw(shape);
}

} // namespace

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

bool UiRenderer::buttonSurface(const sf::FloatRect& rect, sf::Color accent, bool active) {
    const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    const bool hovered = rect.contains(mouse);
    panel(rect, hovered ? sf::Color(26, 42, 57, 248) : sf::Color(17, 29, 41, 245),
          active ? accent : withAlpha(accent, hovered ? 220 : 120), active ? 2.5f : 1.5f);
    sf::RectangleShape bar({4.f, rect.height - 12.f});
    bar.setPosition(rect.left + 6.f, rect.top + 6.f);
    bar.setFillColor(withAlpha(accent, hovered ? 255 : 180));
    window_.draw(bar);
    return hovered;
}

bool UiRenderer::button(const sf::FloatRect& rect, const std::string& label, sf::Color accent, bool active, unsigned size) {
    const bool hovered = buttonSurface(rect, accent, active);
    text(label, size, {rect.left + rect.width / 2.f + 4.f, rect.top + rect.height / 2.f - 1.f},
         hovered ? sf::Color::White : Text, true, true);
    return hovered;
}

bool UiRenderer::iconButton(const sf::FloatRect& rect, const std::string& label, UiIcon iconValue,
                            IconPlacement placement, sf::Color accent, bool active, unsigned size) {
    const bool hovered = buttonSurface(rect, accent, active);
    const auto color = hovered ? sf::Color::White : Text;
    const float iconSize = std::min(18.f, rect.height * .38f);
    sf::Vector2f iconCenter{rect.left + rect.width / 2.f + 4.f, rect.top + rect.height / 2.f};
    sf::Vector2f labelCenter{rect.left + rect.width / 2.f + 4.f, rect.top + rect.height / 2.f - 1.f};
    if (placement == IconPlacement::Left) {
        iconCenter.x = rect.left + 21.f;
        labelCenter.x += 9.f;
    } else if (placement == IconPlacement::Right) {
        iconCenter.x = rect.left + rect.width - 17.f;
        labelCenter.x -= 9.f;
    }
    icon(iconValue, iconCenter, iconSize, color);
    if (!label.empty()) text(label, size, labelCenter, color, true, true);
    return hovered;
}

void UiRenderer::icon(UiIcon iconValue, sf::Vector2f center, float size, sf::Color color) {
    const float half = size / 2.f;
    const float thickness = std::max(2.f, size * .14f);
    switch (iconValue) {
        case UiIcon::ArrowLeft:
            segment(window_, {center.x - half * .55f, center.y}, {center.x + half * .7f, center.y}, thickness, color);
            triangle(window_, {{{center.x - half, center.y}, {center.x - half * .25f, center.y - half * .7f},
                                {center.x - half * .25f, center.y + half * .7f}}}, color);
            break;
        case UiIcon::ArrowRight:
            segment(window_, {center.x - half * .7f, center.y}, {center.x + half * .55f, center.y}, thickness, color);
            triangle(window_, {{{center.x + half, center.y}, {center.x + half * .25f, center.y - half * .7f},
                                {center.x + half * .25f, center.y + half * .7f}}}, color);
            break;
        case UiIcon::ArrowUp:
            segment(window_, {center.x, center.y + half * .7f}, {center.x, center.y - half * .55f}, thickness, color);
            triangle(window_, {{{center.x, center.y - half}, {center.x - half * .7f, center.y - half * .25f},
                                {center.x + half * .7f, center.y - half * .25f}}}, color);
            break;
        case UiIcon::ArrowDown:
            segment(window_, {center.x, center.y - half * .7f}, {center.x, center.y + half * .55f}, thickness, color);
            triangle(window_, {{{center.x, center.y + half}, {center.x - half * .7f, center.y + half * .25f},
                                {center.x + half * .7f, center.y + half * .25f}}}, color);
            break;
        case UiIcon::Undo:
        case UiIcon::Redo: {
            const float direction = iconValue == UiIcon::Undo ? -1.f : 1.f;
            const std::array<sf::Vector2f, 5> points = {{
                {center.x + direction * half * .55f, center.y + half * .25f},
                {center.x + direction * half * .55f, center.y - half * .2f},
                {center.x + direction * half * .15f, center.y - half * .65f},
                {center.x - direction * half * .45f, center.y - half * .5f},
                {center.x - direction * half * .65f, center.y + half * .15f}}};
            for (std::size_t i = 0; i + 1 < points.size(); ++i) segment(window_, points[i], points[i + 1], thickness, color);
            triangle(window_, {{{center.x + direction * half, center.y + half * .25f},
                                {center.x + direction * half * .35f, center.y - half * .25f},
                                {center.x + direction * half * .3f, center.y + half * .7f}}}, color);
            break;
        }
        case UiIcon::Check:
            segment(window_, {center.x - half * .75f, center.y}, {center.x - half * .2f, center.y + half * .55f}, thickness, color);
            segment(window_, {center.x - half * .2f, center.y + half * .55f}, {center.x + half * .85f, center.y - half * .65f}, thickness, color);
            break;
        case UiIcon::Close:
            segment(window_, {center.x - half * .65f, center.y - half * .65f}, {center.x + half * .65f, center.y + half * .65f}, thickness, color);
            segment(window_, {center.x + half * .65f, center.y - half * .65f}, {center.x - half * .65f, center.y + half * .65f}, thickness, color);
            break;
        case UiIcon::Modified: {
            sf::CircleShape dot(half * .55f);
            dot.setOrigin(half * .55f, half * .55f); dot.setPosition(center); dot.setFillColor(color); window_.draw(dot);
            break;
        }
        case UiIcon::Play:
            triangle(window_, {{{center.x - half * .55f, center.y - half * .8f}, {center.x + half * .8f, center.y},
                                {center.x - half * .55f, center.y + half * .8f}}}, color);
            break;
        case UiIcon::Pause:
            segment(window_, {center.x - half * .38f, center.y - half * .75f}, {center.x - half * .38f, center.y + half * .75f}, thickness * 1.45f, color);
            segment(window_, {center.x + half * .38f, center.y - half * .75f}, {center.x + half * .38f, center.y + half * .75f}, thickness * 1.45f, color);
            break;
        case UiIcon::Settings: {
            sf::CircleShape ring(half * .42f);
            ring.setOrigin(half * .42f, half * .42f); ring.setPosition(center); ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(color); ring.setOutlineThickness(thickness); window_.draw(ring);
            for (int i = 0; i < 8; ++i) {
                const float angle = static_cast<float>(i) * PI_F / 4.f;
                const sf::Vector2f from{center.x + std::cos(angle) * half * .58f, center.y + std::sin(angle) * half * .58f};
                const sf::Vector2f to{center.x + std::cos(angle) * half * .9f, center.y + std::sin(angle) * half * .9f};
                segment(window_, from, to, thickness, color);
            }
            break;
        }
        case UiIcon::Delete: {
            sf::RectangleShape bin({half * 1.05f, half * 1.05f});
            bin.setOrigin(half * .525f, 0.f); bin.setPosition(center.x, center.y - half * .15f); bin.setFillColor(color); window_.draw(bin);
            segment(window_, {center.x - half * .75f, center.y - half * .4f}, {center.x + half * .75f, center.y - half * .4f}, thickness, color);
            segment(window_, {center.x - half * .3f, center.y - half * .7f}, {center.x + half * .3f, center.y - half * .7f}, thickness, color);
            break;
        }
    }
}

} // namespace aegis::ui
