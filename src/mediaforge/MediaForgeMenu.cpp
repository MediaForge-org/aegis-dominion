#include "MediaForgeMenu.hpp"

#include "E2AssetCatalog.hpp"
#include "MediaForgeAppModel.hpp"

#include <mediaforge/foundation/Config.hpp>
#include <mediaforge/foundation/Log.hpp>
#include <mediaforge/input/Event.hpp>
#include <mediaforge/input/InputState.hpp>
#include <mediaforge/platform/Window.hpp>
#include <mediaforge/render/Renderer2D.hpp>
#include <mediaforge/ui/SegmentedLevelIndicator.hpp>
#include <mediaforge/ui/Ui.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace aegis::mediaforge {
namespace {

constexpr float canvasWidth = 1600.0F;
constexpr float canvasHeight = 900.0F;
constexpr auto settingChangeAnimationDuration = std::chrono::milliseconds{125};

struct ButtonDefinition {
    mf::ui::WidgetId id{};
    mf::ui::Rect bounds{};
    AppCommand command{};
    std::string_view label;
    std::optional<SettingId> setting;
    bool enabled{true};
};

constexpr std::array mainButtons{
    ButtonDefinition{1, {{570, 310}, {460, 68}}, AppCommand::play, "SPIELEN", std::nullopt},
    ButtonDefinition{2, {{570, 392}, {460, 68}}, AppCommand::mapForge, "MAP FORGE", std::nullopt},
    ButtonDefinition{3, {{570, 474}, {460, 68}}, AppCommand::tutorial, "ANLEITUNG", std::nullopt},
    ButtonDefinition{4, {{570, 556}, {460, 68}}, AppCommand::settings, "EINSTELLUNGEN", std::nullopt},
    ButtonDefinition{5, {{570, 638}, {460, 68}}, AppCommand::quit, "BEENDEN", std::nullopt},
};
constexpr std::array backButton{
    ButtonDefinition{10, {{620, 720}, {360, 66}}, AppCommand::back, "ZURÜCK", std::nullopt},
};

std::vector<ButtonDefinition> buttonsFor(ScreenId screen, const MediaForgeAppModel& application) {
    switch (screen) {
        case ScreenId::mainMenu: return {mainButtons.begin(), mainButtons.end()};
        case ScreenId::settings: {
            const auto settings = application.settingDescriptors();
            std::vector<ButtonDefinition> buttons;
            buttons.reserve(settings.size() + 1);
            for (std::size_t index = 0; index < settings.size(); ++index) {
                buttons.push_back({21U + index, {{750, 300.0F + static_cast<float>(index) * 92.0F}, {400, 66}},
                                   AppCommand::back, "", settings[index].id, !settings[index].disabled});
            }
            buttons.push_back({90, {{620, 720}, {360, 66}}, AppCommand::back, "ZURÜCK", std::nullopt, true});
            return buttons;
        }
        case ScreenId::play:
        case ScreenId::mapForge:
        case ScreenId::tutorial: return {backButton.begin(), backButton.end()};
    }
    return {};
}

mf::Camera2D menuCamera() {
    mf::Camera2D camera;
    camera.setCenter({canvasWidth * 0.5F, canvasHeight * 0.5F});
    camera.setOrthographicSize({canvasWidth, canvasHeight});
    camera.setViewport({0, 0, canvasWidth, canvasHeight});
    return camera;
}

int report(const mf::Error& error) {
    std::cerr << "AEGIS MediaForge menu failed: " << error.message << '\n';
    return 1;
}

bool begin(mf::Renderer2D& renderer, mf::RenderTarget& target, mf::Color clear,
           bool shouldClear, std::string_view label) {
    auto result = renderer.beginPass({&target, menuCamera(), clear, shouldClear, label});
    if (!result) std::cerr << result.error().message << '\n';
    return result.has_value();
}

bool end(mf::Renderer2D& renderer) {
    auto result = renderer.endPass();
    if (!result) std::cerr << result.error().message << '\n';
    return result.has_value();
}

void submitAsset(mf::Renderer2D& renderer, const E2AssetCatalog& assets, std::string_view id,
                 mf::Vec2 position, mf::Vec2 size, float opacity, mf::BlendMode blend,
                 std::int32_t layer, mf::Color color = {}) {
    const auto* asset = assets.find(id);
    if (!asset) return;
    mf::Sprite2D sprite;
    sprite.texture = asset->texture;
    sprite.sampler = asset->sampler;
    sprite.position = position;
    sprite.size = size;
    sprite.pivot = asset->pivot;
    sprite.uv = asset->uv;
    sprite.color = color;
    sprite.opacity = opacity;
    sprite.blend = blend;
    sprite.layer = layer;
    sprite.space = mf::CoordinateSpace::screen;
    renderer.submit(sprite);
}

void submitSolid(mf::Renderer2D& renderer, mf::ui::Rect bounds, mf::Color color, std::int32_t layer) {
    mf::Sprite2D sprite;
    sprite.position = {bounds.position.x + bounds.size.x * 0.5F, bounds.position.y + bounds.size.y * 0.5F};
    sprite.size = bounds.size;
    sprite.color = color;
    sprite.blend = mf::BlendMode::alpha;
    sprite.layer = layer;
    sprite.space = mf::CoordinateSpace::screen;
    renderer.submit(sprite);
}

void submitNineSlice(mf::Renderer2D& renderer, const E2AssetCatalog& assets, std::string_view id,
                     mf::Vec2 position, mf::Vec2 size, float border, std::int32_t layer,
                     float opacity = 1.0F) {
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
            sprite.space = mf::CoordinateSpace::screen;
            renderer.submit(sprite);
        }
    }
}

std::pair<char32_t, std::size_t> decodeUtf8(std::string_view text, std::size_t offset) noexcept {
    const auto lead = static_cast<unsigned char>(text[offset]);
    if (lead < 0x80) return {lead, 1};
    if ((lead & 0xE0) == 0xC0 && offset + 1 < text.size()) {
        return {static_cast<char32_t>(((lead & 0x1F) << 6) |
                (static_cast<unsigned char>(text[offset + 1]) & 0x3F)), 2};
    }
    if ((lead & 0xF0) == 0xE0 && offset + 2 < text.size()) {
        return {static_cast<char32_t>(((lead & 0x0F) << 12) |
                ((static_cast<unsigned char>(text[offset + 1]) & 0x3F) << 6) |
                (static_cast<unsigned char>(text[offset + 2]) & 0x3F)), 3};
    }
    return {U'?', 1};
}

std::size_t glyphIndex(char32_t codepoint) noexcept {
    if (codepoint >= U' ' && codepoint <= U'~') return static_cast<std::size_t>(codepoint - U' ');
    switch (codepoint) {
        case U'ß': return 95;
        case U'Ä': return 96;
        case U'Ö': return 97;
        case U'Ü': return 98;
        case U'ä': return 99;
        case U'ö': return 100;
        case U'ü': return 101;
        case U'—': return 102;
        default: return static_cast<std::size_t>(U'?' - U' ');
    }
}

std::size_t glyphCount(std::string_view text) noexcept {
    std::size_t count{};
    for (std::size_t offset = 0; offset < text.size(); ++count) offset += decodeUtf8(text, offset).second;
    return count;
}

enum class TextAlign { left, center };

void submitText(mf::Renderer2D& renderer, const E2AssetCatalog& assets, std::string_view text,
                mf::Vec2 position, float height, mf::Color color, std::int32_t layer,
                TextAlign alignment = TextAlign::left) {
    const auto* font = assets.find(E2AssetId::interfaceFont);
    if (!font) return;
    constexpr float columns = 16.0F;
    constexpr float rows = 7.0F;
    const float advance = height * 0.70F;
    float x = position.x;
    if (alignment == TextAlign::center) x -= static_cast<float>(glyphCount(text)) * advance * 0.5F;
    for (std::size_t offset = 0; offset < text.size();) {
        const auto [codepoint, length] = decodeUtf8(text, offset);
        offset += length;
        const std::size_t index = glyphIndex(codepoint);
        const float column = static_cast<float>(index % 16);
        const float row = static_cast<float>(index / 16);
        mf::Sprite2D sprite;
        sprite.texture = font->texture;
        sprite.sampler = font->sampler;
        sprite.position = {x + advance * 0.5F, position.y + height * 0.5F};
        sprite.size = {height * (64.0F / 72.0F), height};
        sprite.uv = {column / columns, row / rows, (column + 1.0F) / columns, (row + 1.0F) / rows};
        sprite.color = color;
        sprite.layer = layer;
        sprite.space = mf::CoordinateSpace::screen;
        renderer.submit(sprite);
        x += advance;
    }
}

std::string buttonLabel(const ButtonDefinition& definition, const MediaForgeAppModel& application) {
    if (definition.setting) {
        if (const auto setting = application.settingDescriptor(*definition.setting)) {
            return std::string(setting->displayValue());
        }
    }
    return std::string(definition.label);
}

mf::Color textColor(mf::ui::WidgetState state) noexcept {
    switch (state) {
        case mf::ui::WidgetState::hovered: return {0.72F, 0.98F, 1.0F, 1.0F};
        case mf::ui::WidgetState::pressed: return {1.0F, 0.78F, 0.35F, 1.0F};
        case mf::ui::WidgetState::disabled: return {0.38F, 0.43F, 0.46F, 0.72F};
        case mf::ui::WidgetState::focused: return {0.58F, 0.91F, 0.97F, 1.0F};
        case mf::ui::WidgetState::normal: return {0.84F, 0.91F, 0.93F, 1.0F};
    }
    return {};
}

void drawButton(mf::Renderer2D& renderer, const E2AssetCatalog& assets,
                const ButtonDefinition& definition, mf::ui::WidgetState state,
                std::string_view label) {
    auto bounds = definition.bounds;
    if (state == mf::ui::WidgetState::pressed) bounds.position.y += 2.0F;
    const bool highlighted = state == mf::ui::WidgetState::hovered || state == mf::ui::WidgetState::focused;
    if (highlighted) {
        const mf::Vec2 center{bounds.position.x + bounds.size.x * 0.5F, bounds.position.y + bounds.size.y * 0.5F};
        submitAsset(renderer, assets, "vfx.cyan_halo", center, {bounds.size.x + 100, bounds.size.y + 54},
                    0.18F, mf::BlendMode::additive, 231);
    }
    const float opacity = state == mf::ui::WidgetState::disabled ? 0.40F :
                          state == mf::ui::WidgetState::pressed ? 1.0F : 0.92F;
    submitNineSlice(renderer, assets, "ui.surface.action", bounds.position, bounds.size, 22, 232, opacity);
    const mf::Color accent = state == mf::ui::WidgetState::pressed
        ? mf::Color{0.95F, 0.57F, 0.16F, 0.92F}
        : mf::Color{0.08F, 0.67F, 0.76F, highlighted ? 0.92F : 0.48F};
    submitSolid(renderer, {{bounds.position.x + 12, bounds.position.y + 12}, {4, bounds.size.y - 24}}, accent, 234);
    submitText(renderer, assets, label,
               {bounds.position.x + bounds.size.x * 0.5F, bounds.position.y + 17}, 34,
               textColor(state), 235, TextAlign::center);
}

void drawSettingIndicator(mf::Renderer2D& renderer, mf::ui::Rect buttonBounds,
                          mf::ui::WidgetState state, const SettingDescriptor& setting,
                          float changePulse) {
    if (!setting.usesSegmentedIndicator()) return;
    constexpr float indicatorWidth = 232.0F;
    const float startX = buttonBounds.position.x + (buttonBounds.size.x - indicatorWidth) * 0.5F;
    const float y = buttonBounds.position.y + buttonBounds.size.y - 12.0F;
    mf::ui::submitSegmentedLevelIndicator(
        renderer, {{startX, y}, {indicatorWidth, 7.0F}},
        {setting.stepCount(), setting.currentIndex}, state, changePulse, {}, 235);
}

void drawScreenHeading(mf::Renderer2D& renderer, const E2AssetCatalog& assets, std::string_view heading,
                       std::string_view eyebrow) {
    submitText(renderer, assets, eyebrow, {800, 166}, 22, {0.32F, 0.76F, 0.82F, 0.95F}, 225, TextAlign::center);
    submitText(renderer, assets, heading, {800, 200}, 52, {0.90F, 0.96F, 0.97F, 1.0F}, 226, TextAlign::center);
    submitSolid(renderer, {{590, 270}, {420, 2}}, {0.10F, 0.65F, 0.72F, 0.58F}, 226);
    submitSolid(renderer, {{735, 277}, {130, 3}}, {0.92F, 0.58F, 0.18F, 0.72F}, 226);
}

void drawOverlay(mf::Renderer2D& renderer, const E2AssetCatalog& assets,
                 const MediaForgeAppModel& application,
                 std::span<const ButtonDefinition> definitions,
                 std::span<const mf::ui::WidgetState> buttonStates,
                 const std::optional<SettingId>& pulsedSetting, float settingChangePulse) {
    const auto screen = application.screen();
    const bool main = screen == ScreenId::mainMenu;
    submitNineSlice(renderer, assets, "ui.surface.command", main ? mf::Vec2{500, 72} : mf::Vec2{235, 105},
                    main ? mf::Vec2{600, 756} : mf::Vec2{1130, 710}, main ? 54.0F : 58.0F, 220, 0.97F);
    submitSolid(renderer, {{main ? 522.0F : 257.0F, main ? 94.0F : 127.0F},
                           {main ? 556.0F : 1086.0F, 3}}, {0.08F, 0.67F, 0.75F, 0.62F}, 222);

    if (main) {
        submitText(renderer, assets, "AEGIS", {800, 122}, 76, {0.76F, 0.94F, 0.97F, 1.0F}, 225, TextAlign::center);
        submitText(renderer, assets, "DOMINION", {800, 200}, 42, {0.94F, 0.69F, 0.28F, 1.0F}, 225, TextAlign::center);
        submitText(renderer, assets, "TACTICAL DEFENSE NETWORK", {800, 254}, 18,
                   {0.40F, 0.66F, 0.70F, 0.92F}, 225, TextAlign::center);
        submitText(renderer, assets, "MEDIAFORGE // VULKAN", {800, 770}, 17,
                   {0.38F, 0.52F, 0.55F, 0.86F}, 225, TextAlign::center);
    } else if (screen == ScreenId::play) {
        drawScreenHeading(renderer, assets, "KARTENAUSWAHL", "E2.1 // SPIELEN");
        submitText(renderer, assets, "Interaktive Kartenauswahl folgt in E2.2.", {800, 365}, 30,
                   {0.80F, 0.89F, 0.91F, 1}, 225, TextAlign::center);
        submitText(renderer, assets, "Kein automatischer Start der E2-Referenzszene.", {800, 420}, 25,
                   {0.50F, 0.68F, 0.71F, 1}, 225, TextAlign::center);
        submitNineSlice(renderer, assets, "ui.surface.selected", {550, 500}, {500, 125}, 34, 224, 0.78F);
        submitText(renderer, assets, "E2.2  MAP FLOW", {800, 532}, 27,
                   {0.91F, 0.62F, 0.23F, 1}, 226, TextAlign::center);
        submitText(renderer, assets, "MIGRATION AUSSTEHEND", {800, 573}, 20,
                   {0.49F, 0.70F, 0.73F, 1}, 226, TextAlign::center);
    } else if (screen == ScreenId::mapForge) {
        drawScreenHeading(renderer, assets, "MAP FORGE", "E2.1 // EDITOR");
        submitText(renderer, assets, "MAP FORGE Migration ausstehend.", {800, 375}, 32,
                   {0.84F, 0.91F, 0.92F, 1}, 225, TextAlign::center);
        submitText(renderer, assets, "Der bestehende SFML-Editor bleibt vollständig erhalten.", {800, 433}, 24,
                   {0.51F, 0.69F, 0.71F, 1}, 225, TextAlign::center);
        submitText(renderer, assets, "Terrain  //  Pfade  //  Wellen  //  Playtest", {800, 525}, 23,
                   {0.91F, 0.61F, 0.23F, 0.96F}, 225, TextAlign::center);
    } else if (screen == ScreenId::tutorial) {
        drawScreenHeading(renderer, assets, "ANLEITUNG", "TAKTISCHER ÜBERBLICK");
        submitText(renderer, assets, "ÜBERBLICK", {400, 330}, 26, {0.91F, 0.64F, 0.25F, 1}, 225);
        submitText(renderer, assets, "Verteidige den AEGIS-Kern gegen alle Wellen.", {400, 371}, 22,
                   {0.78F, 0.88F, 0.90F, 1}, 225);
        submitText(renderer, assets, "MAUS", {400, 437}, 26, {0.91F, 0.64F, 0.25F, 1}, 225);
        submitText(renderer, assets, "Wähle Menüs und platziere später Türme im Gelände.", {400, 478}, 22,
                   {0.78F, 0.88F, 0.90F, 1}, 225);
        submitText(renderer, assets, "SPIELABLAUF", {400, 544}, 26, {0.91F, 0.64F, 0.25F, 1}, 225);
        submitText(renderer, assets, "Baue, verbessere und kombiniere Verteidigungssysteme.", {400, 585}, 22,
                   {0.78F, 0.88F, 0.90F, 1}, 225);
        submitText(renderer, assets, "Escape oder ZURÜCK führt zum vorherigen Bildschirm.", {400, 642}, 20,
                   {0.47F, 0.67F, 0.70F, 1}, 225);
    } else if (screen == ScreenId::settings) {
        drawScreenHeading(renderer, assets, "EINSTELLUNGEN", "PERFORMANCE");
        const auto settings = application.settingDescriptors();
        for (std::size_t index = 0; index < settings.size(); ++index) {
            const auto category = settingCategoryMetadata(settings[index].category);
            const auto rowLabel = std::format("{} // {}", category.displayLabel, settings[index].displayLabel);
            submitText(renderer, assets, rowLabel, {420, 318.0F + static_cast<float>(index) * 92.0F}, 18,
                       {0.48F, 0.69F, 0.72F, 1}, 225);
        }
        submitText(renderer, assets, "Änderungen werden sofort auf MediaForge angewendet.", {800, 610}, 21,
                   {0.50F, 0.69F, 0.71F, 1}, 225, TextAlign::center);
    }

    for (std::size_t index = 0; index < definitions.size(); ++index) {
        const auto state = index < buttonStates.size() ? buttonStates[index] : mf::ui::WidgetState::normal;
        drawButton(renderer, assets, definitions[index], state, buttonLabel(definitions[index], application));
        if (definitions[index].setting) {
            if (const auto setting = application.settingDescriptor(*definitions[index].setting)) {
                const float pulse = pulsedSetting == definitions[index].setting ? settingChangePulse : 0.0F;
                drawSettingIndicator(renderer, definitions[index].bounds, state, *setting, pulse);
            }
        }
    }
}

void drawStaticWorld(mf::Renderer2D& renderer, const E2AssetCatalog& assets) {
    submitAsset(renderer, assets, "terrain.verdant.composed", {800, 450}, {1600, 900}, 0.38F,
                mf::BlendMode::opaque, 0, {0.20F, 0.28F, 0.28F, 1});
    submitAsset(renderer, assets, "environment.verdant.grove", {160, 745}, {430, 430}, 0.70F,
                mf::BlendMode::alpha, 10, {0.48F, 0.58F, 0.54F, 1});
    submitAsset(renderer, assets, "environment.checkpoint", {1400, 170}, {370, 370}, 0.62F,
                mf::BlendMode::alpha, 10, {0.50F, 0.58F, 0.60F, 1});
    submitAsset(renderer, assets, "world.spawn_gate", {125, 460}, {350, 235}, 0.56F,
                mf::BlendMode::alpha, 12, {0.52F, 0.58F, 0.60F, 1});
    submitAsset(renderer, assets, "world.aegis_core", {1475, 610}, {350, 235}, 0.62F,
                mf::BlendMode::alpha, 12, {0.50F, 0.62F, 0.64F, 1});
    submitSolid(renderer, {{0, 0}, {1600, 900}}, {0.01F, 0.025F, 0.032F, 0.50F}, 20);
}

void drawStaticEmissive(mf::Renderer2D& renderer, const E2AssetCatalog& assets) {
    submitAsset(renderer, assets, "vfx.cyan_halo", {800, 180}, {780, 350}, 0.20F,
                mf::BlendMode::additive, 1);
    submitAsset(renderer, assets, "vfx.amber_halo", {800, 690}, {600, 260}, 0.12F,
                mf::BlendMode::additive, 1);
    submitAsset(renderer, assets, "vfx.cyan_halo", {1460, 610}, {360, 260}, 0.16F,
                mf::BlendMode::additive, 1);
}

struct MenuTargets {
    mf::RenderTarget world;
    mf::RenderTarget emissive;
    mf::RenderTarget overlay;
};

mf::Result<MenuTargets> createTargets(mf::Renderer2D& renderer, GraphicsQuality quality) {
    const auto profile = qualityConfiguration(quality);
    const auto worldFormat = profile.hdrWorld ? mf::RenderTargetDescription::Format::rgba16Float
                                               : mf::RenderTargetDescription::Format::rgba8Unorm;
    const auto emissiveFormat = profile.hdrEmissive ? mf::RenderTargetDescription::Format::rgba16Float
                                                     : mf::RenderTargetDescription::Format::rgba8Unorm;
    auto world = renderer.createRenderTarget({1600, 900, "E2.1 Menu World", worldFormat});
    if (!world) return std::unexpected(world.error());
    auto emissive = renderer.createRenderTarget({static_cast<std::uint32_t>(canvasWidth * profile.bloomScale),
                                                  static_cast<std::uint32_t>(canvasHeight * profile.bloomScale),
                                                  "E2.1 Menu Emissive", emissiveFormat});
    if (!emissive) return std::unexpected(emissive.error());
    auto overlay = renderer.createRenderTarget({1600, 900, "E2.1 Menu UI",
                                                 mf::RenderTargetDescription::Format::rgba8Unorm});
    if (!overlay) return std::unexpected(overlay.error());
    return MenuTargets{std::move(*world), std::move(*emissive), std::move(*overlay)};
}

mf::Result<mf::PresentMode> applyPresentMode(mf::GPUDevice& device, mf::Window& window, bool verticalSync) {
    mf::PresentMode mode = verticalSync ? mf::PresentMode::vsync : mf::PresentMode::immediate;
    if (!device.supportsPresentMode(window, mode)) {
        mode = !verticalSync && device.supportsPresentMode(window, mf::PresentMode::mailbox)
            ? mf::PresentMode::mailbox : mf::PresentMode::vsync;
        mf::log(mf::LogLevel::warning, "Requested menu present mode unavailable; using " +
                                      std::string(presentModeName(mode)));
    }
    auto result = device.setPresentMode(window, mode);
    if (!result) return std::unexpected(result.error());
    return mode;
}

} // namespace

int runInteractiveMenu(const E2RunConfiguration& run) {
    auto windowResult = mf::Window::create({std::string(mediaForgeWindowTitle()), 1600, 900, true, true});
    if (!windowResult) return report(windowResult.error());
    mf::Window window = std::move(*windowResult);
    mf::EngineConfig config{"AEGIS DOMINION", mf::GpuBackendPreference::preferVulkan,
#ifndef NDEBUG
        true
#else
        false
#endif
    };
    auto deviceResult = mf::GPUDevice::create(window, config);
    if (!deviceResult) return report(deviceResult.error());
    mf::GPUDevice device = std::move(*deviceResult);

    RuntimeSettings initialSettings{run.verticalSync, run.fpsLimit == 0 ? 120U : run.fpsLimit, run.quality};
    MediaForgeAppModel application(initialSettings);
    auto presentMode = applyPresentMode(device, window, application.settings().verticalSync);
    if (!presentMode) return report(presentMode.error());

    auto rendererResult = mf::Renderer2D::create(device, window, MEDIAFORGE_SHADER_DIRECTORY, 32'768);
    if (!rendererResult) return report(rendererResult.error());
    mf::Renderer2D renderer = std::move(*rendererResult);

    std::filesystem::path assetsRoot = "assets";
    if (!std::filesystem::exists(assetsRoot / "e2/manifest.mfassets")) {
        assetsRoot = std::filesystem::path(AEGIS_SOURCE_DIR) / "assets";
    }
    auto assetsResult = E2AssetCatalog::load(device, assetsRoot, assetsRoot / "e2/manifest.mfassets");
    if (!assetsResult) return report(assetsResult.error());
    auto assets = std::move(*assetsResult);
    auto targetsResult = createTargets(renderer, application.settings().quality);
    if (!targetsResult) return report(targetsResult.error());
    auto targets = std::move(*targetsResult);

    mf::log(mf::LogLevel::info, std::format(
        "E2.1 interactive menu: backend={} present={} fps_limit={} quality={} assets={} textures={} load_ms={:.2f}",
        device.backendName(), presentModeName(*presentMode), application.settings().fpsLimit,
        qualityName(application.settings().quality), assets.assetCount(), assets.textureCount(), assets.loadMilliseconds()));

    mf::EventPump eventPump;
    std::vector<mf::Event> frameEvents;
    frameEvents.reserve(32);
    mf::InputState input;
    mf::ui::Canvas canvas({canvasWidth, canvasHeight});
    mf::ui::Context ui;
    aegis::mediaforge::FramePacer framePacer(run.uncappedProfiling ? 0U : application.settings().fpsLimit);
    bool focused = true;
    bool minimized = false;
    bool worldDirty = true;
    bool overlayDirty = true;
    std::vector<mf::ui::WidgetState> visualStates;
    ScreenId visualScreen = application.screen();
    auto definitions = buttonsFor(application.screen(), application);
    visualStates.reserve(definitions.size());
    std::vector<mf::ui::WidgetState> nextVisualStates;
    nextVisualStates.reserve(definitions.size());
    std::size_t focusIndex{};
    if (!definitions.empty()) ui.setKeyboardFocus(definitions.front().id);
    const auto runStart = std::chrono::steady_clock::now();
    auto settingChangeStarted = std::chrono::steady_clock::time_point{};
    std::optional<SettingId> pulsedSetting{std::nullopt};
    bool settingChangeAnimationActive = false;
    float settingChangePulse = 0.0F;

    while (!window.closeRequested()) {
        framePacer.beginFrame();
        input.beginFrame();
        eventPump.poll(frameEvents);
        for (const auto& event : frameEvents) {
            input.consume(event);
            if (std::holds_alternative<mf::QuitEvent>(event)) window.requestClose();
            if (const auto* activity = std::get_if<mf::WindowActivityEvent>(&event)) {
                focused = activity->focused;
                minimized = activity->minimized;
            }
        }
        if (window.closeRequested()) break;
        if (minimized) {
            (void)framePacer.wait(true);
            continue;
        }

        if (input.keyPressed(mf::Key::escape)) {
            if (!application.back()) {
                window.requestClose();
            } else {
                definitions = buttonsFor(application.screen(), application);
                focusIndex = 0;
                if (!definitions.empty()) ui.setKeyboardFocus(definitions.front().id);
                visualStates.assign(definitions.size(), mf::ui::WidgetState::normal);
                visualScreen = application.screen();
            }
            overlayDirty = true;
        }
        if (window.closeRequested()) break;

        if (!definitions.empty() && (input.keyPressed(mf::Key::tab) || input.keyPressed(mf::Key::down))) {
            focusIndex = (focusIndex + 1) % definitions.size();
            ui.setKeyboardFocus(definitions[focusIndex].id);
            overlayDirty = true;
        } else if (!definitions.empty() && input.keyPressed(mf::Key::up)) {
            focusIndex = (focusIndex + definitions.size() - 1) % definitions.size();
            ui.setKeyboardFocus(definitions[focusIndex].id);
            overlayDirty = true;
        }

        const auto windowSize = window.size();
        canvas.setTargetSize({static_cast<float>(windowSize.width), static_cast<float>(windowSize.height)});
        ui.beginFrame(input, canvas);
        nextVisualStates.clear();
        if (nextVisualStates.capacity() < definitions.size()) nextVisualStates.reserve(definitions.size());
        std::optional<ButtonDefinition> activated;
        for (const auto& definition : definitions) {
            const auto result = ui.button(definition.id, definition.bounds, definition.enabled);
            nextVisualStates.push_back(result.state);
            if (result.activated && !activated) activated = definition;
        }
        ui.endFrame();
        if (const auto focusedButton = std::ranges::find(definitions, ui.keyboardFocus(), &ButtonDefinition::id);
            focusedButton != definitions.end()) {
            focusIndex = static_cast<std::size_t>(std::distance(definitions.begin(), focusedButton));
        }
        if (nextVisualStates != visualStates || visualScreen != application.screen()) overlayDirty = true;
        visualStates = nextVisualStates;
        visualScreen = application.screen();

        if (activated) {
            const auto oldSettings = application.settings();
            const auto oldScreen = application.screen();
            if (activated->setting) (void)application.activateSetting(*activated->setting);
            else (void)application.activate(activated->command);
            if (application.quitRequested()) window.requestClose();
            if (window.closeRequested()) break;

            if (application.settings().verticalSync != oldSettings.verticalSync) {
                auto updated = applyPresentMode(device, window, application.settings().verticalSync);
                if (!updated) return report(updated.error());
                presentMode = updated;
            }
            if (application.settings().fpsLimit != oldSettings.fpsLimit) {
                framePacer.setLimit(run.uncappedProfiling ? 0U : application.settings().fpsLimit);
            }
            if (application.settings().quality != oldSettings.quality) {
                auto updatedTargets = createTargets(renderer, application.settings().quality);
                if (!updatedTargets) return report(updatedTargets.error());
                targets = std::move(*updatedTargets);
                worldDirty = true;
            }
            if (activated->setting && application.settings() != oldSettings) {
                pulsedSetting = activated->setting;
                settingChangeStarted = std::chrono::steady_clock::now();
                settingChangeAnimationActive = true;
            }
            if (application.screen() != oldScreen) {
                definitions = buttonsFor(application.screen(), application);
                focusIndex = 0;
                if (!definitions.empty()) ui.setKeyboardFocus(definitions.front().id);
                visualStates.assign(definitions.size(), mf::ui::WidgetState::normal);
                visualScreen = application.screen();
            }
            overlayDirty = true;
        }

        if (settingChangeAnimationActive) {
            const auto elapsed = std::chrono::steady_clock::now() - settingChangeStarted;
            const float progress = std::chrono::duration<float>(elapsed).count() /
                                   std::chrono::duration<float>(settingChangeAnimationDuration).count();
            settingChangePulse = std::clamp(1.0F - progress, 0.0F, 1.0F);
            overlayDirty = true;
            if (elapsed >= settingChangeAnimationDuration) {
                settingChangeAnimationActive = false;
                pulsedSetting.reset();
            }
        }

        renderer.beginFrame();
        if (worldDirty) {
            if (!begin(renderer, targets.world, {0.004F, 0.010F, 0.014F, 1}, true, "E2.1 menu world")) return 2;
            drawStaticWorld(renderer, assets);
            if (!end(renderer)) return 2;
            if (!begin(renderer, targets.emissive, {0, 0, 0, 0}, true, "E2.1 menu emissive")) return 2;
            drawStaticEmissive(renderer, assets);
            if (!end(renderer)) return 2;
            worldDirty = false;
        } else {
            renderer.addCachedSprites(9);
        }
        if (overlayDirty) {
            if (!begin(renderer, targets.overlay, {0, 0, 0, 0}, true, "E2.1 menu UI")) return 2;
            drawOverlay(renderer, assets, application, definitions, visualStates,
                        pulsedSetting, settingChangePulse);
            if (!end(renderer)) return 2;
            overlayDirty = false;
        } else {
            const std::size_t cachedSprites = application.screen() == ScreenId::mainMenu ? 86U :
                                              application.screen() == ScreenId::settings ? 84U : 70U;
            renderer.addCachedSprites(cachedSprites);
        }

        const auto quality = qualityConfiguration(application.settings().quality);
        renderer.setComposite({&targets.world, &targets.emissive, &targets.overlay,
            {0.28F * quality.particleDensity, 1.25F, 0.30F, 1.02F, 1.02F,
             {0.94F, 1.02F, 1.03F, 1}}});
        auto presented = renderer.present();
        if (!presented) return report(presented.error());
        const float limiterWait = framePacer.wait(!focused);
        renderer.setFrameLimitWait(limiterWait);
        if (run.benchmarkSeconds > 0.0F &&
            std::chrono::duration<float>(std::chrono::steady_clock::now() - runStart).count() >=
                run.warmupSeconds + run.benchmarkSeconds) {
            window.requestClose();
        }
    }
    return 0;
}

} // namespace aegis::mediaforge
