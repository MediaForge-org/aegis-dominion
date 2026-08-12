#include "WorldRenderer.hpp"

#include "Assets.hpp"
#include "Common.hpp"
#include "WorldRenderData.hpp"
#include "assets/AssetId.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace aegis::render {
namespace {

sf::Vector2f vector(RenderVec2 value) { return {value.x, value.y}; }
sf::Color color(RenderColor value) { return {value.r, value.g, value.b, value.a}; }
sf::Vector2f normal(sf::Vector2f value) {
    const float magnitude = std::hypot(value.x, value.y);
    return magnitude > .001f ? sf::Vector2f{-value.y / magnitude, value.x / magnitude} : sf::Vector2f{};
}

void shadow(sf::RenderTarget& target, sf::Vector2f position, float radius, float squash = .42f) {
    sf::CircleShape shape(radius, 32);
    shape.setOrigin(radius, radius);
    shape.setPosition(position + sf::Vector2f{9.f, 12.f});
    shape.setScale(1.f, squash);
    shape.setFillColor(sf::Color(0, 0, 0, 92));
    target.draw(shape);
}

void glow(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color tint, float strength = 1.f) {
    sf::VertexArray fan(sf::TriangleFan, 34);
    tint.a = static_cast<sf::Uint8>(std::clamp(62.f * strength, 0.f, 110.f));
    fan[0] = sf::Vertex(position, tint);
    tint.a = 0;
    for (std::size_t i = 1; i < fan.getVertexCount(); ++i) {
        const float angle = static_cast<float>(i - 1) / 32.f * 2.f * PI_F;
        fan[i] = sf::Vertex(position + sf::Vector2f{std::cos(angle) * radius, std::sin(angle) * radius}, tint);
    }
    target.draw(fan, sf::BlendAdd);
}

sf::VertexArray roadStrip(const std::vector<RenderVec2>& points, float width, sf::Color tint) {
    sf::VertexArray strip(sf::TriangleStrip);
    if (points.size() < 2) return strip;
    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto before = vector(points[i == 0 ? i : i - 1]);
        const auto after = vector(points[i + 1 < points.size() ? i + 1 : i]);
        const auto n = normal(after - before) * (width * .5f);
        strip.append(sf::Vertex(vector(points[i]) + n, tint));
        strip.append(sf::Vertex(vector(points[i]) - n, tint));
    }
    return strip;
}

void spriteCentered(sf::RenderTarget& target, const sf::Texture& texture, sf::Vector2f position,
                    float scale, float rotation = 0.f, sf::Color tint = sf::Color::White) {
    sf::Sprite sprite(texture);
    const auto bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    sprite.setPosition(position); sprite.setScale(scale, scale); sprite.setRotation(rotation); sprite.setColor(tint);
    target.draw(sprite);
}

} // namespace

void WorldRenderer::draw(const WorldRenderSnapshot& snapshot) {
    context_.setLayer(Layer::Terrain); drawTerrain(snapshot.map);
    context_.setLayer(Layer::Water); drawWater(snapshot.map, snapshot.elapsedSeconds);
    context_.setLayer(Layer::Road); drawRoad(snapshot.map);
    context_.setLayer(Layer::Environment); drawEnvironment(snapshot.map);
    context_.setLayer(Layer::Zones); drawZones(snapshot.map);
    drawEndpoints(snapshot.map, snapshot.elapsedSeconds);
    context_.setLayer(Layer::Enemies); drawEnemies(snapshot.enemies);
    context_.setLayer(Layer::Towers); drawTowers(snapshot.towers);
    context_.setLayer(Layer::Projectiles); drawProjectiles(snapshot.projectiles);
    context_.setLayer(Layer::Effects); drawLighting(snapshot);
}

void WorldRenderer::drawTerrain(const MapRenderSnapshot& map) {
    auto& target = context_.target();
    const std::string id = map.authoredTerrain ? map.authoredBackgroundId : terrainTextureId(terrainMaterialForBiome(map.biome));
    const auto& texture = assets_.resources().texture(assets::TextureId{id});
    sf::Sprite terrain(texture);
    if (map.authoredTerrain) {
        terrain.setPosition(0.f, 0.f);
    } else {
        terrain.setTextureRect({0, 0, static_cast<int>(WORLD_W), static_cast<int>(WORLD_H)});
        const float phaseX = static_cast<float>((map.terrainSeed * 73u) % 251u);
        const float phaseY = static_cast<float>((map.terrainSeed * 41u) % 193u);
        terrain.setPosition(-phaseX, -phaseY);
        terrain.setTextureRect({0, 0, static_cast<int>(WORLD_W + phaseX), static_cast<int>(WORLD_H + phaseY)});
    }
    target.draw(terrain);

    if (!map.authoredTerrain) {
        // Deterministic broad patches break visible tile repetition without per-frame allocation or noise.
        std::uint32_t state = map.terrainSeed ? map.terrainSeed : 1u;
        for (int i = 0; i < 28; ++i) {
            state = state * 1664525u + 1013904223u;
            const float x = static_cast<float>(state % 1200u);
            state = state * 1664525u + 1013904223u;
            const float y = static_cast<float>(state % 900u);
            sf::CircleShape patch(45.f + static_cast<float>((state >> 8u) % 80u), 22);
            patch.setOrigin(patch.getRadius(), patch.getRadius()); patch.setPosition(x, y); patch.setScale(1.8f, .65f);
            patch.setFillColor(sf::Color(8, 14, 17, static_cast<sf::Uint8>(8 + state % 13u)));
            target.draw(patch, sf::BlendMultiply);
        }
    }
}

void WorldRenderer::drawWater(const MapRenderSnapshot& map, float elapsed) {
    auto& target = context_.target();
    for (const auto& zone : map.zones) {
        if (zone.type != core::ZoneType::Water) continue;
        const auto r = zone.rect;
        sf::RectangleShape deep({r.width, r.height}); deep.setPosition(r.x, r.y);
        deep.setFillColor(sf::Color(14, 63, 83, 235)); deep.setOutlineColor(sf::Color(76, 184, 202, 165)); deep.setOutlineThickness(5.f);
        target.draw(deep);
        const float spacing = 18.f;
        for (float y = r.y + 10.f; y < r.y + r.height - 5.f; y += spacing) {
            sf::VertexArray wave(sf::LineStrip);
            for (float x = r.x + 4.f; x <= r.x + r.width - 4.f; x += 14.f) {
                const float offset = std::sin(x * .035f + y * .021f + elapsed * 1.7f) * 3.2f;
                wave.append(sf::Vertex({x, y + offset}, sf::Color(104, 218, 226, 58)));
            }
            target.draw(wave, sf::BlendAdd);
        }
        sf::RectangleShape shore({r.width - 8.f, r.height - 8.f}); shore.setPosition(r.x + 4.f, r.y + 4.f);
        shore.setFillColor(sf::Color::Transparent); shore.setOutlineColor(sf::Color(171, 224, 207, 75)); shore.setOutlineThickness(2.f); target.draw(shore);
    }
}

void WorldRenderer::drawRoad(const MapRenderSnapshot& map) {
    if (map.path.size() < 2) return;
    auto& target = context_.target();
    target.draw(roadStrip(map.path, 96.f, sf::Color(3, 7, 9, 105)));
    const auto material = terrainMaterialForBiome(map.biome);
    const sf::Color shoulder = material == TerrainMaterial::Snow ? sf::Color(111, 130, 139) :
                                     material == TerrainMaterial::Ash ? sf::Color(74, 55, 48) : sf::Color(93, 82, 59);
    target.draw(roadStrip(map.path, 86.f, shoulder));
    target.draw(roadStrip(map.path, 66.f, sf::Color(44, 49, 51)));
    target.draw(roadStrip(map.path, 58.f, sf::Color(54, 58, 59)));

    float carry = 0.f;
    for (std::size_t i = 0; i + 1 < map.path.size(); ++i) {
        const auto a = vector(map.path[i]), b = vector(map.path[i + 1]);
        const auto delta = b - a;
        const float segmentLength = std::hypot(delta.x, delta.y);
        if (segmentLength < .001f) continue;
        const auto direction = delta / segmentLength;
        for (float d = 34.f - carry; d < segmentLength; d += 68.f) {
            const auto center = a + direction * d;
            sf::RectangleShape mark({25.f, 3.f}); mark.setOrigin(12.5f, 1.5f); mark.setPosition(center);
            mark.setRotation(std::atan2(delta.y, delta.x) * 180.f / PI_F); mark.setFillColor(sf::Color(map.accent.r, map.accent.g, map.accent.b, 118));
            target.draw(mark);
        }
        carry = std::fmod(carry + segmentLength, 68.f);
    }
}

void WorldRenderer::drawEnvironment(const MapRenderSnapshot& map) {
    auto decorations = generateProceduralDecorations(map);
    std::sort(decorations.begin(), decorations.end(), [](const auto& a, const auto& b) { return a.position.y < b.position.y; });
    for (const auto& decoration : decorations) {
        const auto position = vector(decoration.position);
        shadow(context_.target(), position, 25.f * decoration.scale);
        const auto& texture = assets_.resources().texture(assets::TextureId{decoration.visualId});
        spriteCentered(context_.target(), texture, position, .62f * decoration.scale, decoration.rotationDeg);
    }
}

void WorldRenderer::drawZones(const MapRenderSnapshot& map) {
    for (const auto& zone : map.zones) {
        if (!zoneVisibleInGameplay(zone.type, map.buildMode, map.debugZones) || zone.type == core::ZoneType::Water) continue;
        sf::RectangleShape shape({zone.rect.width, zone.rect.height}); shape.setPosition(zone.rect.x, zone.rect.y);
        const bool buildable = zone.type == core::ZoneType::Buildable;
        shape.setFillColor(buildable ? sf::Color(65, 220, 150, 17) : sf::Color(255, 85, 90, 22));
        shape.setOutlineColor(buildable ? sf::Color(90, 235, 175, 95) : sf::Color(255, 95, 110, 110)); shape.setOutlineThickness(2.f);
        context_.target().draw(shape);
    }
}

void WorldRenderer::drawEndpoints(const MapRenderSnapshot& map, float elapsed) {
    const float pulse = .92f + std::sin(elapsed * 3.1f) * .06f;
    shadow(context_.target(), vector(map.spawn), 49.f);
    glow(context_.target(), vector(map.spawn), 88.f, color(map.accent), .9f);
    spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{"world.spawn_gate"}), vector(map.spawn), .72f * pulse, elapsed * 9.f);
    shadow(context_.target(), vector(map.goal), 58.f);
    glow(context_.target(), vector(map.goal), 104.f, color(map.accent), 1.15f);
    spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{"world.aegis_core"}), vector(map.goal), .8f, 0.f);
    spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{"world.aegis_core_energy"}), vector(map.goal), .7f * pulse, -elapsed * 13.f,
                   sf::Color(map.accent.r, map.accent.g, map.accent.b));
}

void WorldRenderer::drawEnemies(const std::vector<EnemyRenderSnapshot>& enemies) {
    for (const auto& enemy : enemies) {
        const auto position = vector(enemy.position);
        const auto suffix=enemy.animation==AnimationState::Spawn?"spawn":enemy.animation==AnimationState::Hit?"hit":"move";
        const auto separator=enemy.visualId.rfind(".idle");const auto clipId=enemy.visualId.substr(0,separator)+"."+suffix;
        const auto* clip=assets_.animations().find(clipId);
        float clipDuration=0.f;
        if(clip)for(const auto& frame:clip->animation.frames)clipDuration+=frame.duration;
        if(clipDuration<=0.f)clipDuration=.35f;
        const float spawnScale = enemy.animation == AnimationState::Spawn ? std::clamp(enemy.animationTime / clipDuration, .15f, 1.f) : 1.f;
        const float cadence=clip&&!clip->animation.frames.empty()?1.f/std::max(.04f,clip->animation.frames.front().duration):7.f;
        const float hover = std::sin(enemy.animationTime * cadence + static_cast<float>(enemy.id)) * (enemy.visualId.find("runner") != std::string::npos ? 2.5f : 1.2f);
        shadow(context_.target(), position, (enemy.boss ? 44.f : 29.f) * enemy.scale);
        if (enemy.shieldRatio > 0.f) glow(context_.target(), position, (enemy.boss ? 62.f : 43.f) * enemy.scale, sf::Color(80, 175, 255), .7f);
        auto tint = enemy.hitFlash ? sf::Color(255, 220, 220) : sf::Color::White;
        spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{enemy.visualId}),
                       position + sf::Vector2f{0.f, hover}, enemy.scale * spawnScale, enemy.rotationDeg, tint);
        if (enemy.healthVisible) {
            const float width = enemy.boss ? 84.f : 52.f;
            const float y = position.y - (enemy.boss ? 58.f : 44.f);
            sf::RectangleShape frame({width, 7.f}); frame.setOrigin(width / 2.f, 3.5f); frame.setPosition(position.x, y);
            frame.setFillColor(sf::Color(4, 8, 12, 220)); frame.setOutlineColor(sf::Color(145, 168, 180, 130)); frame.setOutlineThickness(1.f); context_.target().draw(frame);
            sf::RectangleShape hp({std::max(0.f, width - 4.f) * enemy.healthRatio, 3.f}); hp.setOrigin((width - 4.f) / 2.f, 1.5f); hp.setPosition(position.x, y);
            hp.setFillColor(sf::Color(90, 225, 135)); context_.target().draw(hp);
            if (enemy.shieldRatio > 0.f) {
                sf::RectangleShape shieldBar({(width - 4.f) * enemy.shieldRatio, 2.f}); shieldBar.setOrigin((width - 4.f) / 2.f, 1.f);
                shieldBar.setPosition(position.x, y + 6.f); shieldBar.setFillColor(sf::Color(85, 188, 255)); context_.target().draw(shieldBar);
            }
        }
    }
}

void WorldRenderer::drawTowers(const std::vector<TowerRenderSnapshot>& towers) {
    for (const auto& tower : towers) {
        const auto position = vector(tower.position);
        if (tower.selected) {
            sf::CircleShape range(tower.range, 96); range.setOrigin(tower.range, tower.range); range.setPosition(position);
            range.setFillColor(sf::Color(75, 195, 255, 13)); range.setOutlineColor(sf::Color(110, 220, 255, 75)); range.setOutlineThickness(2.f); context_.target().draw(range);
        }
        shadow(context_.target(), position, 42.f * tower.scale);
        spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{tower.baseVisualId}), position, tower.scale);
        const float angle = (tower.turretRotationDeg - 90.f) * PI_F / 180.f;
        const sf::Vector2f recoilOffset{-std::cos(angle) * tower.recoil * 8.f, -std::sin(angle) * tower.recoil * 8.f};
        spriteCentered(context_.target(), assets_.resources().texture(assets::TextureId{tower.turretVisualId}), position + recoilOffset, tower.scale, tower.turretRotationDeg);
        const sf::Color energy = tower.effectProfile == "cryo" ? sf::Color(110, 230, 255) :
                                 tower.effectProfile == "tesla" ? sf::Color(95, 255, 205) :
                                 tower.effectProfile == "rail" ? sf::Color(215, 130, 255) : sf::Color(255, 145, 78);
        if (tower.effectProfile != "kinetic") glow(context_.target(), position, 34.f + std::sin(tower.idlePhase) * 3.f, energy, .32f);
        for (int i = 0; i < tower.level; ++i) {
            sf::CircleShape pip(2.8f); pip.setOrigin(2.8f, 2.8f); pip.setPosition(position.x - 11.f + static_cast<float>(i) * 7.f, position.y + 40.f);
            pip.setFillColor(tower.branch == 1 ? sf::Color(92, 224, 255) : tower.branch == 2 ? sf::Color(255, 176, 92) : sf::Color(220, 230, 238)); context_.target().draw(pip);
        }
    }
}

void WorldRenderer::drawProjectiles(const std::vector<ProjectileRenderSnapshot>& projectiles) {
    for (const auto& projectile : projectiles) {
        const auto position = vector(projectile.position);
        const auto tint = color(projectile.color);
        if (projectile.effectProfile == "missile") {
            glow(context_.target(), position, 22.f, sf::Color(255, 110, 65), .8f);
            sf::RectangleShape body({18.f, 7.f}); body.setOrigin(9.f, 3.5f); body.setPosition(position); body.setRotation(projectile.rotationDeg - 90.f); body.setFillColor(sf::Color(225, 225, 218)); context_.target().draw(body);
        } else if (projectile.effectProfile == "shell") {
            shadow(context_.target(), position, 7.f, .3f); sf::CircleShape shell(5.f); shell.setOrigin(5.f, 5.f); shell.setPosition(position); shell.setFillColor(sf::Color(245, 175, 80)); context_.target().draw(shell);
        } else if (projectile.effectProfile == "cryo") {
            glow(context_.target(), position, 24.f, tint, .9f); sf::ConvexShape shard(4); shard.setPoint(0, {0.f, -9.f}); shard.setPoint(1, {5.f, 0.f}); shard.setPoint(2, {0.f, 9.f}); shard.setPoint(3, {-5.f, 0.f}); shard.setPosition(position); shard.setRotation(projectile.rotationDeg); shard.setFillColor(tint); context_.target().draw(shard);
        } else {
            glow(context_.target(), position, 18.f, tint, .85f); sf::CircleShape bolt(3.8f); bolt.setOrigin(3.8f, 3.8f); bolt.setPosition(position); bolt.setFillColor(tint); context_.target().draw(bolt);
        }
    }
}

void WorldRenderer::drawLighting(const WorldRenderSnapshot& snapshot) {
    const float darkness = std::clamp(1.05f - snapshot.map.ambientIntensity, 0.f, .45f);
    if (darkness > .01f) {
        sf::RectangleShape ambient({WORLD_W, WORLD_H}); ambient.setFillColor(sf::Color(3, 8, 16, static_cast<sf::Uint8>(darkness * 115.f)));
        context_.target().draw(ambient, sf::BlendMultiply);
    }
    // A subtle edge vignette gives the world depth while leaving the command panel untouched.
    for (int i = 0; i < 5; ++i) {
        sf::RectangleShape frame({WORLD_W - static_cast<float>(i) * 14.f, WORLD_H - static_cast<float>(i) * 14.f}); frame.setPosition(static_cast<float>(i) * 7.f, static_cast<float>(i) * 7.f);
        frame.setFillColor(sf::Color::Transparent); frame.setOutlineColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(18 - i * 2))); frame.setOutlineThickness(8.f); context_.target().draw(frame);
    }
}

} // namespace aegis::render
