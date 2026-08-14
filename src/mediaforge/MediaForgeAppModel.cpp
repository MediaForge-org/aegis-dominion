#include "MediaForgeAppModel.hpp"

#include <algorithm>
#include <iterator>

namespace aegis::mediaforge {
namespace {

constexpr std::array<unsigned, 5> fpsLimits{30, 60, 120, 144, 240};
constexpr std::array graphicsQualities{GraphicsQuality::low, GraphicsQuality::medium,
                                       GraphicsQuality::high, GraphicsQuality::ultra};
constexpr std::array vsyncOptions{
    SettingOption{"off", "AUS"}, SettingOption{"on", "AN"},
};
constexpr std::array fpsOptions{
    SettingOption{"30", "30"}, SettingOption{"60", "60"}, SettingOption{"120", "120"},
    SettingOption{"144", "144"}, SettingOption{"240", "240"},
};
constexpr std::array qualityOptions{
    SettingOption{"low", "LOW"}, SettingOption{"medium", "MEDIUM"},
    SettingOption{"high", "HIGH"}, SettingOption{"ultra", "ULTRA"},
};
constexpr std::array currentSettingIds{SettingId::verticalSync, SettingId::fpsLimit,
                                       SettingId::graphicsQuality};

template <typename Range, typename Value>
std::size_t optionIndex(const Range& options, const Value& value, std::size_t fallback = 0) noexcept {
    const auto found = std::ranges::find(options, value);
    return found == options.end() ? fallback : static_cast<std::size_t>(std::distance(options.begin(), found));
}

SettingDescriptor makeSettingDescriptor(const RuntimeSettings& settings, SettingId id) noexcept {
    switch (id) {
        case SettingId::verticalSync:
            return {id, "display.vsync", "SYNCHRONISIERUNG", SettingCategory::display,
                    SettingType::boolean, vsyncOptions, settings.verticalSync ? 1U : 0U, true,
                    "Synchronisiert die Bildausgabe mit dem Monitor.", false, false};
        case SettingId::fpsLimit:
            return {id, "display.fps_limit", "BILDRATENLIMIT", SettingCategory::display,
                    SettingType::discreteCyclic, fpsOptions, optionIndex(fpsLimits, settings.fpsLimit, 2), true,
                    "Begrenzt die maximale Bildrate und den Energieverbrauch.", false, false};
        case SettingId::graphicsQuality:
            return {id, "graphics.preset", "GRAFIKPROFIL", SettingCategory::graphics,
                    SettingType::discreteCyclic, qualityOptions,
                    optionIndex(graphicsQualities, settings.quality, 2), true,
                    "Wählt das aktive MediaForge-Qualitätsprofil.", false, false};
    }
    return {};
}

} // namespace

SettingCategoryMetadata settingCategoryMetadata(SettingCategory category) noexcept {
    switch (category) {
        case SettingCategory::display: return {category, "display", "ANZEIGE"};
        case SettingCategory::graphics: return {category, "graphics", "GRAFIK"};
    }
    return {SettingCategory::display, "display", "ANZEIGE"};
}

std::vector<SettingDescriptor> settingDescriptors(const RuntimeSettings& settings) {
    std::vector<SettingDescriptor> descriptors;
    descriptors.reserve(currentSettingIds.size());
    for (const auto id : currentSettingIds) descriptors.push_back(makeSettingDescriptor(settings, id));
    return descriptors;
}

std::optional<SettingDescriptor> settingDescriptor(const RuntimeSettings& settings, SettingId id) noexcept {
    return makeSettingDescriptor(settings, id);
}

MediaForgeAppModel::MediaForgeAppModel(RuntimeSettings settings) : settings_(settings) {
    if (std::ranges::find(fpsLimits, settings_.fpsLimit) == fpsLimits.end()) settings_.fpsLimit = 120;
}

void MediaForgeAppModel::open(ScreenId screen) {
    if (screen == screen_) return;
    history_.push_back(screen_);
    screen_ = screen;
}

bool MediaForgeAppModel::back() {
    if (history_.empty()) return false;
    screen_ = history_.back();
    history_.pop_back();
    return true;
}

bool MediaForgeAppModel::activate(AppCommand command) {
    switch (command) {
        case AppCommand::play: open(ScreenId::play); break;
        case AppCommand::mapForge: open(ScreenId::mapForge); break;
        case AppCommand::tutorial: open(ScreenId::tutorial); break;
        case AppCommand::settings: open(ScreenId::settings); break;
        case AppCommand::quit: quitRequested_ = true; break;
        case AppCommand::back: return back();
        case AppCommand::toggleVsync: return activateSetting(SettingId::verticalSync);
        case AppCommand::cycleFpsLimit: return activateSetting(SettingId::fpsLimit);
        case AppCommand::cycleQuality: return activateSetting(SettingId::graphicsQuality);
    }
    return true;
}

bool MediaForgeAppModel::activateSetting(SettingId id) {
    const auto descriptor = settingDescriptor(id);
    if (!descriptor || descriptor->disabled || descriptor->availableValues.empty()) return false;
    std::size_t nextIndex = descriptor->currentIndex + 1;
    if (nextIndex >= descriptor->availableValues.size()) {
        if (!descriptor->wraps) return false;
        nextIndex = 0;
    }
    switch (id) {
        case SettingId::verticalSync: settings_.verticalSync = nextIndex != 0; break;
        case SettingId::fpsLimit: settings_.fpsLimit = fpsLimits[nextIndex]; break;
        case SettingId::graphicsQuality: settings_.quality = graphicsQualities[nextIndex]; break;
    }
    return true;
}

} // namespace aegis::mediaforge
