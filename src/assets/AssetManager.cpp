#include "AssetManager.hpp"

#include "logging/Logger.hpp"
#include "ui/TextService.hpp"

#include <array>

namespace aegis::assets {

sf::Image makeMissingTextureImage() {
    sf::Image image;
    image.create(32, 32, sf::Color(20, 20, 24));
    for (unsigned y = 0; y < 32; ++y)
        for (unsigned x = 0; x < 32; ++x)
            if (((x / 8) + (y / 8)) % 2 == 0) image.setPixel(x, y, sf::Color(255, 0, 190));
    return image;
}

AssetManager::AssetManager(AssetCatalog catalog) : catalog_(std::move(catalog)) {}

void AssetManager::createFallbackTexture() {
    if (missingTexture_) return;
    const auto image = makeMissingTextureImage();
    missingTexture_ = std::make_unique<sf::Texture>();
    missingTexture_->loadFromImage(image);
    missingTexture_->setRepeated(true);
    textures_.setFallback(*missingTexture_);
}

bool AssetManager::loadFallbackFont() {
    const std::array<std::filesystem::path, 9> candidates = {
        root_ / "fonts/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"};
    for (const auto& path : candidates) {
        sf::Font candidate;
        if (std::filesystem::exists(path) && candidate.loadFromFile(path.string()) && ui::TextService::supportsGerman(candidate)) {
            missingFont_ = std::move(candidate);
            fontReady_ = true;
            logging::log().info("Font fallback loaded from {}", path.string());
            return true;
        }
    }
    logging::log().error("No UTF-8 capable font fallback is available");
    return false;
}

bool AssetManager::load(const std::filesystem::path& root) {
    root_ = root;
    bool complete = true;
    for (const auto& definition : catalog_.definitions()) {
        if (definition.type == AssetType::Texture) texture(TextureId{definition.id});
        else if (definition.type == AssetType::Sound) sound(SoundId{definition.id});
    }
    font(FontId{"font.ui.default"});
    for (const auto& definition : catalog_.definitions())
        complete = complete && !reportedMissing_.contains(definition.id);
    return complete && fontReady_;
}

void AssetManager::reportMissing(std::string_view id, AssetType type) {
    if (reportedMissing_.insert(std::string(id)).second)
        logging::log().error("Failed to load {} asset '{}'; using fallback", static_cast<int>(type), id);
}

const sf::Texture& AssetManager::texture(const TextureId& id) {
    if (!id.valid()) { createFallbackTexture(); reportMissing(id.value(), AssetType::Texture); return *missingTexture_; }
    const auto* definition = catalog_.find(id.value(), AssetType::Texture);
    if (!definition) { createFallbackTexture(); reportMissing(id.value(), AssetType::Texture); return *missingTexture_; }
    return textures_.get(id, [&]() -> std::unique_ptr<sf::Texture> {
        auto resource = std::make_unique<sf::Texture>();
        if (!resource->loadFromFile((root_ / definition->relativePath).string())) {
            createFallbackTexture(); reportMissing(id.value(), AssetType::Texture); return nullptr;
        }
        resource->setSmooth(definition->smooth);
        resource->setRepeated(definition->repeated);
        return resource;
    });
}

const sf::Font& AssetManager::font(const FontId& id) {
    if (!id.valid()) { reportMissing(id.value(), AssetType::Font); if (!fontReady_) loadFallbackFont(); return missingFont_; }
    if (const auto found = fonts_.find(id); found != fonts_.end()) return *found->second;
    const auto* definition = catalog_.find(id.value(), AssetType::Font);
    if (definition) {
        auto resource = std::make_unique<sf::Font>();
        if (resource->loadFromFile((root_ / definition->relativePath).string()) && ui::TextService::supportsGerman(*resource)) {
            const auto& result = *resource;
            fonts_.emplace(id, std::move(resource));
            fontReady_ = true;
            return result;
        }
    }
    reportMissing(id.value(), AssetType::Font);
    if (!fontReady_) loadFallbackFont();
    return missingFont_;
}

const sf::SoundBuffer& AssetManager::sound(const SoundId& id) {
    const auto fallback = [&]() -> const sf::SoundBuffer& {
        if (!missingSound_) missingSound_ = std::make_unique<sf::SoundBuffer>();
        return *missingSound_;
    };
    if (!id.valid()) { reportMissing(id.value(), AssetType::Sound); return fallback(); }
    if (const auto found = sounds_.find(id); found != sounds_.end()) return *found->second;
    const auto* definition = catalog_.find(id.value(), AssetType::Sound);
    if (!definition) { reportMissing(id.value(), AssetType::Sound); return fallback(); }
    auto resource = std::make_unique<sf::SoundBuffer>();
    if (!resource->loadFromFile((root_ / definition->relativePath).string())) {
        reportMissing(id.value(), AssetType::Sound);
        return fallback();
    }
    const auto& result = *resource;
    sounds_.emplace(id, std::move(resource));
    return result;
}

std::size_t AssetManager::textureLoadCount(const TextureId& id) const {
    return textures_.requestCount(id);
}

} // namespace aegis::assets
