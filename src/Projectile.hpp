#pragma once
#include "Common.hpp"
#include <vector>
#include <memory>

class Enemy;
class Effects;

struct Projectile {
    ProjectileKind kind=ProjectileKind::Pulse;
    sf::Vector2f pos{};
    int targetId=-1;
    float speed=500.f;
    float damage=10.f;
    float splash=0.f;
    float slowFactor=1.f;
    float slowDuration=0.f;
    sf::Color color=sf::Color::White;
    float life=4.f;
    bool alive=true;
    void update(float dt,std::vector<std::unique_ptr<Enemy>>& enemies,Effects& fx);
    void draw(sf::RenderTarget& rt) const;
};
