#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

#include <string>
#include <string_view>

namespace aegis::ui {

class TextService {
public:
    void bind(const sf::Font& font, bool loaded = true) { font_ = &font; loaded_ = loaded; }

    bool loaded() const { return loaded_; }
    const sf::Font& font() const { return *font_; }
    sf::Text makeText(std::string_view utf8, unsigned characterSize) const;

    static sf::String fromUtf8(std::string_view utf8);
    static bool supportsGerman(const sf::Font& font);

private:
    const sf::Font* font_ = nullptr;
    bool loaded_ = false;
};

} // namespace aegis::ui
