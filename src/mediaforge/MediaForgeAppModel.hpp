#pragma once

#include "E2Performance.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace aegis::mediaforge {

enum class ScreenId { mainMenu, play, mapForge, tutorial, settings };

enum class SettingId { verticalSync, fpsLimit, graphicsQuality };
enum class SettingCategory { display, graphics };
enum class SettingType { discreteCyclic, boolean, continuousNumeric, keyBinding, action };

struct SettingCategoryMetadata {
    SettingCategory id{};
    std::string_view stableId;
    std::string_view displayLabel;
};

struct SettingOption {
    std::string_view stableId;
    std::string_view displayValue;
};

struct SettingDescriptor {
    SettingId id{};
    std::string_view stableId;
    std::string_view displayLabel;
    SettingCategory category{};
    SettingType type{};
    std::span<const SettingOption> availableValues;
    std::size_t currentIndex{};
    bool wraps{};
    std::string_view description;
    bool disabled{};
    bool restartRequired{};

    [[nodiscard]] constexpr std::size_t stepCount() const noexcept { return availableValues.size(); }
    [[nodiscard]] constexpr std::size_t activeSegmentCount() const noexcept {
        return availableValues.empty() ? 0 :
            (currentIndex < availableValues.size() ? currentIndex : availableValues.size() - 1) + 1;
    }
    [[nodiscard]] constexpr std::string_view displayValue() const noexcept {
        return availableValues.empty() ? std::string_view{} :
            availableValues[currentIndex < availableValues.size() ? currentIndex : availableValues.size() - 1]
                .displayValue;
    }
    [[nodiscard]] constexpr bool usesSegmentedIndicator() const noexcept {
        return type == SettingType::discreteCyclic || type == SettingType::boolean;
    }
};

enum class AppCommand {
    play,
    mapForge,
    tutorial,
    settings,
    quit,
    back,
    toggleVsync,
    cycleFpsLimit,
    cycleQuality,
};

struct RuntimeSettings {
    bool verticalSync{true};
    unsigned fpsLimit{120};
    GraphicsQuality quality{GraphicsQuality::high};
    friend constexpr bool operator==(RuntimeSettings, RuntimeSettings) noexcept = default;
};

[[nodiscard]] SettingCategoryMetadata settingCategoryMetadata(SettingCategory category) noexcept;
[[nodiscard]] std::vector<SettingDescriptor> settingDescriptors(const RuntimeSettings& settings);
[[nodiscard]] std::optional<SettingDescriptor> settingDescriptor(const RuntimeSettings& settings,
                                                                 SettingId id) noexcept;

class MediaForgeAppModel {
public:
    explicit MediaForgeAppModel(RuntimeSettings settings = {});

    [[nodiscard]] ScreenId screen() const noexcept { return screen_; }
    [[nodiscard]] const RuntimeSettings& settings() const noexcept { return settings_; }
    [[nodiscard]] std::vector<SettingDescriptor> settingDescriptors() const {
        return ::aegis::mediaforge::settingDescriptors(settings_);
    }
    [[nodiscard]] std::optional<SettingDescriptor> settingDescriptor(SettingId id) const noexcept {
        return ::aegis::mediaforge::settingDescriptor(settings_, id);
    }
    [[nodiscard]] bool quitRequested() const noexcept { return quitRequested_; }
    [[nodiscard]] bool activate(AppCommand command);
    [[nodiscard]] bool activateSetting(SettingId id);
    [[nodiscard]] bool back();

private:
    void open(ScreenId screen);

    ScreenId screen_{ScreenId::mainMenu};
    RuntimeSettings settings_{};
    std::vector<ScreenId> history_;
    bool quitRequested_{};
};

[[nodiscard]] constexpr std::array<std::string_view, 10> mediaForgeGermanUiText() noexcept {
    return {"SPIELEN", "MAP FORGE", "ANLEITUNG", "EINSTELLUNGEN", "BEENDEN",
            "Zurück", "Qualität", "Überblick", "Schließen", "Gelände"};
}

[[nodiscard]] constexpr std::string_view mediaForgeWindowTitle() noexcept {
    return "AEGIS DOMINION — MediaForge";
}

} // namespace aegis::mediaforge
