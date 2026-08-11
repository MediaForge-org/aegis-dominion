#include "Assets.hpp"
#include <iostream>

bool Assets::loadTexture(sf::Texture& t, const std::string& path){
    if(!t.loadFromFile(path)){ std::cerr << "Konnte Textur nicht laden: " << path << "\n"; return false; }
    t.setSmooth(true); return true;
}

bool Assets::load(const std::string& root){
    bool ok=true;
    const std::vector<std::pair<TowerKind,std::string>> towers={
        {TowerKind::Pulse,"pulse"},{TowerKind::Cannon,"cannon"},{TowerKind::Frost,"frost"},
        {TowerKind::Sniper,"sniper"},{TowerKind::Tesla,"tesla"},{TowerKind::Missile,"missile"}
    };
    for(auto& [k,n]:towers){
        ok &= loadTexture(towerBases_[k],root+"/towers/"+n+"_base.png");
        ok &= loadTexture(towerTurrets_[k],root+"/towers/"+n+"_turret.png");
    }
    const std::vector<std::pair<EnemyKind,std::string>> ens={
        {EnemyKind::Raider,"raider"},{EnemyKind::Runner,"runner"},{EnemyKind::Tank,"tank"},
        {EnemyKind::Shield,"shield"},{EnemyKind::Regen,"regen"},{EnemyKind::Splitter,"splitter"},{EnemyKind::Boss,"boss"}
    };
    for(auto& [k,n]:ens) ok &= loadTexture(enemies_[k],root+"/enemies/"+n+".png");
    for(const auto& n:{"verdant","frost","ember"}){ sf::Texture t; ok &= loadTexture(t,root+"/maps/"+n+".png"); maps_.push_back(std::move(t)); }
    for(const auto& n:{"menu_background","logo","credits","core","wave","score"}){ sf::Texture t; ok &= loadTexture(t,root+"/ui/"+std::string(n)+".png"); ui_[n]=std::move(t); }
    ok &= text_.load(root);
    for(const auto& n:{"click","build","upgrade","shoot","laser","explosion","wave","gameover"}){
        sf::SoundBuffer b;
        if(b.loadFromFile(root+"/sfx/"+std::string(n)+".wav")){ buffers_[n]=std::move(b); sounds_[n].setBuffer(buffers_[n]); }
    }
    return ok;
}

const sf::Texture& Assets::towerBase(TowerKind k) const { return towerBases_.at(k); }
const sf::Texture& Assets::towerTurret(TowerKind k) const { return towerTurrets_.at(k); }
const sf::Texture& Assets::enemy(EnemyKind k) const { return enemies_.at(k); }
const sf::Texture& Assets::mapTexture(int index) const { return maps_.at(static_cast<std::size_t>(index)); }
const sf::Texture& Assets::ui(const std::string& n) const { return ui_.at(n); }
void Assets::play(const std::string& n,float volume){ auto it=sounds_.find(n); if(it!=sounds_.end()){ it->second.setVolume(volume); it->second.play(); } }
