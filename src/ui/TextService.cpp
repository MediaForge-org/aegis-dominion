#include "TextService.hpp"

#include <array>

namespace aegis::ui {
namespace {

constexpr std::array<sf::Uint32, 7> GermanGlyphs = {
    0x00C4, 0x00D6, 0x00DC, 0x00E4, 0x00F6, 0x00FC, 0x00DF
};

} // namespace

sf::String TextService::fromUtf8(std::string_view utf8) {
    return sf::String::fromUtf8(utf8.begin(), utf8.end());
}

bool TextService::supportsGerman(const sf::Font& font) {
    for (const auto codepoint : GermanGlyphs) {
        if (!font.hasGlyph(codepoint)) return false;
    }
    return true;
}

sf::Text TextService::makeText(std::string_view utf8, unsigned characterSize) const {
    return sf::Text(fromUtf8(utf8), *font_, characterSize);
}

} // namespace aegis::ui
