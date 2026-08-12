#pragma once
#include "Common.hpp"
#include "Assets.hpp"
#include "render/RenderSnapshot.hpp"
#include <memory>
#include <vector>

class Enemy;
class Effects;
struct Projectile;

struct TowerStats {
    float damage=20.f, range=170.f, cooldown=.7f, projectileSpeed=650.f, splash=0.f;
    float slowFactor=1.f, slowDuration=0.f;
    int chains=1;
};

class Tower {
public:
    Tower(TowerKind kind,sf::Vector2f pos);
    virtual ~Tower()=default;
    void update(float dt,std::vector<std::unique_ptr<Enemy>>& enemies,std::vector<Projectile>& projectiles,Effects& fx,Assets& assets);
    aegis::render::TowerRenderSnapshot renderSnapshot(bool selected) const;
    virtual TowerStats stats() const=0;
    virtual std::string branchAName() const=0;
    virtual std::string branchBName() const=0;
    virtual std::string branchADesc() const=0;
    virtual std::string branchBDesc() const=0;
    virtual void fire(Enemy& target,std::vector<std::unique_ptr<Enemy>>& enemies,std::vector<Projectile>& projectiles,Effects& fx,Assets& assets)=0;

    TowerKind kind() const{return kind_;}
    sf::Vector2f position() const{return pos_;}
    int level() const{return level_;}
    UpgradeBranch branch() const{return branch_;}
    TargetMode targetMode() const{return targetMode_;}
    void cycleTargetMode();
    int upgradeCost() const;
    bool upgrade(UpgradeBranch branchChoice=UpgradeBranch::None);
    int sellValue() const { return static_cast<int>(spent_*0.72f); }
    int spent() const{return spent_;}
    bool maxLevel() const{return level_>=4;}
    bool needsBranch() const{return level_==2 && branch_==UpgradeBranch::None;}
    float range() const{return stats().range;}
    float rotation() const{return turretAngle_;}
protected:
    Enemy* acquire(const std::vector<std::unique_ptr<Enemy>>& enemies,const TowerStats& s) const;
    TowerStats scaled(TowerStats s) const;
    TowerKind kind_;
    sf::Vector2f pos_;
    int level_=1;
    UpgradeBranch branch_=UpgradeBranch::None;
    TargetMode targetMode_=TargetMode::First;
    int spent_=0;
    float cooldownTimer_=0.f;
    float turretAngle_=0.f;
    float desiredTurretAngle_=0.f;
    float recoil_=0.f;
    float idlePhase_=0.f;
};

class PulseTower final:public Tower{public:PulseTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Overclock";}std::string branchBName()const override{return"Panzerbrecher";}std::string branchADesc()const override{return"Sehr viel höhere Feuerrate.";}std::string branchBDesc()const override{return"Mehr Schaden und Reichweite.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};
class CannonTower final:public Tower{public:CannonTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Splitterladung";}std::string branchBName()const override{return"Schweres Kaliber";}std::string branchADesc()const override{return"Deutlich größerer Explosionsradius.";}std::string branchBDesc()const override{return"Massiver Schaden pro Treffer.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};
class FrostTower final:public Tower{public:FrostTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Tiefkühlung";}std::string branchBName()const override{return"Kristallsplitter";}std::string branchADesc()const override{return"Stärkerer und längerer Slow.";}std::string branchBDesc()const override{return"Mehr Direktschaden.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};
class SniperTower final:public Tower{public:SniperTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Beschleuniger";}std::string branchBName()const override{return"Exekutor";}std::string branchADesc()const override{return"Schnellere Schussfolge.";}std::string branchBDesc()const override{return"Extremer Einzelschaden.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};
class TeslaTower final:public Tower{public:TeslaTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Relaisnetz";}std::string branchBName()const override{return"Überladung";}std::string branchADesc()const override{return"Blitz springt auf mehr Ziele.";}std::string branchBDesc()const override{return"Höherer Schaden je Sprung.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};
class MissileTower final:public Tower{public:MissileTower(sf::Vector2f p);TowerStats stats()const override;std::string branchAName()const override{return"Schwarm";}std::string branchBName()const override{return"Sprengkopf";}std::string branchADesc()const override{return"Schnellere Raketenfolge.";}std::string branchBDesc()const override{return"Größere und stärkere Explosion.";}void fire(Enemy&,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects&,Assets&)override;};

std::unique_ptr<Tower> makeTower(TowerKind kind,sf::Vector2f pos);
