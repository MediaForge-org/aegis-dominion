#include "Tower.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "Effects.hpp"
#include <limits>
#include <cmath>

Tower::Tower(TowerKind kind,sf::Vector2f pos):kind_(kind),pos_(pos),spent_(towerCost(kind)){}

TowerStats Tower::scaled(TowerStats s) const{
    float dmg=std::pow(1.42f,float(level_-1));
    float rate=std::pow(.91f,float(level_-1));
    s.damage*=dmg; s.cooldown*=rate; s.range+=8.f*float(level_-1); s.splash+=5.f*float(level_-1);
    return s;
}

Enemy* Tower::acquire(const std::vector<std::unique_ptr<Enemy>>& enemies,const TowerStats& s) const{
    Enemy* best=nullptr; float bestVal=0.f; bool init=false;
    for(const auto& up:enemies){ Enemy* e=up.get(); if(e->dead()||e->reachedEnd()||distance(pos_,e->position())>s.range) continue;
        float v=0.f;
        switch(targetMode_){
            case TargetMode::First:v=e->progress(); break;
            case TargetMode::Last:v=-e->progress(); break;
            case TargetMode::Strongest:v=e->hp()+e->shield(); break;
            case TargetMode::Nearest:v=-distance(pos_,e->position()); break;
        }
        if(!init||v>bestVal){init=true;bestVal=v;best=e;}
    }
    return best;
}

void Tower::update(float dt,std::vector<std::unique_ptr<Enemy>>& enemies,std::vector<Projectile>& projectiles,Effects& fx,Assets& assets){
    cooldownTimer_-=dt; auto s=stats(); Enemy* t=acquire(enemies,s);
    if(t){ turretAngle_=angleDeg(t->position()-pos_); if(cooldownTimer_<=0){ fire(*t,enemies,projectiles,fx,assets); cooldownTimer_=s.cooldown; } }
}

void Tower::draw(sf::RenderTarget& rt,const Assets& assets,bool selected) const{
    if(selected){ sf::CircleShape r(range()); r.setOrigin(range(),range()); r.setPosition(pos_); r.setFillColor(sf::Color(80,190,255,18)); r.setOutlineColor(sf::Color(115,220,255,90)); r.setOutlineThickness(2); rt.draw(r); }
    sf::Sprite base(assets.towerBase(kind_)); auto bb=base.getLocalBounds(); base.setOrigin(bb.width/2,bb.height/2); base.setPosition(pos_); base.setScale(.72f,.72f); rt.draw(base);
    sf::Sprite turret(assets.towerTurret(kind_)); auto tb=turret.getLocalBounds(); turret.setOrigin(tb.width/2,tb.height/2); turret.setPosition(pos_); turret.setRotation(turretAngle_); turret.setScale(.72f,.72f); rt.draw(turret);
    for(int i=0;i<level_;++i){ sf::CircleShape pip(3.2f); pip.setOrigin(3.2f,3.2f); pip.setPosition(pos_.x-10.5f+i*7.f,pos_.y+40.f); pip.setFillColor(branch_==UpgradeBranch::A?sf::Color(92,224,255):branch_==UpgradeBranch::B?sf::Color(255,176,92):sf::Color(220,230,238)); rt.draw(pip); }
}
void Tower::cycleTargetMode(){ targetMode_=static_cast<TargetMode>((static_cast<int>(targetMode_)+1)%4); }
int Tower::upgradeCost() const { if(level_>=4)return 0; return static_cast<int>(towerCost(kind_)*(0.72f+0.38f*level_)); }
bool Tower::upgrade(UpgradeBranch choice){
    if(level_>=4) return false;
    if(level_==2 && branch_==UpgradeBranch::None){
        if(choice==UpgradeBranch::None) return false;
        branch_=choice;
    }
    spent_+=upgradeCost();
    level_++;
    return true;
}

PulseTower::PulseTower(sf::Vector2f p):Tower(TowerKind::Pulse,p){}
TowerStats PulseTower::stats()const{ auto s=scaled({24,175,.42f,760,0,1,0,1}); if(branch_==UpgradeBranch::A){s.cooldown*=.70f;s.damage*=1.06f;} if(branch_==UpgradeBranch::B){s.damage*=1.42f;s.range*=1.08f;} return s; }
void PulseTower::fire(Enemy& t,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>& p,Effects& fx,Assets& a){ auto s=stats(); p.push_back({ProjectileKind::Pulse,pos_,t.id(),s.projectileSpeed,s.damage,0,1,0,sf::Color(84,224,255),4,true}); fx.tracer(pos_,pos_+normalize(t.position()-pos_)*26.f,sf::Color(150,245,255),.06f,2); a.play("laser",36); }

CannonTower::CannonTower(sf::Vector2f p):Tower(TowerKind::Cannon,p){}
TowerStats CannonTower::stats()const{ auto s=scaled({70,185,1.35f,410,72,1,0,1}); if(branch_==UpgradeBranch::A){s.splash*=1.5f;s.damage*=.94f;} if(branch_==UpgradeBranch::B){s.damage*=1.52f;s.cooldown*=1.10f;} return s; }
void CannonTower::fire(Enemy& t,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>& p,Effects& fx,Assets& a){ auto s=stats(); p.push_back({ProjectileKind::Shell,pos_,t.id(),s.projectileSpeed,s.damage,s.splash,1,0,sf::Color(255,178,76),4,true}); fx.burst(pos_,sf::Color(255,190,90),6,70); a.play("shoot",42); }

FrostTower::FrostTower(sf::Vector2f p):Tower(TowerKind::Frost,p){}
TowerStats FrostTower::stats()const{ auto s=scaled({16,165,.62f,620,0,.68f,1.7f,1}); if(branch_==UpgradeBranch::A){s.slowFactor=.48f;s.slowDuration=2.7f;} if(branch_==UpgradeBranch::B){s.damage*=1.55f;s.slowFactor=.62f;} return s; }
void FrostTower::fire(Enemy& t,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>& p,Effects&,Assets& a){ auto s=stats(); p.push_back({ProjectileKind::Frost,pos_,t.id(),s.projectileSpeed,s.damage,0,s.slowFactor,s.slowDuration,sf::Color(132,238,255),4,true}); a.play("laser",24); }

SniperTower::SniperTower(sf::Vector2f p):Tower(TowerKind::Sniper,p){}
TowerStats SniperTower::stats()const{ auto s=scaled({148,330,1.85f,0,0,1,0,1}); if(branch_==UpgradeBranch::A){s.cooldown*=.68f;s.damage*=.96f;} if(branch_==UpgradeBranch::B){s.damage*=1.72f;s.cooldown*=1.08f;} return s; }
void SniperTower::fire(Enemy& t,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>&,Effects& fx,Assets& a){ auto s=stats(); float dealt=t.takeDamage(s.damage); fx.tracer(pos_,t.position(),sf::Color(220,154,255),.16f,3.5f); fx.burst(t.position(),sf::Color(225,170,255),10,100); fx.text(t.position()+sf::Vector2f(8,-30),"-"+std::to_string(int(dealt)),sf::Color(240,190,255)); a.play("shoot",50); }

TeslaTower::TeslaTower(sf::Vector2f p):Tower(TowerKind::Tesla,p){}
TowerStats TeslaTower::stats()const{ auto s=scaled({41,185,.92f,0,0,1,0,3}); if(branch_==UpgradeBranch::A){s.chains+=2;s.damage*=.92f;} if(branch_==UpgradeBranch::B){s.damage*=1.38f;s.chains-=1;} return s; }
void TeslaTower::fire(Enemy& first,std::vector<std::unique_ptr<Enemy>>& enemies,std::vector<Projectile>&,Effects& fx,Assets& a){
    auto s=stats(); Enemy* current=&first; sf::Vector2f from=pos_; std::vector<int> hit;
    for(int chain=0;chain<s.chains && current; ++chain){ float mult=std::pow(.82f,float(chain)); current->takeDamage(s.damage*mult); fx.tracer(from,current->position(),sf::Color(103,255,208),.15f,3); fx.burst(current->position(),sf::Color(120,255,218),5,60); hit.push_back(current->id()); from=current->position(); Enemy* next=nullptr; float bd=120.f;
        for(auto& e:enemies){ if(e->dead()||e->reachedEnd()||std::find(hit.begin(),hit.end(),e->id())!=hit.end())continue; float d=distance(from,e->position()); if(d<bd){bd=d;next=e.get();} } current=next; }
    a.play("laser",48);
}

MissileTower::MissileTower(sf::Vector2f p):Tower(TowerKind::Missile,p){}
TowerStats MissileTower::stats()const{ auto s=scaled({112,230,1.75f,330,94,1,0,1}); if(branch_==UpgradeBranch::A){s.cooldown*=.68f;s.damage*=.9f;s.splash*=.88f;} if(branch_==UpgradeBranch::B){s.damage*=1.62f;s.splash*=1.3f;s.cooldown*=1.13f;} return s; }
void MissileTower::fire(Enemy& t,std::vector<std::unique_ptr<Enemy>>&,std::vector<Projectile>& p,Effects& fx,Assets& a){ auto s=stats(); p.push_back({ProjectileKind::Missile,pos_,t.id(),s.projectileSpeed,s.damage,s.splash,1,0,sf::Color(255,105,78),5,true}); fx.burst(pos_,sf::Color(255,130,85),9,90); a.play("shoot",44); }

std::unique_ptr<Tower> makeTower(TowerKind k,sf::Vector2f p){
    switch(k){
        case TowerKind::Pulse:return std::make_unique<PulseTower>(p);
        case TowerKind::Cannon:return std::make_unique<CannonTower>(p);
        case TowerKind::Frost:return std::make_unique<FrostTower>(p);
        case TowerKind::Sniper:return std::make_unique<SniperTower>(p);
        case TowerKind::Tesla:return std::make_unique<TeslaTower>(p);
        case TowerKind::Missile:return std::make_unique<MissileTower>(p);
    }
    return std::make_unique<PulseTower>(p);
}
