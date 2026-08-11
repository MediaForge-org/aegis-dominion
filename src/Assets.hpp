#pragma once
#include "Common.hpp"
#include "ui/TextService.hpp"
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
    aegis::ui::TextService& text() { return text_; }
    const aegis::ui::TextService& text() const { return text_; }

    void play(const std::string& name, float volume=100.f);
private:
    bool loadTexture(sf::Texture& t, const std::string& path);
    std::map<TowerKind, sf::Texture> towerBases_, towerTurrets_;
    std::map<EnemyKind, sf::Texture> enemies_;
    std::vector<sf::Texture> maps_;
    std::map<std::string, sf::Texture> ui_;
    std::map<std::string, sf::SoundBuffer> buffers_;
    std::map<std::string, sf::Sound> sounds_;
    aegis::ui::TextService text_;
};
