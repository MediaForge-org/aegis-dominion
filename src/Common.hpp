#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

constexpr unsigned WINDOW_W = 1600;
constexpr unsigned WINDOW_H = 900;
constexpr float WORLD_W = 1200.f;
constexpr float WORLD_H = 900.f;
constexpr float PANEL_X = 1200.f;
constexpr float PANEL_W = 400.f;
constexpr float PI_F = 3.14159265358979323846f;

enum class TowerKind { Pulse, Cannon, Frost, Sniper, Tesla, Missile };
enum class EnemyKind { Raider, Runner, Tank, Shield, Regen, Splitter, Boss };
enum class TargetMode { First, Strongest, Nearest, Last };
enum class UpgradeBranch { None, A, B };
enum class ProjectileKind { Pulse, Shell, Frost, Sniper, Missile };

inline float length(sf::Vector2f v) { return std::sqrt(v.x*v.x + v.y*v.y); }
inline float distance(sf::Vector2f a, sf::Vector2f b) { return length(a-b); }
inline sf::Vector2f normalize(sf::Vector2f v) {
    float l = length(v); return l > 0.0001f ? v/l : sf::Vector2f{};
}
inline float angleDeg(sf::Vector2f v) { return std::atan2(v.y, v.x) * 180.f / PI_F + 90.f; }
inline float clampf(float v,float a,float b){return std::max(a,std::min(b,v));}
inline sf::Color withAlpha(sf::Color c, sf::Uint8 a){ c.a=a; return c; }
inline bool pointIn(const sf::FloatRect& r, sf::Vector2f p){ return r.contains(p); }

inline std::string towerName(TowerKind k) {
    switch(k){
        case TowerKind::Pulse: return "Pulsar";
        case TowerKind::Cannon: return "Mörser";
        case TowerKind::Frost: return "Kryo";
        case TowerKind::Sniper: return "Railgun";
        case TowerKind::Tesla: return "Tesla";
        case TowerKind::Missile: return "Raketen";
    }
    return "Turm";
}
inline std::string towerShort(TowerKind k) {
    switch(k){
        case TowerKind::Pulse: return "Schneller Allrounder";
        case TowerKind::Cannon: return "Explosiver Flächenschaden";
        case TowerKind::Frost: return "Verlangsamt Gegner";
        case TowerKind::Sniper: return "Extrem hohe Reichweite";
        case TowerKind::Tesla: return "Kettenblitz";
        case TowerKind::Missile: return "Große Explosionen";
    }
    return "";
}
inline int towerCost(TowerKind k) {
    switch(k){
        case TowerKind::Pulse: return 90;
        case TowerKind::Cannon: return 145;
        case TowerKind::Frost: return 120;
        case TowerKind::Sniper: return 175;
        case TowerKind::Tesla: return 165;
        case TowerKind::Missile: return 210;
    }
    return 100;
}
inline std::string targetModeName(TargetMode m){
    switch(m){
        case TargetMode::First: return "Vorne";
        case TargetMode::Strongest: return "Stärkster";
        case TargetMode::Nearest: return "Nächster";
        case TargetMode::Last: return "Hinten";
    }
    return "Vorne";
}
inline std::string enemyName(EnemyKind k){
    switch(k){
        case EnemyKind::Raider: return "Plünderer";
        case EnemyKind::Runner: return "Jäger";
        case EnemyKind::Tank: return "Brecher";
        case EnemyKind::Shield: return "Schildträger";
        case EnemyKind::Regen: return "Regenerator";
        case EnemyKind::Splitter: return "Splitter";
        case EnemyKind::Boss: return "Titan";
    }
    return "Feind";
}
