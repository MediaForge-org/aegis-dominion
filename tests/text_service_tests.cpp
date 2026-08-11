#include "ui/TextService.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

void utf8ConversionPreservesGermanCodepoints() {
    const auto converted = aegis::ui::TextService::fromUtf8("äöü ÄÖÜ ß – Größe");
    const std::array<sf::Uint32, 7> expected = {0x00E4, 0x00F6, 0x00FC, 0x00C4, 0x00D6, 0x00DC, 0x00DF};
    for (const auto codepoint : expected) require(converted.find(codepoint) != sf::String::InvalidPos, "German UTF-8 codepoint was lost");
}

void discoveredFontSupportsGerman() {
    aegis::ui::TextService text;
    require(text.load("assets"), "no font with German glyph coverage could be loaded");
    require(aegis::ui::TextService::supportsGerman(text.font()), "loaded font lacks German glyphs");
}

void textFactoryUsesUtf8Conversion() {
    aegis::ui::TextService text;
    require(text.load("assets"), "font loading failed");
    const auto sfText = text.makeText("Mörsergröße", 18);
    require(sfText.getString() == aegis::ui::TextService::fromUtf8("Mörsergröße"), "text creation bypassed UTF-8 conversion");
}
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"UTF-8 conversion", utf8ConversionPreservesGermanCodepoints},
        {"German font coverage", discoveredFontSupportsGerman},
        {"UTF-8 text factory", textFactoryUsesUtf8Conversion}
    };
    try { for (const auto& test : tests) test.second(); std::cout << tests.size() << " text test cases passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << "TEST FAILURE: " << error.what() << '\n'; return 1; }
}
