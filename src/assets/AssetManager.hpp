#pragma once

#include "AssetCatalog.hpp"
#include "ResourceCache.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace aegis::assets {

sf::Image makeMissingTextureImage();

class AssetManager {
public:
    explicit AssetManager(AssetCatalog catalog = AssetCatalog::builtIn());

    bool load(const std::filesystem::path& root);
    const sf::Texture& texture(const TextureId& id);
    const sf::Font& font(const FontId& id);
    const sf::SoundBuffer& sound(const SoundId& id);
    bool hasFontFallback() const { return fontReady_; }
    std::size_t textureLoadCount(const TextureId& id) const;
    const AssetCatalog& catalog() const { return catalog_; }

private:
    void createFallbackTexture();
    bool loadFallbackFont();
    void reportMissing(std::string_view id, AssetType type);

    AssetCatalog catalog_;
    std::filesystem::path root_;
    std::unordered_map<FontId, std::unique_ptr<sf::Font>, AssetIdHash> fonts_;
    std::unordered_map<SoundId, std::unique_ptr<sf::SoundBuffer>, AssetIdHash> sounds_;
    std::unordered_set<std::string> reportedMissing_;
    std::unique_ptr<sf::Texture> missingTexture_;
    ResourceCache<TextureId, sf::Texture> textures_;
    sf::Font missingFont_;
    std::unique_ptr<sf::SoundBuffer> missingSound_;
    bool fontReady_ = false;
};

} // namespace aegis::assets
