#pragma once

#include "core/MapDocument.hpp"
#include "render/RenderLayer.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace aegis::render {

struct RenderVec2 { float x = 0.f; float y = 0.f; };
struct RenderRect { float x = 0.f; float y = 0.f; float width = 0.f; float height = 0.f; };
struct RenderColor { std::uint8_t r = 255, g = 255, b = 255, a = 255; };

enum class AnimationState { Spawn, Move, Hit, Death, Idle, Fire, Ability, Stun, ShieldBreak };

struct EnemyRenderSnapshot {
    int id = 0;
    RenderVec2 position;
    float rotationDeg = 0.f;
    float scale = 1.f;
    std::string visualId;
    AnimationState animation = AnimationState::Move;
    float animationTime = 0.f;
    float healthRatio = 1.f;
    float shieldRatio = 0.f;
    bool healthVisible = false;
    bool boss = false;
    bool hitFlash = false;
    Layer layer = Layer::Enemies;
};

struct TowerRenderSnapshot {
    RenderVec2 position;
    float turretRotationDeg = 0.f;
    float scale = 1.f;
    float range = 0.f;
    float recoil = 0.f;
    float idlePhase = 0.f;
    int level = 1;
    int branch = 0;
    std::string baseVisualId;
    std::string turretVisualId;
    std::string effectProfile;
    bool selected = false;
    Layer layer = Layer::Towers;
};

struct ProjectileRenderSnapshot {
    RenderVec2 position;
    float rotationDeg = 0.f;
    float scale = 1.f;
    std::string visualId;
    std::string effectProfile;
    RenderColor color;
    Layer layer = Layer::Projectiles;
};

struct ZoneRenderSnapshot { core::ZoneType type = core::ZoneType::Buildable; RenderRect rect; };
struct DecorationRenderSnapshot {
    std::string visualId;
    RenderVec2 position;
    float rotationDeg = 0.f;
    float scale = 1.f;
    int variant = 0;
    Layer layer = Layer::Environment;
};

struct MapRenderSnapshot {
    std::string id;
    std::string biome;
    std::string authoredBackgroundId;
    RenderColor accent;
    std::vector<RenderVec2> path;
    RenderVec2 spawn;
    RenderVec2 goal;
    std::vector<ZoneRenderSnapshot> zones;
    std::vector<DecorationRenderSnapshot> decorations;
    unsigned terrainSeed = 1;
    float ambientIntensity = 1.f;
    bool authoredTerrain = false;
    bool buildMode = false;
    bool debugZones = false;
};

struct WorldRenderSnapshot {
    MapRenderSnapshot map;
    std::vector<EnemyRenderSnapshot> enemies;
    std::vector<TowerRenderSnapshot> towers;
    std::vector<ProjectileRenderSnapshot> projectiles;
    float elapsedSeconds = 0.f;
};

} // namespace aegis::render
