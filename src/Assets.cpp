#include "Assets.hpp"
#include <array>

namespace {
std::string towerId(TowerKind kind, std::string_view part) {
    const std::array names{"pulse", "cannon", "frost", "sniper", "tesla", "missile"};
    return "tower." + std::string(names[static_cast<std::size_t>(kind)]) + "." + std::string(part);
}
std::string enemyId(EnemyKind kind) {
    const std::array names{"raider", "runner", "tank", "shield", "regen", "splitter", "boss"};
    return "enemy." + std::string(names[static_cast<std::size_t>(kind)]) + ".idle";
}
}

bool Assets::load(const std::string& root){
    const bool ok = resources_.load(root);
    const auto& font = resources_.font(aegis::assets::FontId{"font.ui.default"});
    text_.bind(font, resources_.hasFontFallback());
    for(const auto& n:{"click","build","upgrade","shoot","laser","explosion","wave","gameover"}){
        sounds_[n].setBuffer(resources_.sound(aegis::assets::SoundId{"sfx." + std::string(n)}));
    }
    return ok;
}

const sf::Texture& Assets::towerBase(TowerKind k) const { return const_cast<Assets*>(this)->resources_.texture(aegis::assets::TextureId{towerId(k, "base")}); }
const sf::Texture& Assets::towerTurret(TowerKind k) const { return const_cast<Assets*>(this)->resources_.texture(aegis::assets::TextureId{towerId(k, "turret")}); }
const sf::Texture& Assets::enemy(EnemyKind k) const { return const_cast<Assets*>(this)->resources_.texture(aegis::assets::TextureId{enemyId(k)}); }
const sf::Texture& Assets::mapTexture(int index) const {
    const std::array ids{"environment.verdant.background", "environment.frost.background", "environment.ember.background"};
    const auto safeIndex = std::clamp(index, 0, 2);
    return const_cast<Assets*>(this)->resources_.texture(aegis::assets::TextureId{ids[static_cast<std::size_t>(safeIndex)]});
}
const sf::Texture& Assets::ui(const std::string& n) const { return const_cast<Assets*>(this)->resources_.texture(aegis::assets::TextureId{"ui." + n}); }
void Assets::play(const std::string& n,float volume){ auto it=sounds_.find(n); if(it!=sounds_.end()){ it->second.setVolume(volume); it->second.play(); } }
