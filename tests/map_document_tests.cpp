#include "core/MapDocument.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::filesystem::path tempMap(const std::string& name) {
    return std::filesystem::temp_directory_path() / ("aegis_dominion_" + name + ".aegismap");
}

aegis::core::MapDocument validDocument() {
    aegis::core::MapDocument document;
    document.metadata.id = "test_map";
    document.metadata.name = "Prüfkarte ÄÖÜ äöü ß";
    document.metadata.description = "Größe, Höhe und grüner Außenposten";
    document.paths.push_back({"main", {{0.f, 100.f}, {400.f, 100.f}, {800.f, 500.f}}});
    document.spawns.push_back({"spawn_0", {0.f, 100.f}, "main"});
    document.goals.push_back({"goal_0", {800.f, 500.f}, "main"});
    document.zones.push_back({aegis::core::ZoneType::Buildable, {50.f, 180.f, 300.f, 250.f}});
    document.decorations.push_back({"tree_pine_01", {200.f, 300.f}, 17.f, 1.1f, 1});
    return document;
}

void roundTripPreservesAllV1Data() {
    const auto path = tempMap("roundtrip");
    const auto source = validDocument();
    std::string error;
    require(source.save(path.string(), &error), error);
    const auto loaded = aegis::core::MapDocument::load(path.string(), &error);
    require(loaded.has_value(), error);
    require(loaded->metadata.name == source.metadata.name, "UTF-8 map name changed");
    require(loaded->metadata.description == source.metadata.description, "UTF-8 description changed");
    require(loaded->paths.front().nodes.size() == 3, "path nodes changed");
    require(loaded->spawns.front().id == "spawn_0" && loaded->goals.front().id == "goal_0", "endpoint IDs changed");
    require(loaded->zones.size() == 1 && loaded->decorations.size() == 1, "map layers changed");
    std::filesystem::remove(path);
}

bool hasErrorContaining(const aegis::core::MapDocument& document, const std::string& text) {
    const auto issues = document.validateDetailed();
    return std::any_of(issues.begin(), issues.end(), [&](const auto& issue) {
        return issue.severity == aegis::core::ValidationSeverity::Error && issue.message.find(text) != std::string::npos;
    });
}

void missingPathIsAnError() { auto document = validDocument(); document.paths.clear(); require(hasErrorContaining(document, "Gegnerpfad"), "missing path was not an error"); }
void missingSpawnIsAnError() { auto document = validDocument(); document.spawns.clear(); require(hasErrorContaining(document, "Spawnpunkt"), "missing spawn was not an error"); }
void missingGoalIsAnError() { auto document = validDocument(); document.goals.clear(); require(hasErrorContaining(document, "Zielpunkt"), "missing goal was not an error"); }

void validMinimalMapHasNoFatalErrors() {
    auto document = validDocument();
    document.zones.clear(); document.decorations.clear(); document.paths.front().nodes.resize(2);
    const auto issues = document.validateDetailed();
    require(std::none_of(issues.begin(), issues.end(), [](const auto& issue) {
        return issue.severity == aegis::core::ValidationSeverity::Error;
    }), "valid minimal map has fatal errors");
}

void outOfBoundsAndInvalidGeometryAreErrors() {
    auto document = validDocument();
    document.paths.front().nodes.front().x = -1.f;
    document.spawns.front().position.x = document.width + 1.f;
    document.goals.front().position.y = document.height + 1.f;
    document.zones.front().rect.w = -2.f;
    document.decorations.front().scale = 0.f;
    require(document.validateDetailed().size() >= 5, "invalid geometry was not fully reported");
}

void duplicateStableIdsAreErrors() {
    auto document = validDocument();
    document.paths.push_back(document.paths.front());
    document.spawns.push_back(document.spawns.front());
    document.goals.push_back(document.goals.front());
    const auto issues = document.validate();
    require(std::count_if(issues.begin(), issues.end(), [](const std::string& value) { return value.find("doppelt") != std::string::npos; }) == 3,
            "duplicate path/spawn/goal IDs were not reported");
}

void warningDoesNotMakeMapFatal() {
    auto document = validDocument();
    document.zones.clear();
    const auto issues = document.validateDetailed();
    require(std::any_of(issues.begin(), issues.end(), [](const auto& issue) { return issue.severity == aegis::core::ValidationSeverity::Warning; }),
            "missing build zone warning is absent");
    require(std::none_of(issues.begin(), issues.end(), [](const auto& issue) { return issue.severity == aegis::core::ValidationSeverity::Error; }),
            "warning-only map became fatal");
}

void semanticallyInvalidDraftCanBeLoadedForRepair() {
    auto document = validDocument();
    document.spawns.clear();
    const auto path = tempMap("draft");
    std::string error;
    require(document.save(path.string(), &error), error);
    const auto loaded = aegis::core::MapDocument::load(path.string(), &error);
    require(loaded.has_value(), "editor draft could not be loaded: " + error);
    require(!loaded->validate().empty(), "invalid draft unexpectedly validates");
    std::filesystem::remove(path);
}

void malformedAndUnknownFormatsFailCleanly() {
    const auto unknown = tempMap("unknown_version");
    { std::ofstream out(unknown, std::ios::binary); out << "AEGIS_MAP_V99\n"; }
    std::string error;
    require(!aegis::core::MapDocument::load(unknown.string(), &error), "unknown version accepted");
    require(error.find("Unbekanntes Map-Format") != std::string::npos, "unknown version error unclear");
    std::filesystem::remove(unknown);

    const auto malformed = tempMap("malformed");
    { std::ofstream out(malformed, std::ios::binary); out << "AEGIS_MAP_V1\nsize broken data\n"; }
    require(!aegis::core::MapDocument::load(malformed.string(), &error), "malformed data accepted");
    require(error.find("Fehlerhafte Map-Daten") != std::string::npos, "malformed error unclear");
    std::filesystem::remove(malformed);

    auto unsupported = validDocument();
    unsupported.metadata.formatVersion = 99;
    require(!unsupported.save(tempMap("unsupported_save").string(), &error), "unsupported version written as V1");
}

void builtInMapsRemainValid() {
    const std::array<std::string, 3> names = {"verdant", "frost", "ember"};
    for (const auto& name : names) {
        std::string error;
        const auto path = std::filesystem::path(AEGIS_SOURCE_DIR) / "maps" / (name + ".aegismap");
        const auto document = aegis::core::MapDocument::load(path.string(), &error);
        require(document.has_value(), name + " failed to load: " + error);
        require(document->validateDetailed().empty(), name + " is invalid");
    }
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"V1 round-trip", roundTripPreservesAllV1Data},
        {"missing path", missingPathIsAnError}, {"missing spawn", missingSpawnIsAnError}, {"missing goal", missingGoalIsAnError},
        {"valid minimal map", validMinimalMapHasNoFatalErrors},
        {"invalid geometry", outOfBoundsAndInvalidGeometryAreErrors},
        {"duplicate stable IDs", duplicateStableIdsAreErrors},
        {"warning severity", warningDoesNotMakeMapFatal},
        {"repairable draft load", semanticallyInvalidDraftCanBeLoadedForRepair},
        {"parser failures", malformedAndUnknownFormatsFailCleanly},
        {"built-in maps", builtInMapsRemainValid}
    };
    try {
        for (const auto& test : tests) test.second();
        std::cout << tests.size() << " MapDocument test cases passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
