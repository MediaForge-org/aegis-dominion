#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

#include <string>
#include <string_view>

namespace aegis::ui {

class TextService {
public:
    bool load(const std::string& assetRoot = "assets");

    bool loaded() const { return loaded_; }
    const sf::Font& font() const { return font_; }
    sf::Text makeText(std::string_view utf8, unsigned characterSize) const;

    static sf::String fromUtf8(std::string_view utf8);
    static bool supportsGerman(const sf::Font& font);

private:
    bool tryLoad(const std::string& path);

    sf::Font font_;
    bool loaded_ = false;
};

} // namespace aegis::ui
