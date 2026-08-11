#pragma once
#include "Common.hpp"
#include "Map.hpp"
#include "Assets.hpp"
#include <memory>

class Enemy {
public:
    Enemy(int id, EnemyKind kind, float hp, float speed, int reward, float armor=0.f);
    virtual ~Enemy() = default;
    virtual void update(float dt, const GameMap& map);
    virtual float takeDamage(float dmg);
    virtual void applySlow(float factor, float duration);
    virtual void draw(sf::RenderTarget& rt, const Assets& assets) const;
    virtual void onSecond(float /*dt*/) {}

    int id() const { return id_; }
    EnemyKind kind() const { return kind_; }
    sf::Vector2f position() const { return pos_; }
    bool dead() const { return hp_<=0.f; }
    bool reachedEnd() const { return reachedEnd_; }
    float hp() const { return hp_; }
    float maxHp() const { return maxHp_; }
    float progress() const { return progress_; }
    int reward() const { return reward_; }
    float armor() const { return armor_; }
    float shield() const { return shield_; }
    float maxShield() const { return maxShield_; }
    void heal(float v){ hp_=std::min(maxHp_,hp_+v); }
    void kill(){ hp_=0.f; }
    void setShield(float s){ shield_=maxShield_=s; }
    void setRegen(float r){ regenPerSec_=r; }
    void setSlowResistance(float r){ slowResistance_=clampf(r,0.f,.95f); }
    float radius() const { return radius_; }
    std::size_t segment() const { return segment_; }
    float segmentT() const { return segmentT_; }
    void setPathState(std::size_t seg,float t,const GameMap& map);
protected:
    int id_;
    EnemyKind kind_;
    float hp_, maxHp_;
    float baseSpeed_, armor_;
    int reward_;
    float shield_=0.f,maxShield_=0.f,regenPerSec_=0.f,slowResistance_=0.f;
    float slowFactor_=1.f,slowTimer_=0.f;
    float radius_=28.f;
    sf::Vector2f pos_{};
    std::size_t segment_=0;
    float segmentT_=0.f;
    float progress_=0.f;
    float angle_=0.f;
    bool reachedEnd_=false;
};

class RaiderEnemy final: public Enemy { public: RaiderEnemy(int id,float scale); };
class RunnerEnemy final: public Enemy { public: RunnerEnemy(int id,float scale); };
class TankEnemy final: public Enemy { public: TankEnemy(int id,float scale); };
class ShieldEnemy final: public Enemy { public: ShieldEnemy(int id,float scale); };
class RegenEnemy final: public Enemy { public: RegenEnemy(int id,float scale); };
class SplitterEnemy final: public Enemy { public: SplitterEnemy(int id,float scale); };
class BossEnemy final: public Enemy { public: BossEnemy(int id,float scale); };

std::unique_ptr<Enemy> makeEnemy(EnemyKind kind,int id,float scale);
