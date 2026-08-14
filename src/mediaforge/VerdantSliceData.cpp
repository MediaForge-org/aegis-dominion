#include "VerdantSliceData.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace aegis::mediaforge {
namespace {

render::RenderVec2 sampleRoute(float progress) {
    constexpr std::array<render::RenderVec2, 7> route{{
        {40, 318}, {250, 320}, {430, 360}, {610, 565}, {790, 608}, {1040, 520}, {1240, 590},
    }};
    const float scaled = std::clamp(progress, 0.0F, 0.999F) * static_cast<float>(route.size() - 1);
    const auto segment = static_cast<std::size_t>(scaled);
    const float local = scaled - static_cast<float>(segment);
    return {route[segment].x + (route[segment + 1].x - route[segment].x) * local,
            route[segment].y + (route[segment + 1].y - route[segment].y) * local};
}

float routeRotation(float progress) {
    const auto before = sampleRoute(std::max(0.0F, progress - 0.005F));
    const auto after = sampleRoute(std::min(0.999F, progress + 0.005F));
    return std::atan2(after.y - before.y, after.x - before.x);
}

} // namespace

render::WorldRenderSnapshot makeVerdantSliceSnapshot(float elapsedSeconds) {
    render::WorldRenderSnapshot snapshot;
    snapshot.elapsedSeconds = elapsedSeconds;
    snapshot.map.id = "e2_verdant_reference";
    snapshot.map.biome = "verdant";
    snapshot.map.authoredBackgroundId = "terrain.verdant.composed";
    snapshot.map.authoredTerrain = true;
    snapshot.map.spawn = {84, 318};
    snapshot.map.goal = {1166, 586};
    snapshot.map.accent = {59, 205, 220, 255};
    snapshot.map.ambientIntensity = 0.78F;
    snapshot.map.terrainSeed = 0xE2A615U;
    snapshot.map.path = {{40, 318}, {250, 320}, {430, 360}, {610, 565}, {790, 608}, {1040, 520}, {1240, 590}};

    const std::array<float, 7> progresses{0.12F, 0.22F, 0.34F, 0.45F, 0.57F, 0.69F, 0.78F};
    int id = 1;
    for (const float base : progresses) {
        const float progress = std::fmod(base + elapsedSeconds * 0.018F, 0.84F);
        render::EnemyRenderSnapshot enemy;
        enemy.id = id++;
        enemy.position = sampleRoute(progress);
        enemy.rotationDeg = routeRotation(progress) * 180.0F / 3.14159265F + 90.0F;
        enemy.scale = 1.0F;
        enemy.visualId = "enemy.raider";
        enemy.animationTime = elapsedSeconds + base * 7.0F;
        enemy.healthVisible = progress > 0.46F;
        enemy.healthRatio = 0.72F + std::sin(base * 19.0F) * 0.18F;
        snapshot.enemies.push_back(std::move(enemy));
    }
    render::EnemyRenderSnapshot heavy;
    const float heavyProgress = std::fmod(0.29F + elapsedSeconds * 0.009F, 0.76F);
    heavy.id = id;
    heavy.position = sampleRoute(heavyProgress);
    heavy.rotationDeg = routeRotation(heavyProgress) * 180.0F / 3.14159265F + 90.0F;
    heavy.visualId = "enemy.heavy_tank";
    heavy.animationTime = elapsedSeconds;
    heavy.healthVisible = true;
    heavy.healthRatio = 0.86F;
    snapshot.enemies.push_back(std::move(heavy));

    render::TowerRenderSnapshot pulse;
    pulse.position = {520, 330};
    pulse.turretRotationDeg = 32.0F + std::sin(elapsedSeconds * 0.7F) * 16.0F;
    pulse.idlePhase = elapsedSeconds;
    pulse.recoil = std::max(0.0F, 1.0F - std::fmod(elapsedSeconds, 0.72F) * 9.0F);
    pulse.baseVisualId = "tower.pulse.base";
    pulse.turretVisualId = "tower.pulse.turret";
    pulse.effectProfile = "pulse";
    snapshot.towers.push_back(std::move(pulse));
    render::TowerRenderSnapshot rail;
    rail.position = {845, 710};
    rail.turretRotationDeg = -24.0F + std::sin(elapsedSeconds * 0.34F) * 8.0F;
    rail.idlePhase = elapsedSeconds;
    rail.recoil = std::max(0.0F, 1.0F - std::fmod(elapsedSeconds, 2.8F) * 5.0F);
    rail.baseVisualId = "tower.railgun.base";
    rail.turretVisualId = "tower.railgun.turret";
    rail.effectProfile = "rail";
    rail.selected = true;
    snapshot.towers.push_back(std::move(rail));

    render::ProjectileRenderSnapshot bolt;
    bolt.position = {665.0F + std::fmod(elapsedSeconds * 250.0F, 180.0F), 470.0F};
    bolt.rotationDeg = -9.0F;
    bolt.visualId = "vfx.pulse_bolt";
    bolt.effectProfile = "pulse";
    bolt.color = {100, 225, 255, 255};
    snapshot.projectiles.push_back(std::move(bolt));
    return snapshot;
}

std::vector<SliceVisual> translateVerdantSlice(const render::WorldRenderSnapshot& snapshot) {
    std::vector<SliceVisual> visuals;
    visuals.reserve(snapshot.enemies.size() * 3 + snapshot.towers.size() * 4 + snapshot.projectiles.size() * 2 + 8);
    visuals.push_back({"world.spawn_gate", snapshot.map.spawn, {245, 164}, 0, 1, 40, mf::BlendMode::alpha, SliceVisualKind::entity});
    visuals.push_back({"world.aegis_core", snapshot.map.goal, {235, 157}, 0, 1, 40, mf::BlendMode::alpha, SliceVisualKind::entity});
    for (const auto& enemy : snapshot.enemies) {
        const bool heavy = enemy.visualId == "enemy.heavy_tank";
        visuals.push_back({"vfx.soft_shadow", {enemy.position.x + 10, enemy.position.y + 13},
            heavy ? render::RenderVec2{118, 64} : render::RenderVec2{76, 42}, enemy.rotationDeg * 0.0174532925F,
            heavy ? 0.72F : 0.58F, 50, mf::BlendMode::alpha, SliceVisualKind::shadow});
        visuals.push_back({enemy.visualId, enemy.position, heavy ? render::RenderVec2{142, 142} : render::RenderVec2{92, 92},
            enemy.rotationDeg * 0.0174532925F, 1, 60, mf::BlendMode::alpha, SliceVisualKind::entity});
    }
    for (const auto& tower : snapshot.towers) {
        const bool rail = tower.effectProfile == "rail";
        visuals.push_back({"vfx.soft_shadow", {tower.position.x + 12, tower.position.y + 14}, {150, 74}, 0, 0.72F,
                           50, mf::BlendMode::alpha, SliceVisualKind::shadow});
        visuals.push_back({tower.baseVisualId, tower.position, rail ? render::RenderVec2{168, 190} : render::RenderVec2{158, 158},
                           0, 1, 70, mf::BlendMode::alpha, SliceVisualKind::entity});
        const float rotation = tower.turretRotationDeg * 0.0174532925F;
        const float recoilDistance = tower.recoil * (rail ? 14.0F : 7.0F);
        const render::RenderVec2 headPosition{tower.position.x + std::sin(rotation) * recoilDistance,
                                              tower.position.y - std::cos(rotation) * recoilDistance};
        visuals.push_back({tower.turretVisualId, headPosition, rail ? render::RenderVec2{88, 193} : render::RenderVec2{92, 160},
                           rotation, 1, 71, mf::BlendMode::alpha, SliceVisualKind::entity});
    }
    for (const auto& projectile : snapshot.projectiles) {
        visuals.push_back({projectile.visualId, projectile.position, {118, 38}, projectile.rotationDeg * 0.0174532925F,
                           1, 80, mf::BlendMode::additive, SliceVisualKind::projectile});
    }
    return visuals;
}

std::vector<SliceVisual> deterministicVerdantDetails(unsigned seed) {
    std::mt19937 random(seed);
    std::uniform_real_distribution<float> rotation(-0.45F, 0.45F);
    std::uniform_real_distribution<float> scale(0.72F, 1.15F);
    const std::array<render::RenderVec2, 12> positions{{
        {170, 178}, {335, 740}, {535, 172}, {710, 205}, {950, 170}, {1115, 275},
        {190, 690}, {385, 805}, {640, 790}, {965, 780}, {1120, 740}, {760, 345},
    }};
    std::vector<SliceVisual> details;
    details.reserve(positions.size());
    for (std::size_t i = 0; i < positions.size(); ++i) {
        const bool crack = i % 3 == 0;
        details.push_back({crack ? "decal.crack" : "decal.scorch", positions[i],
                           crack ? render::RenderVec2{82, 55} : render::RenderVec2{64, 43}, rotation(random),
                           0.16F + scale(random) * 0.11F, 15, mf::BlendMode::alpha, SliceVisualKind::decal});
    }
    return details;
}

} // namespace aegis::mediaforge
