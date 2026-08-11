#include "TextService.hpp"

#include <array>
#include <filesystem>
#include <iostream>

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

bool TextService::tryLoad(const std::string& path) {
    sf::Font candidate;
    if (!candidate.loadFromFile(path) || !supportsGerman(candidate)) return false;
    font_ = std::move(candidate);
    loaded_ = true;
    return true;
}

bool TextService::load(const std::string& assetRoot) {
    loaded_ = false;
    const std::array<std::string, 10> candidates = {
        assetRoot + "/fonts/NotoSans-Regular.ttf",
        assetRoot + "/fonts/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && tryLoad(path)) return true;
    }

    std::error_code error;
    for (const auto& root : {std::string("/usr/share/fonts"), std::string("/usr/local/share/fonts")}) {
        if (!std::filesystem::exists(root, error)) continue;
        for (auto it = std::filesystem::recursive_directory_iterator(
                 root, std::filesystem::directory_options::skip_permission_denied, error);
             it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
            if (error) {
                error.clear();
                continue;
            }
            if (!it->is_regular_file()) continue;
            const auto extension = it->path().extension().string();
            if ((extension == ".ttf" || extension == ".otf") && tryLoad(it->path().string())) return true;
        }
    }

    std::cerr << "WARNUNG: Keine Schrift mit vollständiger deutscher Zeichenabdeckung gefunden.\n";
    return false;
}

sf::Text TextService::makeText(std::string_view utf8, unsigned characterSize) const {
    return sf::Text(fromUtf8(utf8), font_, characterSize);
}

} // namespace aegis::ui
