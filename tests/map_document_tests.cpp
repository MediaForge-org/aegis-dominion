#include "core/MapDocument.hpp"
#include "editor/MapEditorModel.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::filesystem::path tempMap(const std::string& name) {
    return std::filesystem::temp_directory_path() / ("aegis_dominion_" + name + ".aegismap");
}

aegis::core::MapDocument validDocument() {
    aegis::editor::MapEditorModel editor;
    editor.newMap("test_map", "Prüfkarte ÄÖÜ äöü ß");
    editor.document().metadata.description = "Größe, Höhe und grüner Außenposten";
    editor.addPathNode({0.f, 100.f});
    editor.addPathNode({400.f, 100.f});
    editor.addPathNode({800.f, 500.f});
    editor.setSpawn({0.f, 100.f});
    editor.setGoal({800.f, 500.f});
    editor.addZone(aegis::core::ZoneType::Buildable, {50.f, 180.f, 300.f, 250.f});
    editor.addDecoration("tree_pine_01", {200.f, 300.f}, 17.f, 1.1f, 1);
    return editor.document();
}

void testRoundTripPreservesDataAndUtf8() {
    const auto path = tempMap("roundtrip");
    const auto source = validDocument();
    std::string error;
    require(source.save(path.string(), &error), error);
    const auto loaded = aegis::core::MapDocument::load(path.string(), &error);
    require(loaded.has_value(), error);
    require(loaded->metadata.name == source.metadata.name, "UTF-8 map name changed during round-trip");
    require(loaded->metadata.description == source.metadata.description, "UTF-8 description changed during round-trip");
    require(loaded->metadata.formatVersion == aegis::core::CurrentMapFormatVersion, "format version was not restored");
    require(loaded->paths.at(0).nodes.size() == 3, "path nodes were not preserved");
    require(loaded->zones.size() == 1, "zones were not preserved");
    require(loaded->decorations.size() == 1, "decorations were not preserved");
    std::filesystem::remove(path);
}

void testInvalidDocumentsAreRejected() {
    auto document = validDocument();
    document.spawns.front().position.x = document.width + 1.f;
    document.zones.front().rect.w = -2.f;
    document.decorations.front().scale = 0.f;
    require(document.validate().size() >= 3, "invalid geometry was not reported");

    document = validDocument();
    document.paths.push_back(document.paths.front());
    require(!document.validate().empty(), "duplicate path ID was not reported");
}

void testMalformedAndUnknownFormatsFailCleanly() {
    const auto unknown = tempMap("unknown_version");
    {
        std::ofstream out(unknown, std::ios::binary);
        out << "AEGIS_MAP_V99\n";
    }
    std::string error;
    require(!aegis::core::MapDocument::load(unknown.string(), &error), "unknown version was accepted");
    require(error.find("Unbekanntes Map-Format") != std::string::npos, "unknown version error is unclear");
    std::filesystem::remove(unknown);

    const auto malformed = tempMap("malformed");
    {
        std::ofstream out(malformed, std::ios::binary);
        out << "AEGIS_MAP_V1\nsize broken data\n";
    }
    require(!aegis::core::MapDocument::load(malformed.string(), &error), "malformed data was accepted");
    require(error.find("Fehlerhafte Map-Daten") != std::string::npos, "malformed data error is unclear");
    std::filesystem::remove(malformed);

    auto unsupported = validDocument();
    unsupported.metadata.formatVersion = 99;
    require(!unsupported.save(tempMap("unsupported_save").string(), &error), "unsupported version was written as V1");
}

void testEditorUndoRedoAndBranchingHistory() {
    aegis::editor::MapEditorModel editor;
    editor.newMap("undo_map", "Undo Karte");
    editor.addPathNode({0.f, 50.f});
    editor.addPathNode({300.f, 50.f});
    require(editor.document().paths.front().nodes.size() == 2, "editor action failed");
    require(editor.undo(), "undo was unavailable");
    require(editor.document().paths.front().nodes.size() == 1, "undo restored wrong state");
    require(editor.redo(), "redo was unavailable");
    require(editor.document().paths.front().nodes.size() == 2, "redo restored wrong state");
    require(editor.undo(), "second undo was unavailable");
    editor.addPathNode({500.f, 80.f});
    require(!editor.redo(), "new edit did not invalidate redo history");
    require(editor.document().paths.front().nodes.back().x == 500.f, "branched edit was lost");
}

void testBuiltInMapsRemainValid() {
    const std::array<std::string, 3> names = {"verdant", "frost", "ember"};
    for (const auto& name : names) {
        std::string error;
        const auto path = std::filesystem::path(AEGIS_SOURCE_DIR) / "maps" / (name + ".aegismap");
        const auto document = aegis::core::MapDocument::load(path.string(), &error);
        require(document.has_value(), name + " failed to load: " + error);
        require(document->metadata.author == "AEGIS DOMINION", name + " still carries old product metadata");
        require(document->validate().empty(), name + " is invalid");
    }
}

} // namespace

int main() {
    try {
        testRoundTripPreservesDataAndUtf8();
        testInvalidDocumentsAreRejected();
        testMalformedAndUnknownFormatsFailCleanly();
        testEditorUndoRedoAndBranchingHistory();
        testBuiltInMapsRemainValid();
        std::cout << "5 MapDocument/MapEditor test groups passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }
}
