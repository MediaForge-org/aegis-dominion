#include "core/MapCatalog.hpp"
#include "mediaforge/MediaForgeAppModel.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

aegis::core::MapDocument sentinel(std::string id, std::string name, float width, float height,
                                  float middleX, float middleY) {
    aegis::core::MapDocument document;
    document.metadata.id = std::move(id);
    document.metadata.name = std::move(name);
    document.metadata.biome = middleY > height * 0.5F ? "frost" : "ember";
    document.metadata.difficulty = middleX > width * 0.5F ? "Schwer" : "Normal";
    document.width = width;
    document.height = height;
    document.paths.push_back({"sentinel_route", {{0, 31}, {middleX, middleY}, {width, height - 47}}});
    document.spawns.push_back({"sentinel_spawn", {0, 31}, "sentinel_route"});
    document.goals.push_back({"sentinel_goal", {width, height - 47}, "sentinel_route"});
    document.zones.push_back({aegis::core::ZoneType::Buildable, {71, 83, 120, 140}});
    document.environment.terrainSeed = static_cast<unsigned>(middleX + middleY);
    return document;
}

void save(const aegis::core::MapDocument& document, const std::filesystem::path& file) {
    std::string error;
    require(document.save(file.string(), &error), error);
}

} // namespace

int main() {
    const auto directory = std::filesystem::temp_directory_path() / "aegis_e22_map_catalog_tests";
    try {
        const auto realMaps = aegis::core::discoverMapCatalog(std::filesystem::path(AEGIS_SOURCE_DIR) / "maps");
        require(realMaps.size() >= 3 && realMaps[0].valid() && realMaps[1].valid() && realMaps[2].valid() &&
                realMaps[0].stableId() == "verdant_frontier" && realMaps[1].stableId() == "frost_pass" &&
                realMaps[2].stableId() == "ember_field",
                "repository built-in discovery did not preserve Grüne Grenze, Frostpass and Aschefeld IDs");
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
        const auto alpha = sentinel("sentinel_alpha", "ALPHA SENTINEL", 1000, 700, 211, 503);
        const auto beta = sentinel("sentinel_beta", "BETA SENTINEL", 1733, 977, 1289, 211);
        const auto gamma = sentinel("sentinel_gamma", "GAMMA SENTINEL", 1200, 900, 610, 450);
        const auto custom = sentinel("custom_delta", "CUSTOM DELTA", 1440, 960, 901, 723);
        const auto duplicateOne = sentinel("duplicate_id", "DUPLICATE ONE", 1300, 800, 500, 300);
        const auto duplicateTwo = sentinel("duplicate_id", "DUPLICATE TWO", 1500, 1000, 700, 600);
        save(alpha, directory / "verdant.aegismap");
        save(beta, directory / "frost.aegismap");
        save(gamma, directory / "ember.aegismap");
        save(custom, directory / "custom_delta.aegismap");
        save(duplicateOne, directory / "duplicate_one.aegismap");
        save(duplicateTwo, directory / "duplicate_two.aegismap");
        auto invalidSemantic = sentinel("invalid_semantic", "INVALID SEMANTIC", 1200, 900, 600, 450);
        invalidSemantic.goals.clear();
        save(invalidSemantic, directory / "invalid_semantic.aegismap");
        {
            std::ofstream invalid(directory / "broken.aegismap", std::ios::binary);
            invalid << "NOT_AN_AEGIS_MAP\n";
        }

        auto catalog = aegis::core::discoverMapCatalog(directory);
        require(catalog.size() == 8, "real built-in/custom discovery count changed");
        require(catalog[0].source == aegis::core::MapSource::BuiltIn && catalog[0].stableId() == "sentinel_alpha" &&
                catalog[1].stableId() == "sentinel_beta" && catalog[2].stableId() == "sentinel_gamma",
                "built-in ordering or stable MapDocument IDs changed");
        require(catalog[0].valid() && catalog[1].valid() && catalog[2].valid(), "valid built-in map was disabled");
        require(catalog[3].source == aegis::core::MapSource::Custom && !catalog[3].valid() &&
                catalog[3].error.contains("Unbekanntes Map-Format"), "malformed custom map was not exposed as invalid");
        require(catalog[4].stableId() == "custom_delta" && catalog[4].valid(), "valid MAP FORGE map was not discovered");
        require(catalog[5].stableId() == "duplicate_id" && !catalog[5].valid() &&
                catalog[5].error.contains("bereits vergeben") && catalog[6].stableId() == "duplicate_id" &&
                !catalog[6].valid() && catalog[6].error.contains("bereits vergeben"),
                "every entry sharing a duplicate stable map ID must be unavailable");
        require(catalog[7].stableId() == "invalid_semantic" && !catalog[7].valid() &&
                catalog[7].error.contains("Mindestens ein Zielpunkt"),
                "authoritative semantic validation failure remained startable");

        aegis::mediaforge::MediaForgeAppModel application({}, std::move(catalog));
        require(application.activate(aegis::mediaforge::AppCommand::play) &&
                application.screen() == aegis::mediaforge::ScreenId::mapSelection,
                "SPIELEN did not enter real map selection");
        require(!application.selectedMap() && !application.canStart() && !application.startSelectedMap(),
                "START was enabled without a selected map");
        require(!application.selectMap(3) && !application.isMapSelected(3) && !application.canStart(),
                "invalid map card became selectable/startable");

        require(application.selectMap(0) && application.isMapSelected(0) && application.canStart(),
                "valid map A selection/card state failed");
        require(application.selectedMap()->document->metadata.name == "ALPHA SENTINEL" &&
                application.selectedMap()->document->metadata.difficulty == "Normal",
                "selected-map live metadata did not come from MapDocument");
        const auto alphaPreview = application.previewIdentity();
        require(alphaPreview.contains("sentinel_alpha") && alphaPreview.contains("sentinel_route"),
                "map A preview identity lost real geometry metadata");
        require(application.previewIdentity() == alphaPreview,
                "unchanged map selection did not retain its cached preview identity");

        require(application.selectMap(1) && application.isMapSelected(1) && !application.isMapSelected(0),
                "selection change to map B failed");
        const auto betaPreview = application.previewIdentity();
        require(betaPreview != alphaPreview && betaPreview.contains("sentinel_beta"),
                "preview identity did not change with selected map");
        require(application.startSelectedMap() && application.screen() == aegis::mediaforge::ScreenId::gameplay,
                "valid START did not enter gameplay");
        const auto* betaSession = application.gameSession();
        require(betaSession && !betaSession->isMapForgePlaytest() && betaSession->map().id == "sentinel_beta",
                "standard launch/session lost selected map B identity");
        require(betaSession->map().width == 1733 && betaSession->map().height == 977 &&
                betaSession->map().route[1].x == 1289 && betaSession->map().route[1].y == 211 &&
                betaSession->map().spawn.y == 31 && betaSession->map().goal.x == 1733,
                "selected map B dimensions/path/spawn/goal did not reach gameplay");
        require(betaSession->map().id != "verdant_frontier" && betaSession->map().id != "sentinel_alpha",
                "map B launch fell back to Verdant, index zero, or the previous selection");
        require(application.back() && application.screen() == aegis::mediaforge::ScreenId::mapSelection &&
                !application.gameSession() && application.isMapSelected(1),
                "gameplay back did not predictably restore map selection");

        require(application.selectMap(0) && application.startSelectedMap() &&
                application.gameSession()->map().route[1].x == 211,
                "second launch did not carry map A sentinel geometry");
        require(application.back() && application.selectMap(1) && application.startSelectedMap() &&
                application.gameSession()->map().route[1].x == 1289,
                "two sentinel launches crossed or reused previous gameplay data");
        require(application.back(), "second gameplay return failed");

        require(application.moveMapSelection(1) && application.selectedMapIndex() == 2,
                "keyboard-style next-map navigation failed");
        require(application.moveMapSelection(1) && application.selectedMapIndex() == 4,
                "keyboard navigation did not skip invalid cards");
        require(application.firstVisibleMap() == 1, "selected custom map was not kept in the four-row viewport");
        require(application.scrollMaps(10) && application.firstVisibleMap() == 4 &&
                !application.scrollMaps(1), "bounded list scrolling failed");
        require(application.back() && application.screen() == aegis::mediaforge::ScreenId::mainMenu,
                "map-selection ZURÜCK did not return to main menu");

        std::filesystem::remove_all(directory);
        std::cout << "E2.2 real map discovery, selection, scrolling and launch handoff checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::filesystem::remove_all(directory);
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
