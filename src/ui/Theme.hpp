#pragma once

#include <SFML/Graphics/Color.hpp>

namespace aegis::ui {

struct ThemeColors {
    sf::Color background, surface, elevatedSurface, border, primary, secondary, accent;
    sf::Color success, warning, error, textPrimary, textSecondary, disabled;
};
struct ThemeSpacing { float xs, s, m, l, xl; };
struct ThemeSizes {
    float buttonSmall, button, buttonLarge, panelPadding, corner, border, iconSmall, icon, iconLarge;
};
struct ThemeTypography { unsigned body, caption, heading, title, display; };

struct Theme {
    ThemeColors colors;
    ThemeSpacing spacing;
    ThemeSizes sizes;
    ThemeTypography typography;
    static const Theme& command();
    bool complete() const;
};

} // namespace aegis::ui
