#include "mediaforge/VerdantSliceData.hpp"
#include "mediaforge/E2Performance.hpp"
#include "mediaforge/MediaForgeAppModel.hpp"

#include <mediaforge/ui/SegmentedLevelIndicator.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(float left, float right) { return std::abs(left - right) < 0.001F; }

bool validUtf8(std::string_view text) {
    for (std::size_t index = 0; index < text.size();) {
        const auto lead = static_cast<unsigned char>(text[index]);
        std::size_t trailing{};
        if (lead < 0x80) trailing = 0;
        else if ((lead & 0xE0) == 0xC0) trailing = 1;
        else if ((lead & 0xF0) == 0xE0) trailing = 2;
        else if ((lead & 0xF8) == 0xF0) trailing = 3;
        else return false;
        if (index + trailing >= text.size()) return false;
        for (std::size_t offset = 1; offset <= trailing; ++offset) {
            if ((static_cast<unsigned char>(text[index + offset]) & 0xC0) != 0x80) return false;
        }
        index += trailing + 1;
    }
    return true;
}

}

int main() {
    try {
        const auto snapshot = aegis::mediaforge::makeVerdantSliceSnapshot(4.0F);
        require(snapshot.map.id == "e2_verdant_reference" && snapshot.map.authoredTerrain,
                "reference snapshot lost its AEGIS map identity");
        require(snapshot.enemies.size() == 8 && snapshot.towers.size() == 2 && snapshot.projectiles.size() == 1,
                "reference snapshot composition changed");
        require(snapshot.enemies.back().visualId == "enemy.heavy_tank" &&
                snapshot.enemies.front().visualId == "enemy.raider", "enemy silhouettes are not distinct logical assets");
        require(snapshot.towers[0].effectProfile == "pulse" && snapshot.towers[1].effectProfile == "rail",
                "reference tower profiles changed");

        const auto primitives = aegis::mediaforge::translateVerdantSlice(snapshot);
        require(primitives.size() >= snapshot.enemies.size() * 2 + snapshot.towers.size() * 2,
                "snapshot translation dropped grounded entity primitives");
        std::size_t shadows{};
        std::size_t entities{};
        for (const auto& primitive : primitives) {
            shadows += primitive.kind == aegis::mediaforge::SliceVisualKind::shadow;
            entities += primitive.kind == aegis::mediaforge::SliceVisualKind::entity;
        }
        require(shadows == snapshot.enemies.size() + snapshot.towers.size(), "each mobile/defense entity needs a shadow");
        require(entities >= snapshot.enemies.size() + snapshot.towers.size() + 2, "landmarks/entities missing from translation");

        const auto first = aegis::mediaforge::deterministicVerdantDetails(91);
        const auto repeat = aegis::mediaforge::deterministicVerdantDetails(91);
        const auto changed = aegis::mediaforge::deterministicVerdantDetails(92);
        require(first.size() == 12 && repeat.size() == first.size(), "Verdant detail density changed");
        require(first.front().logicalId == repeat.front().logicalId && near(first.front().rotationRadians, repeat.front().rotationRadians),
                "same detail seed changed output");
        require(!near(first.front().rotationRadians, changed.front().rotationRadians), "different detail seed did not vary output");
        require(aegis::mediaforge::qualityConfiguration(aegis::mediaforge::GraphicsQuality::high).bloomScale == 0.5F &&
                aegis::mediaforge::qualityConfiguration(aegis::mediaforge::GraphicsQuality::ultra).hdrWorld,
                "High/Ultra quality must retain HDR and optimized bloom configuration");
        using aegis::mediaforge::GraphicsQuality;
        using aegis::mediaforge::RuntimeSettings;
        using aegis::mediaforge::SettingCategory;
        using aegis::mediaforge::SettingId;
        using aegis::mediaforge::SettingType;
        const auto defaultDescriptors = aegis::mediaforge::settingDescriptors(RuntimeSettings{});
        require(defaultDescriptors.size() == 3 &&
                defaultDescriptors[0].stableId == "display.vsync" &&
                defaultDescriptors[1].stableId == "display.fps_limit" &&
                defaultDescriptors[2].stableId == "graphics.preset",
                "current settings lost their stable persistence-ready IDs");
        require(defaultDescriptors[0].type == SettingType::boolean &&
                defaultDescriptors[1].type == SettingType::discreteCyclic &&
                defaultDescriptors[2].type == SettingType::discreteCyclic &&
                SettingType::continuousNumeric != SettingType::keyBinding &&
                SettingType::keyBinding != SettingType::action,
                "generic setting types are not distinguishable");
        require(defaultDescriptors[0].category == SettingCategory::display &&
                defaultDescriptors[1].category == SettingCategory::display &&
                defaultDescriptors[2].category == SettingCategory::graphics &&
                aegis::mediaforge::settingCategoryMetadata(SettingCategory::display).stableId == "display" &&
                aegis::mediaforge::settingCategoryMetadata(SettingCategory::graphics).displayLabel == "GRAFIK",
                "setting category metadata changed");

        for (const bool enabled : {false, true}) {
            const auto vsync = *aegis::mediaforge::settingDescriptor(
                RuntimeSettings{enabled, 120, GraphicsQuality::high}, SettingId::verticalSync);
            require(vsync.stepCount() == 2 && vsync.currentIndex == (enabled ? 1U : 0U) &&
                    vsync.activeSegmentCount() == (enabled ? 2U : 1U) && vsync.usesSegmentedIndicator(),
                    "VSync metadata no longer maps Off/On to 1/2 and 2/2 segments");
        }
        constexpr std::array fpsValues{30U, 60U, 120U, 144U, 240U};
        for (std::size_t index = 0; index < fpsValues.size(); ++index) {
            const auto fps = *aegis::mediaforge::settingDescriptor(
                RuntimeSettings{true, fpsValues[index], GraphicsQuality::high}, SettingId::fpsLimit);
            require(fps.stepCount() == fpsValues.size() && fps.currentIndex == index &&
                    fps.activeSegmentCount() == index + 1,
                    "FPS indicator is not derived from the actual five-value option list");
        }
        constexpr std::array qualityValues{GraphicsQuality::low, GraphicsQuality::medium,
                                            GraphicsQuality::high, GraphicsQuality::ultra};
        for (std::size_t index = 0; index < qualityValues.size(); ++index) {
            const auto qualitySetting = *aegis::mediaforge::settingDescriptor(
                RuntimeSettings{true, 120, qualityValues[index]}, SettingId::graphicsQuality);
            const mf::ui::SegmentedLevelIndicator indicator{qualitySetting.stepCount(),
                                                             qualitySetting.currentIndex};
            require(qualitySetting.stepCount() == qualityValues.size() &&
                    qualitySetting.activeSegmentCount() == index + 1 &&
                    indicator.activeSegmentCount() == qualitySetting.activeSegmentCount(),
                    "graphics quality indicator is not derived from live descriptor state");
        }
        char executable[] = "e2";
        char profile[] = "--profile";
        char stressOption[] = "--stress-particles";
        char stressValue[] = "1000";
        char* arguments[]{executable, profile, stressOption, stressValue};
        const auto run = aegis::mediaforge::parseE2RunConfiguration(4, arguments);
        require(!run.showcase && run.uncappedProfiling && !run.verticalSync && run.fpsLimit == 0 && run.stressParticles == 1000,
                "profiling/stress command-line configuration changed");
        char showcaseOption[] = "--e2-showcase";
        char* showcaseArguments[]{executable, showcaseOption};
        require(aegis::mediaforge::parseE2RunConfiguration(2, showcaseArguments).showcase,
                "Verdant showcase is not explicitly selectable");

        aegis::mediaforge::MediaForgeAppModel application;
        require(application.screen() == aegis::mediaforge::ScreenId::mainMenu && !application.quitRequested(),
                "MediaForge application must start at its real main menu");
        require(application.activate(aegis::mediaforge::AppCommand::play) &&
                application.screen() == aegis::mediaforge::ScreenId::play && application.back() &&
                application.screen() == aegis::mediaforge::ScreenId::mainMenu, "play transition/back navigation failed");
        require(application.activate(aegis::mediaforge::AppCommand::mapForge) && application.back() &&
                application.activate(aegis::mediaforge::AppCommand::tutorial) && application.back() &&
                application.activate(aegis::mediaforge::AppCommand::settings),
                "secondary screen transitions/back navigation failed");
        const auto initialSettings = application.settings();
        require(application.activate(aegis::mediaforge::AppCommand::toggleVsync) &&
                application.activate(aegis::mediaforge::AppCommand::cycleFpsLimit) &&
                application.activate(aegis::mediaforge::AppCommand::cycleQuality) &&
                application.settings().verticalSync != initialSettings.verticalSync &&
                application.settings().fpsLimit != initialSettings.fpsLimit &&
                application.settings().quality != initialSettings.quality, "runtime settings values did not change");
        require(application.activate(aegis::mediaforge::AppCommand::back) &&
                application.screen() == aegis::mediaforge::ScreenId::mainMenu &&
                application.activate(aegis::mediaforge::AppCommand::quit) && application.quitRequested(),
                "settings back or clean quit request failed");
        aegis::mediaforge::MediaForgeAppModel ultraApplication({true, 120, GraphicsQuality::ultra});
        require(ultraApplication.settingDescriptor(SettingId::graphicsQuality)->activeSegmentCount() == 4 &&
                ultraApplication.activateSetting(SettingId::graphicsQuality) &&
                ultraApplication.settings().quality == GraphicsQuality::low &&
                ultraApplication.settingDescriptor(SettingId::graphicsQuality)->activeSegmentCount() == 1,
                "Ultra to Low quality wrap did not immediately restore one active segment");
        aegis::mediaforge::MediaForgeAppModel fpsWrapApplication({true, 240, GraphicsQuality::high});
        require(fpsWrapApplication.activateSetting(SettingId::fpsLimit) &&
                fpsWrapApplication.settings().fpsLimit == 30 &&
                fpsWrapApplication.settingDescriptor(SettingId::fpsLimit)->activeSegmentCount() == 1,
                "FPS last-to-first wrap did not update runtime and indicator state together");
        aegis::mediaforge::MediaForgeAppModel vsyncWrapApplication({true, 120, GraphicsQuality::high});
        require(vsyncWrapApplication.activateSetting(SettingId::verticalSync) &&
                !vsyncWrapApplication.settings().verticalSync &&
                vsyncWrapApplication.settingDescriptor(SettingId::verticalSync)->activeSegmentCount() == 1 &&
                vsyncWrapApplication.activateSetting(SettingId::verticalSync) &&
                vsyncWrapApplication.settings().verticalSync &&
                vsyncWrapApplication.settingDescriptor(SettingId::verticalSync)->activeSegmentCount() == 2,
                "boolean toggle/wrap no longer follows the shared setting activation path");
        for (const auto text : aegis::mediaforge::mediaForgeGermanUiText()) {
            require(validUtf8(text), "MediaForge German UI copy is not valid UTF-8");
        }
        require(aegis::mediaforge::mediaForgeGermanUiText()[5] == "Zurück" &&
                aegis::mediaforge::mediaForgeGermanUiText()[6] == "Qualität" &&
                aegis::mediaforge::mediaForgeGermanUiText()[7] == "Überblick",
                "German umlauts were altered or mojibake was introduced");
        require(validUtf8(aegis::mediaforge::mediaForgeWindowTitle()) &&
                aegis::mediaforge::mediaForgeWindowTitle() == "AEGIS DOMINION — MediaForge" &&
                !aegis::mediaforge::mediaForgeWindowTitle().contains("â"),
                "MediaForge window title contains invalid UTF-8 or mojibake");
        std::cout << "E2/E2.1 snapshot, data-driven settings, UI-flow and performance checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
