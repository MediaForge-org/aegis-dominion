#pragma once

#include "Assets.hpp"
#include "Theme.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

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

enum class UiIcon {
    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    Undo,
    Redo,
    Check,
    Close,
    Modified,
    Play,
    Pause,
    Settings,
    Delete
};

enum class IconPlacement { Left, Right, Center };

class UiRenderer {
public:
    UiRenderer(sf::RenderWindow& window, Assets& assets) : window_(window), assets_(assets) {}

    void panel(const sf::FloatRect& rect, sf::Color fill, sf::Color outline = sf::Color::Transparent, float thickness = 1.f);
    void card(const sf::FloatRect& rect, sf::Color accent = sf::Color::Transparent, bool elevated = true);
    void nineSlice(const sf::FloatRect& rect, const assets::TextureId& texture, float sourceBorder = 76.f,
                   float destinationBorder = 22.f, sf::Color tint = sf::Color::White);
    void separator(sf::Vector2f from, sf::Vector2f to, sf::Color color = sf::Color(55, 84, 102));
    void text(const std::string& value, unsigned size, sf::Vector2f position, sf::Color color = Text,
              bool bold = false, bool centered = false);
    void wrapped(const std::string& value, unsigned size, sf::FloatRect box, sf::Color color = Text,
                 float lineGap = 5.f, bool bold = false);
    bool button(const sf::FloatRect& rect, const std::string& label, sf::Color accent,
                bool active = false, unsigned size = 18);
    bool iconButton(const sf::FloatRect& rect, const std::string& label, UiIcon icon,
                    IconPlacement placement, sf::Color accent, bool active = false, unsigned size = 18);
    void icon(UiIcon icon, sf::Vector2f center, float size, sf::Color color = Text);
    void label(const std::string& value, sf::Vector2f position, bool heading = false, sf::Color color = Text);
    void progressBar(const sf::FloatRect& rect, float progress, sf::Color accent, const std::string& caption = {});
    void tooltip(const sf::FloatRect& anchor, const std::string& value, sf::Color accent = Cyan);
    bool toggle(const sf::FloatRect& rect, const std::string& label, bool value, bool enabled = true);
    float slider(const sf::FloatRect& rect, float value, sf::Color accent = Cyan, bool enabled = true);
    void dropdown(const sf::FloatRect& rect, const std::string& label, bool open = false, bool enabled = true);
    void scrollArea(const sf::FloatRect& rect, float position, float contentRatio);
    void modalDialog(const sf::FloatRect& rect, const std::string& title);
    void toast(const sf::FloatRect& rect, const std::string& message, sf::Color accent, float visibility = 1.f);
    int tabBar(const sf::FloatRect& rect, const std::vector<std::string>& labels, int selected);
    const Theme& theme() const { return Theme::command(); }

private:
    bool buttonSurface(const sf::FloatRect& rect, sf::Color accent, bool active);

    sf::RenderWindow& window_;
    Assets& assets_;
};

} // namespace aegis::ui
