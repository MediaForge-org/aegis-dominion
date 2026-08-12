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

void UiRenderer::card(const sf::FloatRect& rect, sf::Color accent, bool elevated) {
    if (elevated) panel({rect.left + 5.f, rect.top + 7.f, rect.width, rect.height}, sf::Color(0, 0, 0, 85));
    if(rect.width>=92.f&&rect.height>=66.f) nineSlice(rect,assets::TextureId{accent==sf::Color::Transparent?"ui.surface.card":"ui.surface.card_selected"},76.f,std::min(24.f,std::min(rect.width,rect.height)*.2f));
    else {
        const auto border = accent == sf::Color::Transparent ? theme().colors.border : withAlpha(accent, 135);
        panel(rect, elevated ? theme().colors.elevatedSurface : theme().colors.surface, border, theme().sizes.border);
    }
    if (accent != sf::Color::Transparent) panel({rect.left, rect.top, 4.f, rect.height}, accent);
}

void UiRenderer::nineSlice(const sf::FloatRect& rect,const assets::TextureId& textureId,float sourceBorder,float destinationBorder,sf::Color tint){
    const auto& texture=assets_.resources().texture(textureId);const auto size=texture.getSize();
    const int sb=static_cast<int>(std::clamp(sourceBorder,1.f,static_cast<float>(std::min(size.x,size.y))/2.f-1.f));
    const float db=std::clamp(destinationBorder,1.f,std::min(rect.width,rect.height)/2.f);
    const int sourceX[4]={0,sb,static_cast<int>(size.x)-sb,static_cast<int>(size.x)};
    const int sourceY[4]={0,sb,static_cast<int>(size.y)-sb,static_cast<int>(size.y)};
    const float destX[4]={rect.left,rect.left+db,rect.left+rect.width-db,rect.left+rect.width};
    const float destY[4]={rect.top,rect.top+db,rect.top+rect.height-db,rect.top+rect.height};
    for(int y=0;y<3;++y)for(int x=0;x<3;++x){
        const int sw=sourceX[x+1]-sourceX[x],sh=sourceY[y+1]-sourceY[y];const float dw=destX[x+1]-destX[x],dh=destY[y+1]-destY[y];
        if(sw<=0||sh<=0||dw<=0.f||dh<=0.f)continue;
        sf::Sprite sprite(texture,sf::IntRect(sourceX[x],sourceY[y],sw,sh));sprite.setPosition(destX[x],destY[y]);sprite.setScale(dw/static_cast<float>(sw),dh/static_cast<float>(sh));sprite.setColor(tint);window_.draw(sprite);
    }
}

void UiRenderer::separator(sf::Vector2f from, sf::Vector2f to, sf::Color color) { segment(window_, from, to, 1.f, color); }

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
    const bool pressed = hovered && sf::Mouse::isButtonPressed(sf::Mouse::Left);
    const auto tint=pressed?sf::Color(135,145,150):hovered?sf::Color(215,225,230):sf::Color(185,195,202);
    nineSlice(rect,assets::TextureId{"ui.surface.button_primary"},76.f,std::min(16.f,rect.height*.3f),tint);
    if(active){sf::RectangleShape selected({rect.width,rect.height});selected.setPosition(rect.left,rect.top);selected.setFillColor(withAlpha(accent,24));selected.setOutlineColor(accent);selected.setOutlineThickness(2.f);window_.draw(selected);}
    sf::RectangleShape bar({4.f, rect.height - 12.f});
    bar.setPosition(rect.left + 6.f, rect.top + 6.f);
    bar.setFillColor(withAlpha(accent, hovered ? 255 : 180));
    window_.draw(bar);
    return hovered;
}

void UiRenderer::label(const std::string& value, sf::Vector2f position, bool heading, sf::Color color) {
    text(value, heading ? theme().typography.heading : theme().typography.body, position, color, heading);
}

void UiRenderer::progressBar(const sf::FloatRect& rect, float progress, sf::Color accent, const std::string& caption) {
    panel(rect, sf::Color(5, 11, 18, 220), theme().colors.border);
    const float fill = std::clamp(progress, 0.f, 1.f) * std::max(0.f, rect.width - 4.f);
    panel({rect.left + 2.f, rect.top + 2.f, fill, std::max(0.f, rect.height - 4.f)}, accent);
    if (!caption.empty()) text(caption, theme().typography.caption, {rect.left + rect.width / 2.f, rect.top + rect.height / 2.f - 1.f}, Text, true, true);
}

void UiRenderer::tooltip(const sf::FloatRect& anchor, const std::string& value, sf::Color accent) {
    const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    if (!anchor.contains(mouse)) return;
    const float width = std::clamp(24.f + static_cast<float>(value.size()) * 7.f, 120.f, 340.f);
    const sf::FloatRect box(mouse.x + 14.f, mouse.y + 18.f, width, 34.f);
    card(box, accent, true);
    text(value, 12, {box.left + 12.f, box.top + 9.f}, Text);
}

bool UiRenderer::toggle(const sf::FloatRect& rect, const std::string& labelValue, bool value, bool enabled) {
    const auto mouse = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    const auto color = enabled ? (value ? theme().colors.success : theme().colors.textSecondary) : theme().colors.disabled;
    label(labelValue, {rect.left, rect.top + 5.f}, false, enabled ? Text : theme().colors.disabled);
    const sf::FloatRect track(rect.left + rect.width - 52.f, rect.top, 52.f, 28.f);
    panel(track, value ? withAlpha(color, 110) : sf::Color(18, 29, 39), color, 1.f);
    sf::CircleShape knob(10.f); knob.setOrigin(10.f, 10.f); knob.setPosition(track.left + (value ? 38.f : 14.f), track.top + 14.f);
    knob.setFillColor(color); window_.draw(knob);
    return enabled && rect.contains(mouse);
}

float UiRenderer::slider(const sf::FloatRect& rect, float value, sf::Color accent, bool enabled) {
    value = std::clamp(value, 0.f, 1.f);
    const auto color = enabled ? accent : theme().colors.disabled;
    panel({rect.left, rect.top + rect.height / 2.f - 2.f, rect.width, 4.f}, sf::Color(35, 50, 61));
    panel({rect.left, rect.top + rect.height / 2.f - 2.f, rect.width * value, 4.f}, color);
    sf::CircleShape knob(8.f); knob.setOrigin(8.f, 8.f); knob.setPosition(rect.left + rect.width * value, rect.top + rect.height / 2.f); knob.setFillColor(color); window_.draw(knob);
    return value;
}

void UiRenderer::dropdown(const sf::FloatRect& rect, const std::string& labelValue, bool open, bool enabled) {
    card(rect, open ? theme().colors.primary : sf::Color::Transparent, false);
    text(labelValue, 14, {rect.left + 12.f, rect.top + rect.height / 2.f - 8.f}, enabled ? Text : theme().colors.disabled);
    icon(open ? UiIcon::ArrowUp : UiIcon::ArrowDown, {rect.left + rect.width - 18.f, rect.top + rect.height / 2.f}, 11.f, enabled ? Text : theme().colors.disabled);
}

void UiRenderer::scrollArea(const sf::FloatRect& rect, float position, float contentRatio) {
    card(rect, sf::Color::Transparent, false);
    const float thumbHeight = std::max(24.f, rect.height * std::clamp(contentRatio, 0.f, 1.f));
    panel({rect.left + rect.width - 6.f, rect.top + (rect.height - thumbHeight) * std::clamp(position, 0.f, 1.f), 4.f, thumbHeight}, theme().colors.primary);
}

void UiRenderer::modalDialog(const sf::FloatRect& rect, const std::string& titleValue) {
    panel({0.f, 0.f, 1600.f, 900.f}, sf::Color(0, 0, 0, 185));
    card(rect, theme().colors.primary, true);
    text(titleValue, theme().typography.title, {rect.left + rect.width / 2.f, rect.top + 48.f}, Text, true, true);
}

void UiRenderer::toast(const sf::FloatRect& rect, const std::string& message, sf::Color accent, float visibility) {
    const auto alpha = static_cast<sf::Uint8>(255.f * std::clamp(visibility, 0.f, 1.f));
    card({rect.left + (1.f - visibility) * 24.f, rect.top, rect.width, rect.height}, withAlpha(accent, alpha), true);
    text(message, 14, {rect.left + 18.f, rect.top + rect.height / 2.f - 8.f}, withAlpha(Text, alpha), true);
}

int UiRenderer::tabBar(const sf::FloatRect& rect, const std::vector<std::string>& labels, int selected) {
    if (labels.empty()) return selected;
    const float width = rect.width / static_cast<float>(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i)
        button({rect.left + static_cast<float>(i) * width, rect.top, width - 4.f, rect.height}, labels[i], theme().colors.primary, static_cast<int>(i) == selected, 13);
    return selected;
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
