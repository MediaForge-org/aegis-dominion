#pragma once
#include "Common.hpp"
#include "assets/AssetManager.hpp"
#include "ui/TextService.hpp"
#include "animation/AnimationCatalog.hpp"
#include <SFML/Audio.hpp>
#include <map>

class Assets {
public:
    bool load(const std::string& root="assets");
    const sf::Texture& towerBase(TowerKind k) const;
    const sf::Texture& towerTurret(TowerKind k) const;
    const sf::Texture& enemy(EnemyKind k) const;
    const sf::Texture& mapTexture(int index) const;
    const sf::Texture& ui(const std::string& name) const;
    aegis::assets::AssetManager& resources() { return resources_; }
    const aegis::assets::AssetManager& resources() const { return resources_; }
    aegis::ui::TextService& text() { return text_; }
    const aegis::ui::TextService& text() const { return text_; }
    const aegis::animation::AnimationCatalog& animations() const { return animations_; }

    void play(const std::string& name, float volume=100.f);
private:
    // Asset lookup is logically const but may populate the owned resource cache.
    mutable aegis::assets::AssetManager resources_;
    std::map<std::string, sf::Sound> sounds_;
    aegis::ui::TextService text_;
    aegis::animation::AnimationCatalog animations_;
};
