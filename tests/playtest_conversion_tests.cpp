#include "core/GameLaunchConfig.hpp"

#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

aegis::core::MapDocument playableDocument() {
    aegis::core::MapDocument document;
    document.metadata.id = "playtest"; document.metadata.name = "Playtest";
    document.width = 1600.f; document.height = 1000.f;
    document.paths.push_back({"route_a", {{0.f, 100.f}, {800.f, 400.f}, {1600.f, 700.f}}});
    document.spawns.push_back({"spawn_a", {0.f, 100.f}, "route_a"});
    document.goals.push_back({"goal_a", {1600.f, 700.f}, "route_a"});
    document.zones.push_back({aegis::core::ZoneType::Blocked, {200.f, 200.f, 100.f, 100.f}});
    return document;
}

aegis::core::MapDocument sentinelDocument() {
    aegis::core::MapDocument document;
    document.metadata.id = "playtest_sentinel_id";
    document.metadata.name = "PLAYTEST_SENTINEL_MAP";
    document.metadata.biome = "sentinel_material";
    document.width = 1733.f;
    document.height = 977.f;
    document.paths.push_back({"sentinel_route", {{37.f, 83.f}, {419.f, 731.f}, {1289.f, 211.f}, {1691.f, 901.f}}});
    document.spawns.push_back({"sentinel_spawn", {37.f, 83.f}, "sentinel_route"});
    document.goals.push_back({"sentinel_goal", {1691.f, 901.f}, "sentinel_route"});
    document.zones.push_back({aegis::core::ZoneType::Buildable, {111.f, 444.f, 222.f, 123.f}});
    document.zones.push_back({aegis::core::ZoneType::Blocked, {777.f, 55.f, 91.f, 203.f}});
    document.zones.push_back({aegis::core::ZoneType::Water, {1401.f, 501.f, 177.f, 311.f}});
    return document;
}

const aegis::core::PlayableMap& playtestMap(const aegis::core::GameLaunchConfig& config) {
    const auto* playtest = std::get_if<aegis::core::MapForgePlaytestLaunch>(&config);
    require(playtest != nullptr, "launch silently became a standard-map launch");
    return playtest->map;
}

void validDocumentConverts() {
    const auto document = playableDocument();
    std::string error;
    const auto playable = aegis::core::buildPlayableMap(document, &error);
    require(playable.has_value(), error);
    require(playable->route.size() == 3 && playable->spawn.x == 0.f && playable->goal.x == 1600.f, "route/endpoints changed");
    require(playable->zones.size() == 1 && playable->width == 1600.f, "zones/world dimensions changed");
}

void fatalValidationBlocksConversion() {
    auto document = playableDocument(); document.goals.clear();
    std::string error;
    require(!aegis::core::buildPlayableMap(document, &error), "invalid document converted");
    require(error.find("Zielpunkt") != std::string::npos, "conversion error unclear");
}

void conversionDoesNotMutateDocument() {
    const auto document = playableDocument();
    const auto originalName = document.metadata.name;
    const auto originalNode = document.paths.front().nodes[1];
    const auto originalZone = document.zones.front().rect;
    require(aegis::core::buildPlayableMap(document).has_value(), "conversion failed");
    require(document.metadata.name == originalName && document.paths.front().nodes[1].x == originalNode.x &&
            document.zones.front().rect.w == originalZone.w, "conversion mutated editor document");
}

void conversionChoosesRouteWithEndpoints() {
    auto document = playableDocument();
    document.paths.insert(document.paths.begin(), {"unfinished", {{10.f, 10.f}, {20.f, 20.f}}});
    const auto playable = aegis::core::buildPlayableMap(document);
    require(playable.has_value() && playable->routeId == "route_a" && playable->route.front().x == 0.f, "conversion chose a route without endpoints");
}

void unsavedSentinelLaunchPreservesIdentityAndGeometry() {
    const auto document = sentinelDocument();
    std::string error;
    const auto launch = aegis::core::makeMapForgePlaytestLaunch(document, &error);
    require(launch.has_value(), error);
    const auto& map = playtestMap(*launch);
    require(map.id == "playtest_sentinel_id" && map.name == "PLAYTEST_SENTINEL_MAP", "sentinel identity changed");
    require(map.width == 1733.f && map.height == 977.f, "sentinel dimensions changed");
    require(map.route.size() == 4 && map.route[1].x == 419.f && map.route[2].y == 211.f, "sentinel path changed");
    require(map.spawn.x == 37.f && map.spawn.y == 83.f, "sentinel spawn changed");
    require(map.goal.x == 1691.f && map.goal.y == 901.f, "sentinel goal changed");
    require(map.zones.size() == 3 && map.zones[0].type == aegis::core::ZoneType::Buildable &&
            map.zones[1].type == aegis::core::ZoneType::Blocked && map.zones[2].type == aegis::core::ZoneType::Water,
            "sentinel zones changed");
}

void playtestLaunchCannotRepresentStandardFallback() {
    const auto launch = aegis::core::makeMapForgePlaytestLaunch(sentinelDocument());
    require(launch.has_value(), "sentinel launch failed");
    require(!std::holds_alternative<aegis::core::StandardGameLaunch>(*launch), "MAP FORGE launch contains a standard map index");
    require(playtestMap(*launch).id != "verdant_frontier", "MAP FORGE launch fell back to Verdant");
}

void savedAndReloadedSentinelProducesSameLaunch() {
    const auto source = sentinelDocument();
    const auto file = std::filesystem::temp_directory_path() / "aegis_playtest_sentinel.aegismap";
    std::string error;
    require(source.save(file.string(), &error), error);
    const auto loaded = aegis::core::MapDocument::load(file.string(), &error);
    require(loaded.has_value(), error);
    const auto memoryLaunch = aegis::core::makeMapForgePlaytestLaunch(source, &error);
    const auto loadedLaunch = aegis::core::makeMapForgePlaytestLaunch(*loaded, &error);
    require(memoryLaunch.has_value() && loadedLaunch.has_value(), error);
    const auto& memoryMap = playtestMap(*memoryLaunch);
    const auto& loadedMap = playtestMap(*loadedLaunch);
    require(loadedMap.id == memoryMap.id && loadedMap.name == memoryMap.name && loadedMap.width == memoryMap.width && loadedMap.height == memoryMap.height,
            "saved launch identity differs");
    require(loadedMap.route.size() == memoryMap.route.size() && loadedMap.route[2].x == memoryMap.route[2].x &&
            loadedMap.spawn.y == memoryMap.spawn.y && loadedMap.goal.x == memoryMap.goal.x && loadedMap.zones.size() == memoryMap.zones.size(),
            "saved launch geometry differs");
    std::filesystem::remove(file);
}
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"valid conversion", validDocumentConverts}, {"invalid conversion", fatalValidationBlocksConversion},
        {"immutable conversion", conversionDoesNotMutateDocument}, {"multi-route selection", conversionChoosesRouteWithEndpoints},
        {"unsaved sentinel launch", unsavedSentinelLaunchPreservesIdentityAndGeometry},
        {"no standard fallback", playtestLaunchCannotRepresentStandardFallback},
        {"saved sentinel launch", savedAndReloadedSentinelProducesSameLaunch}
    };
    try { for (const auto& test : tests) test.second(); std::cout << tests.size() << " playtest conversion cases passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << "TEST FAILURE: " << error.what() << '\n'; return 1; }
}
