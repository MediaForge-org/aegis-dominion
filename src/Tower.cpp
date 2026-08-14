#include "Tower.hpp"

#include "Effects.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"

#include <cmath>
#include <limits>

Tower::Tower(TowerKind kind, sf::Vector2f pos) : kind_(kind), pos_(pos), spent_(towerCost(kind)) {}

TowerStats Tower::scaled(TowerStats stats) const {
    const float damageScale = std::pow(1.42f, float(level_ - 1));
    const float rateScale = std::pow(.91f, float(level_ - 1));
    stats.damage *= damageScale;
    stats.cooldown *= rateScale;
    stats.range += 8.f * float(level_ - 1);
    stats.splash += 5.f * float(level_ - 1);
    return stats;
}

Enemy* Tower::acquire(const std::vector<std::unique_ptr<Enemy>>& enemies, const TowerStats& stats) const {
    Enemy* best = nullptr;
    float bestValue = 0.f;
    bool initialized = false;
    for (const auto& enemy : enemies) {
        Enemy* candidate = enemy.get();
        if (candidate->dead() || candidate->reachedEnd() || distance(pos_, candidate->position()) > stats.range)
            continue;

        float value = 0.f;
        switch (targetMode_) {
            case TargetMode::First: value = candidate->progress(); break;
            case TargetMode::Last: value = -candidate->progress(); break;
            case TargetMode::Strongest: value = candidate->hp() + candidate->shield(); break;
            case TargetMode::Nearest: value = -distance(pos_, candidate->position()); break;
        }
        if (!initialized || value > bestValue) {
            initialized = true;
            bestValue = value;
            best = candidate;
        }
    }
    return best;
}

void Tower::update(float dt, std::vector<std::unique_ptr<Enemy>>& enemies,
                   std::vector<Projectile>& projectiles, Effects& fx, Assets& assets) {
    cooldownTimer_ -= dt;
    recoil_ = std::max(0.f, recoil_ - dt * 5.5f);
    idlePhase_ += dt * 2.2f;
    const auto towerStats = stats();
    Enemy* target = acquire(enemies, towerStats);
    if (target) {
        desiredTurretAngle_ = angleDeg(target->position() - pos_);
        const float delta = std::fmod(desiredTurretAngle_ - turretAngle_ + 540.f, 360.f) - 180.f;
        turretAngle_ += clampf(delta, -240.f * dt, 240.f * dt);
        if (cooldownTimer_ <= 0) {
            fire(*target, enemies, projectiles, fx, assets);
            cooldownTimer_ = towerStats.cooldown;
            recoil_ = 1.f;
        }
    }
}

aegis::render::TowerRenderSnapshot Tower::renderSnapshot(bool selected) const {
    static constexpr const char* names[] = {"pulse", "cannon", "frost", "sniper", "tesla", "missile"};
    static constexpr const char* profiles[] = {"energy", "kinetic", "cryo", "rail", "tesla", "missile"};
    const auto index = static_cast<std::size_t>(kind_);
    aegis::render::TowerRenderSnapshot snapshot;
    snapshot.position = {pos_.x, pos_.y};
    snapshot.turretRotationDeg = turretAngle_;
    snapshot.scale = .72f;
    snapshot.range = range();
    snapshot.recoil = recoil_;
    snapshot.idlePhase = idlePhase_;
    snapshot.level = level_;
    snapshot.branch = static_cast<int>(branch_);
    snapshot.baseVisualId = "tower." + std::string(names[index]) + ".base";
    snapshot.turretVisualId = "tower." + std::string(names[index]) + ".turret";
    snapshot.effectProfile = profiles[index];
    snapshot.selected = selected;
    return snapshot;
}

void Tower::cycleTargetMode() {
    targetMode_ = static_cast<TargetMode>((static_cast<int>(targetMode_) + 1) % 4);
}

int Tower::upgradeCost() const {
    if (level_ >= 4) return 0;
    return static_cast<int>(towerCost(kind_) * (0.72f + 0.38f * level_));
}

bool Tower::upgrade(UpgradeBranch choice) {
    if (level_ >= 4) return false;
    if (level_ == 2 && branch_ == UpgradeBranch::None) {
        if (choice == UpgradeBranch::None) return false;
        branch_ = choice;
    }
    spent_ += upgradeCost();
    level_++;
    return true;
}

PulseTower::PulseTower(sf::Vector2f position) : Tower(TowerKind::Pulse, position) {}

TowerStats PulseTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 24,
        .range = 175,
        .cooldown = .42f,
        .projectileSpeed = 760,
        .splash = 0,
        .slowFactor = 1,
        .slowDuration = 0,
        .chains = 1,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.cooldown *= .70f;
        stats.damage *= 1.06f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.42f;
        stats.range *= 1.08f;
    }
    return stats;
}

void PulseTower::fire(Enemy& target, std::vector<std::unique_ptr<Enemy>>&,
                      std::vector<Projectile>& projectiles, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    projectiles.push_back({ProjectileKind::Pulse, pos_, target.id(), towerStats.projectileSpeed,
                           towerStats.damage, 0, 1, 0, sf::Color(84, 224, 255), 4, true});
    fx.tracer(pos_, pos_ + normalize(target.position() - pos_) * 26.f, sf::Color(150, 245, 255), .06f, 2);
    assets.play("laser", 36);
}

CannonTower::CannonTower(sf::Vector2f position) : Tower(TowerKind::Cannon, position) {}

TowerStats CannonTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 70,
        .range = 185,
        .cooldown = 1.35f,
        .projectileSpeed = 410,
        .splash = 72,
        .slowFactor = 1,
        .slowDuration = 0,
        .chains = 1,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.splash *= 1.5f;
        stats.damage *= .94f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.52f;
        stats.cooldown *= 1.10f;
    }
    return stats;
}

void CannonTower::fire(Enemy& target, std::vector<std::unique_ptr<Enemy>>&,
                       std::vector<Projectile>& projectiles, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    projectiles.push_back({ProjectileKind::Shell, pos_, target.id(), towerStats.projectileSpeed,
                           towerStats.damage, towerStats.splash, 1, 0, sf::Color(255, 178, 76), 4, true});
    fx.burst(pos_, sf::Color(255, 190, 90), 6, 70);
    assets.play("shoot", 42);
}

FrostTower::FrostTower(sf::Vector2f position) : Tower(TowerKind::Frost, position) {}

TowerStats FrostTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 16,
        .range = 165,
        .cooldown = .62f,
        .projectileSpeed = 620,
        .splash = 0,
        .slowFactor = .68f,
        .slowDuration = 1.7f,
        .chains = 1,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.slowFactor = .48f;
        stats.slowDuration = 2.7f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.55f;
        stats.slowFactor = .62f;
    }
    return stats;
}

void FrostTower::fire(Enemy& target, std::vector<std::unique_ptr<Enemy>>&,
                      std::vector<Projectile>& projectiles, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    projectiles.push_back({ProjectileKind::Frost, pos_, target.id(), towerStats.projectileSpeed,
                           towerStats.damage, 0, towerStats.slowFactor, towerStats.slowDuration,
                           sf::Color(132, 238, 255), 4, true});
    fx.burst(pos_, sf::Color(145, 240, 255), 7, 55.f);
    assets.play("laser", 24);
}

SniperTower::SniperTower(sf::Vector2f position) : Tower(TowerKind::Sniper, position) {}

TowerStats SniperTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 148,
        .range = 330,
        .cooldown = 1.85f,
        .projectileSpeed = 0,
        .splash = 0,
        .slowFactor = 1,
        .slowDuration = 0,
        .chains = 1,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.cooldown *= .68f;
        stats.damage *= .96f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.72f;
        stats.cooldown *= 1.08f;
    }
    return stats;
}

void SniperTower::fire(Enemy& target, std::vector<std::unique_ptr<Enemy>>&,
                       std::vector<Projectile>&, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    const float dealt = target.takeDamage(towerStats.damage);
    fx.ring(pos_, sf::Color(210, 135, 255), 34.f, .18f);
    fx.tracer(pos_, target.position(), sf::Color(220, 154, 255), .16f, 3.5f);
    fx.burst(target.position(), sf::Color(225, 170, 255), 10, 100);
    fx.text(target.position() + sf::Vector2f(8, -30), "-" + std::to_string(int(dealt)),
            sf::Color(240, 190, 255));
    fx.shake(1.6f);
    assets.play("shoot", 50);
}

TeslaTower::TeslaTower(sf::Vector2f position) : Tower(TowerKind::Tesla, position) {}

TowerStats TeslaTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 41,
        .range = 185,
        .cooldown = .92f,
        .projectileSpeed = 0,
        .splash = 0,
        .slowFactor = 1,
        .slowDuration = 0,
        .chains = 3,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.chains += 2;
        stats.damage *= .92f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.38f;
        stats.chains -= 1;
    }
    return stats;
}

void TeslaTower::fire(Enemy& first, std::vector<std::unique_ptr<Enemy>>& enemies,
                      std::vector<Projectile>&, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    Enemy* current = &first;
    sf::Vector2f from = pos_;
    std::vector<int> hit;
    for (int chain = 0; chain < towerStats.chains && current; ++chain) {
        const float multiplier = std::pow(.82f, float(chain));
        current->takeDamage(towerStats.damage * multiplier);
        fx.tracer(from, current->position(), sf::Color(103, 255, 208), .15f, 3);
        fx.burst(current->position(), sf::Color(120, 255, 218), 5, 60);
        hit.push_back(current->id());
        from = current->position();
        Enemy* next = nullptr;
        float bestDistance = 120.f;
        for (auto& enemy : enemies) {
            if (enemy->dead() || enemy->reachedEnd() ||
                std::find(hit.begin(), hit.end(), enemy->id()) != hit.end())
                continue;
            const float candidateDistance = distance(from, enemy->position());
            if (candidateDistance < bestDistance) {
                bestDistance = candidateDistance;
                next = enemy.get();
            }
        }
        current = next;
    }
    assets.play("laser", 48);
}

MissileTower::MissileTower(sf::Vector2f position) : Tower(TowerKind::Missile, position) {}

TowerStats MissileTower::stats() const {
    auto stats = scaled(TowerStats{
        .damage = 112,
        .range = 230,
        .cooldown = 1.75f,
        .projectileSpeed = 330,
        .splash = 94,
        .slowFactor = 1,
        .slowDuration = 0,
        .chains = 1,
    });
    if (branch_ == UpgradeBranch::A) {
        stats.cooldown *= .68f;
        stats.damage *= .9f;
        stats.splash *= .88f;
    }
    if (branch_ == UpgradeBranch::B) {
        stats.damage *= 1.62f;
        stats.splash *= 1.3f;
        stats.cooldown *= 1.13f;
    }
    return stats;
}

void MissileTower::fire(Enemy& target, std::vector<std::unique_ptr<Enemy>>&,
                        std::vector<Projectile>& projectiles, Effects& fx, Assets& assets) {
    const auto towerStats = stats();
    projectiles.push_back({ProjectileKind::Missile, pos_, target.id(), towerStats.projectileSpeed,
                           towerStats.damage, towerStats.splash, 1, 0, sf::Color(255, 105, 78), 5, true});
    fx.burst(pos_, sf::Color(255, 130, 85), 9, 90);
    assets.play("shoot", 44);
}

std::unique_ptr<Tower> makeTower(TowerKind kind, sf::Vector2f pos) {
    switch (kind) {
        case TowerKind::Pulse: return std::make_unique<PulseTower>(pos);
        case TowerKind::Cannon: return std::make_unique<CannonTower>(pos);
        case TowerKind::Frost: return std::make_unique<FrostTower>(pos);
        case TowerKind::Sniper: return std::make_unique<SniperTower>(pos);
        case TowerKind::Tesla: return std::make_unique<TeslaTower>(pos);
        case TowerKind::Missile: return std::make_unique<MissileTower>(pos);
    }
    return std::make_unique<PulseTower>(pos);
}
