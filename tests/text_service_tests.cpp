#include "ui/TextService.hpp"
#include "assets/AssetManager.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
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
    aegis::assets::AssetManager assets;
    aegis::ui::TextService text;
    const auto& font = assets.font(aegis::assets::FontId{"font.ui.default"});
    text.bind(font, assets.hasFontFallback());
    require(text.loaded(), "no font with German glyph coverage could be loaded");
    require(aegis::ui::TextService::supportsGerman(text.font()), "loaded font lacks German glyphs");
}

void textFactoryUsesUtf8Conversion() {
    aegis::assets::AssetManager assets;
    aegis::ui::TextService text;
    const auto& font = assets.font(aegis::assets::FontId{"font.ui.default"});
    text.bind(font, assets.hasFontFallback());
    require(text.loaded(), "font loading failed");
    const auto sfText = text.makeText("Mörsergröße", 18);
    require(sfText.getString() == aegis::ui::TextService::fromUtf8("Mörsergröße"), "text creation bypassed UTF-8 conversion");
}

void visibleUiSourcesDoNotEmbedReservedIconGlyphs() {
    const std::array<std::string, 12> reservedIcons = {
        "\xE2\x86\x90", "\xE2\x86\x92", "\xE2\x96\xB6", "\xE2\x97\x80",
        "\xE2\x96\xB2", "\xE2\x96\xBC", "\xE2\x9C\x93", "\xE2\x9C\x95",
        "\xE2\x9A\x99", "\xE2\x86\xB6", "\xE2\x86\xB7", "\xE2\x97\x8F"};
    const auto sourceRoot = std::filesystem::path(AEGIS_SOURCE_DIR) / "src";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceRoot)) {
        if (!entry.is_regular_file()) continue;
        const auto extension = entry.path().extension();
        if (extension != ".cpp" && extension != ".hpp" && extension != ".h") continue;
        std::ifstream stream(entry.path(), std::ios::binary);
        const std::string contents{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        for (const auto& icon : reservedIcons)
            require(contents.find(icon) == std::string::npos,
                    "reserved UI icon glyph embedded in " + entry.path().string());
    }
}
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"UTF-8 conversion", utf8ConversionPreservesGermanCodepoints},
        {"German font coverage", discoveredFontSupportsGerman},
        {"UTF-8 text factory", textFactoryUsesUtf8Conversion},
        {"UI icons stay out of font text", visibleUiSourcesDoNotEmbedReservedIconGlyphs}
    };
    try { for (const auto& test : tests) test.second(); std::cout << tests.size() << " text test cases passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << "TEST FAILURE: " << error.what() << '\n'; return 1; }
}
