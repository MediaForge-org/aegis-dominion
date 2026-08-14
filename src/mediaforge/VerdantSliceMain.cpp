#include "E2AssetCatalog.hpp"
#include "E2Performance.hpp"
#include "VerdantSliceData.hpp"
#include "VerdantShowcase.hpp"

#include <mediaforge/foundation/Config.hpp>
#include <mediaforge/foundation/Log.hpp>
#include <mediaforge/input/Event.hpp>
#include <mediaforge/input/InputState.hpp>
#include <mediaforge/platform/Window.hpp>
#include <mediaforge/render/ParticleSystem2D.hpp>
#include <mediaforge/render/Renderer2D.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <iostream>
#include <random>
#include <string>
#include <variant>
#include <vector>

namespace {

constexpr float referenceWidth = 1600.0F;
constexpr float referenceHeight = 900.0F;

int report(const mf::Error& error) {
    std::cerr << "AEGIS MediaForge E2 slice failed: " << error.message << '\n';
    return 1;
}

mf::Camera2D referenceCamera(float viewportWidth = referenceWidth, float viewportHeight = referenceHeight) {
    mf::Camera2D camera;
    camera.setCenter({referenceWidth * 0.5F, referenceHeight * 0.5F});
    camera.setOrthographicSize({referenceWidth, referenceHeight});
    camera.setViewport({0, 0, viewportWidth, viewportHeight});
    return camera;
}

void submitAsset(mf::Renderer2D& renderer, const aegis::mediaforge::E2AssetCatalog& assets,
                 std::string_view id, mf::Vec2 position, mf::Vec2 size, float rotation = 0.0F,
                 float opacity = 1.0F, mf::BlendMode blend = mf::BlendMode::alpha,
                 std::int32_t layer = 0, std::int32_t order = 0, mf::Color color = {}) {
    const auto* asset = assets.find(id);
    if (!asset) return;
    mf::Sprite2D sprite;
    sprite.texture = asset->texture;
    sprite.sampler = asset->sampler;
    sprite.position = position;
    sprite.size = size;
    sprite.pivot = asset->pivot;
    sprite.rotationRadians = rotation;
    sprite.uv = asset->uv;
    sprite.color = color;
    sprite.opacity = opacity;
    sprite.blend = blend;
    sprite.layer = layer;
    sprite.order = order;
    renderer.submit(sprite);
}

void submitSolid(mf::Renderer2D& renderer, mf::Vec2 position, mf::Vec2 size, mf::Color color,
                 std::int32_t layer, std::int32_t order = 0) {
    mf::Sprite2D sprite;
    sprite.position = position;
    sprite.size = size;
    sprite.color = color;
    sprite.blend = mf::BlendMode::alpha;
    sprite.layer = layer;
    sprite.order = order;
    sprite.space = mf::CoordinateSpace::screen;
    renderer.submit(sprite);
}

void submitNineSlice(mf::Renderer2D& renderer, const aegis::mediaforge::E2AssetCatalog& assets,
                     std::string_view id, mf::Vec2 position, mf::Vec2 size, float border,
                     std::int32_t layer, float opacity = 1.0F) {
    const auto* asset = assets.find(id);
    if (!asset || size.x <= border * 2.0F || size.y <= border * 2.0F) return;
    const std::array<float, 4> x{position.x, position.x + border, position.x + size.x - border, position.x + size.x};
    const std::array<float, 4> y{position.y, position.y + border, position.y + size.y - border, position.y + size.y};
    const float uSpan = asset->uv.right - asset->uv.left;
    const float vSpan = asset->uv.bottom - asset->uv.top;
    const float sourceBorderX = std::min(border / asset->sourceSize.x, 0.32F);
    const float sourceBorderY = std::min(border / asset->sourceSize.y, 0.32F);
    const std::array<float, 4> u{asset->uv.left, asset->uv.left + uSpan * sourceBorderX,
                                 asset->uv.right - uSpan * sourceBorderX, asset->uv.right};
    const std::array<float, 4> v{asset->uv.top, asset->uv.top + vSpan * sourceBorderY,
                                 asset->uv.bottom - vSpan * sourceBorderY, asset->uv.bottom};
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 3; ++column) {
            mf::Sprite2D sprite;
            sprite.texture = asset->texture;
            sprite.sampler = asset->sampler;
            sprite.position = {(x[column] + x[column + 1]) * 0.5F, (y[row] + y[row + 1]) * 0.5F};
            sprite.size = {x[column + 1] - x[column], y[row + 1] - y[row]};
            sprite.uv = {u[column], v[row], u[column + 1], v[row + 1]};
            sprite.opacity = opacity;
            sprite.layer = layer;
            sprite.order = 0;
            sprite.space = mf::CoordinateSpace::screen;
            renderer.submit(sprite);
        }
    }
}

void spawnImpactParticles(mf::ParticleSystem2D& particles, std::mt19937& random) {
    std::uniform_real_distribution<float> angle(0.0F, 6.2831853F);
    std::uniform_real_distribution<float> speed(35.0F, 180.0F);
    std::uniform_real_distribution<float> jitter(-12.0F, 12.0F);
    for (int i = 0; i < 28; ++i) {
        const float direction = angle(random);
        const float velocity = speed(random);
        mf::Particle2D particle;
        particle.position = {970.0F + jitter(random), 545.0F + jitter(random)};
        particle.velocity = {std::cos(direction) * velocity, std::sin(direction) * velocity};
        particle.acceleration = {0, 65};
        particle.lifetime = 0.45F + static_cast<float>(i % 5) * 0.06F;
        particle.startSize = 28;
        particle.endSize = 8;
        particle.startColor = {1.0F, 0.62F, 0.18F, 0.92F};
        particle.endColor = {0.28F, 0.08F, 0.01F, 0};
        particle.drag = 1.2F;
        particle.textureKey = 0;
        (void)particles.emit(particle);
    }
    for (int i = 0; i < 8; ++i) {
        mf::Particle2D particle;
        particle.position = {970.0F + jitter(random), 542.0F + jitter(random)};
        particle.velocity = {jitter(random) * 0.8F, -28.0F - static_cast<float>(i) * 4.0F};
        particle.lifetime = 1.35F + static_cast<float>(i) * 0.08F;
        particle.startSize = 64;
        particle.endSize = 128;
        particle.startColor = {0.55F, 0.56F, 0.58F, 0.58F};
        particle.endColor = {0.18F, 0.19F, 0.20F, 0};
        particle.drag = 0.35F;
        particle.textureKey = 1;
        (void)particles.emit(particle);
    }
}

void seedStressParticles(mf::ParticleSystem2D& particles, std::size_t count) {
    for (std::size_t index = 0; index < count; ++index) {
        mf::Particle2D particle;
        particle.position = {45.0F + static_cast<float>(index % 40) * 29.0F,
                             55.0F + static_cast<float>((index / 40) % 26) * 29.0F};
        particle.lifetime = 3'600.0F;
        particle.startSize = 18.0F + static_cast<float>(index % 4) * 3.0F;
        particle.endSize = particle.startSize;
        particle.startColor = {1.0F, 0.52F, 0.14F, 0.78F};
        particle.endColor = particle.startColor;
        particle.textureKey = static_cast<std::uint32_t>(index % 2);
        (void)particles.emit(particle);
    }
}

void submitStressSprites(mf::Renderer2D& renderer, const aegis::mediaforge::E2AssetCatalog& assets,
                         std::size_t count, bool enemies) {
    constexpr std::size_t columns = 40;
    for (std::size_t index = 0; index < count; ++index) {
        const float x = 28.0F + static_cast<float>(index % columns) * 30.0F;
        const float y = 32.0F + static_cast<float>((index / columns) % 28) * 29.0F;
        if (enemies) {
            submitAsset(renderer, assets, index % 9 == 0 ? "enemy.heavy_tank" : "enemy.raider", {x, y},
                        index % 9 == 0 ? mf::Vec2{38, 38} : mf::Vec2{30, 30}, 0, 1,
                        mf::BlendMode::alpha, 62, 0);
        } else {
            submitAsset(renderer, assets, index % 2 == 0 ? "decal.crack" : "decal.scorch", {x, y}, {28, 20},
                        static_cast<float>(index % 7) * 0.12F, 0.38F, mf::BlendMode::alpha, 16,
                        0);
        }
    }
}

bool begin(mf::Renderer2D& renderer, mf::RenderTarget& target, const mf::Camera2D& camera,
           mf::Color clear, bool shouldClear, std::string_view label) {
    auto result = renderer.beginPass({&target, camera, clear, shouldClear, label});
    if (!result) std::cerr << result.error().message << '\n';
    return result.has_value();
}

bool end(mf::Renderer2D& renderer) {
    auto result = renderer.endPass();
    if (!result) std::cerr << result.error().message << '\n';
    return result.has_value();
}

} // namespace

int aegis::mediaforge::runVerdantShowcase(const E2RunConfiguration& run) {
    const auto quality = aegis::mediaforge::qualityConfiguration(run.quality);
    auto windowResult = mf::Window::create({"AEGIS DOMINION — MediaForge E2 Verdant Slice", 1600, 900, true, true});
    if (!windowResult) return report(windowResult.error());
    mf::Window window = std::move(*windowResult);
    mf::EngineConfig config{"AEGIS DOMINION E2", mf::GpuBackendPreference::preferVulkan,
#ifndef NDEBUG
        true
#else
        false
#endif
    };
    auto deviceResult = mf::GPUDevice::create(window, config);
    if (!deviceResult) return report(deviceResult.error());
    mf::GPUDevice device = std::move(*deviceResult);
    mf::PresentMode presentMode = run.verticalSync ? mf::PresentMode::vsync : mf::PresentMode::immediate;
    if (!device.supportsPresentMode(window, presentMode)) {
        presentMode = device.supportsPresentMode(window, mf::PresentMode::mailbox)
            ? mf::PresentMode::mailbox : mf::PresentMode::vsync;
        mf::log(mf::LogLevel::warning, "Requested present mode unavailable; using " +
            std::string(aegis::mediaforge::presentModeName(presentMode)));
    }
    if (auto configured = device.setPresentMode(window, presentMode); !configured) return report(configured.error());

    const std::filesystem::path shaderDirectory = MEDIAFORGE_SHADER_DIRECTORY;
    const std::size_t requestedVertices = (run.stressEnemies + run.stressEnvironment + run.stressParticles + 512U) * 6U;
    const std::size_t vertexCapacity = std::max<std::size_t>(32'768, requestedVertices);
    auto rendererResult = mf::Renderer2D::create(device, window, shaderDirectory, vertexCapacity);
    if (!rendererResult) return report(rendererResult.error());
    mf::Renderer2D renderer = std::move(*rendererResult);

    std::filesystem::path assetsRoot = "assets";
    if (!std::filesystem::exists(assetsRoot / "e2/manifest.mfassets")) assetsRoot = std::filesystem::path(AEGIS_SOURCE_DIR) / "assets";
    auto assetsResult = aegis::mediaforge::E2AssetCatalog::load(device, assetsRoot, assetsRoot / "e2/manifest.mfassets");
    if (!assetsResult) return report(assetsResult.error());
    auto assets = std::move(*assetsResult);
    mf::log(mf::LogLevel::info, std::format(
        "E2 config: quality={} present={} fps_limit={} profile={} vertex_capacity={} | assets={} textures={} texture_bytes={} load_ms={:.2f}",
        aegis::mediaforge::qualityName(run.quality), aegis::mediaforge::presentModeName(presentMode), run.fpsLimit,
        run.uncappedProfiling, vertexCapacity, assets.assetCount(), assets.textureCount(), assets.approximateTextureBytes(),
        assets.loadMilliseconds()));

    const auto worldFormat = quality.hdrWorld ? mf::RenderTargetDescription::Format::rgba16Float
                                               : mf::RenderTargetDescription::Format::rgba8Unorm;
    const auto emissiveFormat = quality.hdrEmissive ? mf::RenderTargetDescription::Format::rgba16Float
                                                     : mf::RenderTargetDescription::Format::rgba8Unorm;
    const auto emissiveWidth = static_cast<std::uint32_t>(referenceWidth * quality.bloomScale);
    const auto emissiveHeight = static_cast<std::uint32_t>(referenceHeight * quality.bloomScale);
    auto worldResult = renderer.createRenderTarget({1600, 900, "E2WorldLinear", worldFormat});
    auto emissiveResult = renderer.createRenderTarget({emissiveWidth, emissiveHeight, "E2EmissiveLinear", emissiveFormat});
    auto overlayResult = renderer.createRenderTarget({1600, 900, "E2Overlay", mf::RenderTargetDescription::Format::rgba8Unorm});
    if (!worldResult) return report(worldResult.error());
    if (!emissiveResult) return report(emissiveResult.error());
    if (!overlayResult) return report(overlayResult.error());
    mf::RenderTarget world = std::move(*worldResult);
    mf::RenderTarget emissive = std::move(*emissiveResult);
    mf::RenderTarget overlay = std::move(*overlayResult);

    mf::EventPump events;
    std::vector<mf::Event> frameEvents;
    frameEvents.reserve(32);
    mf::InputState input;
    mf::ParticleSystem2D particles(std::max<std::size_t>(2048, run.stressParticles + 64));
    seedStressParticles(particles, run.stressParticles);
    std::mt19937 random(0xE2A615U);
    const auto details = aegis::mediaforge::deterministicVerdantDetails(0xE2A615U);
    auto previous = std::chrono::steady_clock::now();
    float elapsed{};
    float logAccumulator{};
    int lastImpactCycle = -1;
    aegis::mediaforge::FramePacer framePacer(run.uncappedProfiling ? 0U : run.fpsLimit);
    aegis::mediaforge::BenchmarkAccumulator benchmark;
    const auto runStart = std::chrono::steady_clock::now();
    bool focused = true;
    bool minimized = false;
    bool overlayDirty = true;

    while (!window.closeRequested()) {
        framePacer.beginFrame();
        const auto wallFrameStart = std::chrono::steady_clock::now();
        input.beginFrame();
        events.poll(frameEvents);
        for (const auto& event : frameEvents) {
            input.consume(event);
            if (std::holds_alternative<mf::QuitEvent>(event)) window.requestClose();
            if (const auto* activity = std::get_if<mf::WindowActivityEvent>(&event)) {
                focused = activity->focused;
                minimized = activity->minimized;
            }
        }
        if (input.keyPressed(mf::Key::escape)) window.requestClose();
        if (window.closeRequested()) break;
        if (minimized) {
            previous = std::chrono::steady_clock::now();
            (void)framePacer.wait(true);
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        const float delta = std::min(std::chrono::duration<float>(now - previous).count(), 0.05F);
        previous = now;
        elapsed += delta;
        logAccumulator += delta;
        const auto particleStart = std::chrono::steady_clock::now();
        particles.update(delta);
        const float particleMilliseconds = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - particleStart).count();
        const int impactCycle = static_cast<int>(elapsed / 2.8F);
        if (run.stressParticles == 0 && impactCycle != lastImpactCycle) {
            lastImpactCycle = impactCycle;
            spawnImpactParticles(particles, random);
        }

        const auto updateStart = std::chrono::steady_clock::now();
        const auto snapshot = aegis::mediaforge::makeVerdantSliceSnapshot(elapsed);
        const auto visuals = aegis::mediaforge::translateVerdantSlice(snapshot);
        const auto camera = referenceCamera();
        const auto emissiveCamera = referenceCamera(static_cast<float>(emissiveWidth), static_cast<float>(emissiveHeight));
        const float updateMilliseconds = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - updateStart).count();
        renderer.beginFrame();
        const auto spriteStart = std::chrono::steady_clock::now();

        if (!begin(renderer, world, camera, {0.01F, 0.018F, 0.012F, 1}, true, "World base")) return 2;
        submitAsset(renderer, assets, "terrain.verdant.composed", {800, 450}, {1600, 900}, 0, 1,
                    mf::BlendMode::opaque, 0);
        for (const auto& detail : details) {
            submitAsset(renderer, assets, detail.logicalId, {detail.position.x, detail.position.y},
                        {detail.size.x, detail.size.y}, detail.rotationRadians, detail.opacity,
                        detail.blend, detail.layer, 0);
        }
        submitStressSprites(renderer, assets, run.stressEnvironment, false);
        submitAsset(renderer, assets, "vfx.soft_shadow", {195, 765}, {330, 150}, 0, 0.58F, mf::BlendMode::alpha, 20);
        submitAsset(renderer, assets, "environment.verdant.grove", {190, 740}, {310, 310}, -0.05F, 1, mf::BlendMode::alpha, 22);
        submitAsset(renderer, assets, "vfx.soft_shadow", {1090, 195}, {300, 135}, 0, 0.62F, mf::BlendMode::alpha, 20);
        submitAsset(renderer, assets, "environment.checkpoint", {1090, 175}, {300, 300}, 0.02F, 1, mf::BlendMode::alpha, 22);
        submitAsset(renderer, assets, "environment.verdant.grove", {690, 88}, {205, 205}, 0.18F, 0.95F, mf::BlendMode::alpha, 22);
        submitAsset(renderer, assets, "vfx.soft_shadow", {96, 340}, {260, 120}, 0, 0.72F, mf::BlendMode::alpha, 50);
        submitAsset(renderer, assets, "vfx.soft_shadow", {1178, 603}, {250, 118}, 0, 0.76F, mf::BlendMode::alpha, 50);
        for (const auto& visual : visuals) {
            if (visual.kind == aegis::mediaforge::SliceVisualKind::emissive ||
                visual.kind == aegis::mediaforge::SliceVisualKind::projectile) continue;
            submitAsset(renderer, assets, visual.logicalId, {visual.position.x, visual.position.y},
                        {visual.size.x, visual.size.y}, visual.rotationRadians, visual.opacity,
                        visual.blend, visual.layer, 0);
        }
        submitStressSprites(renderer, assets, run.stressEnemies, true);
        const float impactPhase = std::fmod(elapsed, 2.8F);
        if (impactPhase < 0.7F) {
            const float normalized = impactPhase / 0.7F;
            submitAsset(renderer, assets, "vfx.smoke", {970, 535}, {145 + normalized * 65, 105 + normalized * 55},
                        normalized * 0.2F, 0.75F * (1 - normalized), mf::BlendMode::alpha, 85);
            submitAsset(renderer, assets, "vfx.debris", {970, 545}, {130 + normalized * 80, 88 + normalized * 55},
                        normalized * 0.5F, 0.65F * (1 - normalized), mf::BlendMode::alpha, 84);
        }
        for (const auto& particle : particles.particles()) {
            const auto color = mf::particleColor(particle);
            const float size = mf::particleSize(particle);
            submitAsset(renderer, assets, particle.textureKey == 0 ? "vfx.sparks" : "vfx.smoke",
                        particle.position, {size, size * 0.68F}, particle.rotationRadians, color.w,
                        particle.textureKey == 0 ? mf::BlendMode::additive : mf::BlendMode::alpha, 88,
                        static_cast<std::int32_t>(particle.textureKey), {color.x, color.y, color.z, 1});
        }
        renderer.addSubmittedParticles(particles.particles().size());
        if (!end(renderer)) return 2;

        if (!begin(renderer, emissive, emissiveCamera, {0, 0, 0, 0}, true, "Emissive effects")) return 2;
        const float energyPulse = 0.86F + std::sin(elapsed * 3.2F) * 0.10F;
        submitAsset(renderer, assets, "vfx.amber_halo", {92, 323}, {250, 175}, elapsed * 0.04F,
                    0.72F * energyPulse, mf::BlendMode::additive, 100);
        submitAsset(renderer, assets, "vfx.cyan_halo", {1168, 585}, {280, 190}, -elapsed * 0.05F,
                    0.82F * energyPulse, mf::BlendMode::additive, 100);
        submitAsset(renderer, assets, "vfx.cyan_halo", {520, 330}, {150, 104}, 0, 0.46F * energyPulse, mf::BlendMode::additive, 100);
        submitAsset(renderer, assets, "vfx.cyan_halo", {845, 710}, {190, 128}, 0, 0.58F * energyPulse, mf::BlendMode::additive, 100);
        const float pulseFirePhase = std::fmod(elapsed, 0.72F);
        if (pulseFirePhase < 0.13F) {
            submitAsset(renderer, assets, "vfx.pulse_muzzle", {560, 294}, {102, 66}, -0.35F,
                        1.0F - pulseFirePhase / 0.13F, mf::BlendMode::additive, 112);
        }
        const float railCharge = std::clamp(1.0F - std::abs(impactPhase - 2.2F) / 0.55F, 0.0F, 1.0F);
        submitAsset(renderer, assets, "vfx.energy_ring", {845, 710}, {150 + railCharge * 75, 100 + railCharge * 48},
                    elapsed * 0.2F, railCharge * 0.55F, mf::BlendMode::additive, 106);
        for (const auto& visual : visuals) {
            if (visual.kind != aegis::mediaforge::SliceVisualKind::projectile) continue;
            submitAsset(renderer, assets, visual.logicalId, {visual.position.x, visual.position.y},
                        {visual.size.x, visual.size.y}, visual.rotationRadians, visual.opacity,
                        mf::BlendMode::additive, 105);
        }
        if (impactPhase < 0.55F) {
            const float normalized = impactPhase / 0.55F;
            submitAsset(renderer, assets, "vfx.explosion", {970, 540}, {170 + normalized * 80, 116 + normalized * 52},
                        normalized * 0.4F, 1 - normalized, mf::BlendMode::additive, 110);
            submitAsset(renderer, assets, "vfx.shock_ring", {970, 545}, {120 + normalized * 250, 80 + normalized * 165},
                        0, 0.78F * (1 - normalized), mf::BlendMode::additive, 109);
            submitAsset(renderer, assets, "vfx.rail_impact", {970, 540}, {145, 98}, 0,
                        std::max(0.0F, 1 - normalized * 2.2F), mf::BlendMode::additive, 111);
        }
        submitAsset(renderer, assets, "vfx.cyan_halo", {970, 540}, {impactPhase < 0.4F ? 340.0F : 80.0F,
                    impactPhase < 0.4F ? 230.0F : 54.0F}, 0, impactPhase < 0.4F ? 0.75F : 0.08F,
                    mf::BlendMode::additive, 120);
        submitAsset(renderer, assets, "vfx.rail_tracer", {910, 610}, {390, 42}, -0.22F,
                    std::max(0.0F, 1.0F - impactPhase * 5.0F), mf::BlendMode::additive, 121);
        if (!end(renderer)) return 2;

        const bool drawOverlay = overlayDirty;
        if (drawOverlay) {
            if (!begin(renderer, overlay, camera, {0, 0, 0, 0}, true, "UI")) return 2;
            submitNineSlice(renderer, assets, "ui.surface.command", {1250, 10}, {340, 880}, 56, 200, 0.97F);
            submitNineSlice(renderer, assets, "ui.surface.status", {20, 66}, {760, 90}, 26, 201, 0.94F);
            submitNineSlice(renderer, assets, "ui.surface.status", {1270, 120}, {300, 315}, 42, 201, 0.96F);
            submitNineSlice(renderer, assets, "ui.surface.selected", {1270, 455}, {300, 300}, 44, 202, 0.98F);
            submitNineSlice(renderer, assets, "ui.surface.action", {1280, 760}, {280, 64}, 22, 203, 1.0F);
            submitSolid(renderer, {121, 132}, {158, 10}, {0.12F, 0.72F, 0.84F, 0.85F}, 205);
            submitSolid(renderer, {329, 132}, {174, 10}, {0.22F, 0.83F, 0.50F, 0.90F}, 205);
            submitSolid(renderer, {530, 132}, {92, 10}, {0.95F, 0.67F, 0.20F, 0.92F}, 205);
            submitAsset(renderer, assets, "enemy.raider", {1332, 260}, {105, 105}, 0, 1, mf::BlendMode::alpha, 206);
            submitAsset(renderer, assets, "enemy.heavy_tank", {1485, 260}, {125, 125}, 0, 1, mf::BlendMode::alpha, 206);
            submitAsset(renderer, assets, "tower.pulse.complete", {1335, 645}, {120, 80}, 0, 1, mf::BlendMode::alpha, 206);
            submitAsset(renderer, assets, "tower.railgun.complete", {1490, 645}, {128, 86}, 0, 1, mf::BlendMode::alpha, 206);
            submitAsset(renderer, assets, "ui.labels.e2", {800, 450}, {1600, 900}, 0, 1, mf::BlendMode::alpha, 210);
            if (!end(renderer)) return 2;
            overlayDirty = false;
        }
        if (!drawOverlay) renderer.addCachedSprites(53);

        renderer.setComposite({&world, &emissive, &overlay,
            {0.48F, 1.45F, 0.20F, 1.05F, 1.04F, {0.98F, 1.015F, 0.99F, 1}}});
        const float spriteMilliseconds = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - spriteStart).count();
        renderer.setApplicationTimings(updateMilliseconds, spriteMilliseconds, particleMilliseconds);
        auto presented = renderer.present();
        if (!presented) return report(presented.error());
        const float limiterWait = framePacer.wait(!focused);
        renderer.setFrameLimitWait(limiterWait);
        const float wallFrameMilliseconds = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - wallFrameStart).count();

        const float runtimeSeconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - runStart).count();
        if (run.benchmarkSeconds > 0.0F && runtimeSeconds >= run.warmupSeconds) {
            benchmark.add(renderer.statistics(), wallFrameMilliseconds);
        }

        if (logAccumulator >= 1.0F) {
            const auto& stats = renderer.statistics();
            const float fps = wallFrameMilliseconds > 0.001F ? 1000.0F / wallFrameMilliseconds : 0.0F;
            const std::string line = std::format(
                "E2 {:.0f} FPS | {:.2f} ms CPU + {:.2f} ms present + {:.2f} ms cap | {} sprites ({} culled) | "
                "{} particles | {} batches | {} draws | {} triangles",
                fps, stats.cpuFrameMilliseconds, stats.presentWaitMilliseconds, stats.frameLimitWaitMilliseconds,
                stats.submittedSprites + stats.cachedSprites, stats.culledSprites, stats.submittedParticles, stats.batches,
                stats.drawCalls, stats.triangles);
            mf::log(mf::LogLevel::info, line);
            if (const auto titleResult = window.setTitle("AEGIS DOMINION — MediaForge E2 — " + line); !titleResult) {
                mf::log(mf::LogLevel::warning, titleResult.error().message);
            }
            logAccumulator = 0.0F;
        }
        if (run.benchmarkSeconds > 0.0F && runtimeSeconds >= run.warmupSeconds + run.benchmarkSeconds) {
            const std::string scene = std::format("e2_reference_e{}_d{}_p{}", run.stressEnemies,
                                                   run.stressEnvironment, run.stressParticles);
            std::cout << benchmark.report(scene, aegis::mediaforge::processResidentBytes(), assets.textureCount(),
                                          assets.approximateTextureBytes(), assets.loadMilliseconds()) << '\n';
            window.requestClose();
        }
    }
    return 0;
}
