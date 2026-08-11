#include "editor/MapEditorModel.hpp"

#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

void pathCreateUndoRedo() {
    aegis::editor::MapEditorModel editor;
    const auto id = editor.createPath();
    require(editor.document().paths.size() == 2 && editor.document().paths.back().id == id, "path creation failed");
    require(editor.undo() && editor.document().paths.size() == 1, "path create undo failed");
    require(editor.redo() && editor.document().paths.size() == 2, "path create redo failed");
}

void pathPointAddInsertRemoveMove() {
    aegis::editor::MapEditorModel editor;
    editor.addPathNode({0.f, 20.f}); editor.addPathNode({200.f, 20.f});
    require(editor.insertPathNode("main", 1, {100.f, 40.f}), "path insert failed");
    require(editor.movePathNode("main", 1, {110.f, 50.f}), "path move failed");
    require(editor.document().paths.front().nodes[1].x == 110.f, "moved point mismatch");
    require(editor.removePathNode("main", 1), "path remove failed");
    require(editor.document().paths.front().nodes.size() == 2, "path point count wrong");
}

void pathMoveUndoRedo() {
    aegis::editor::MapEditorModel editor;
    editor.addPathNode({0.f, 20.f});
    editor.movePathNode("main", 0, {40.f, 60.f});
    require(editor.undo() && editor.document().paths.front().nodes.front().x == 0.f, "move undo failed");
    require(editor.redo() && editor.document().paths.front().nodes.front().x == 40.f, "move redo failed");
}

void pathDeleteUndoRedo() {
    aegis::editor::MapEditorModel editor;
    editor.addPathNode({0.f, 20.f}); editor.addPathNode({100.f, 20.f});
    editor.removePathNode("main", 0);
    require(editor.undo() && editor.document().paths.front().nodes.size() == 2, "delete undo failed");
    require(editor.redo() && editor.document().paths.front().nodes.size() == 1, "delete redo failed");
}

void spawnCrudAndStableIds() {
    aegis::editor::MapEditorModel editor;
    const auto first = editor.addSpawn({10.f, 20.f});
    const auto second = editor.addSpawn({30.f, 40.f});
    require(first != second && editor.document().spawns.size() == 2, "multiple spawn IDs failed");
    editor.moveSpawn(first, {50.f, 60.f}); require(editor.document().spawns.front().position.x == 50.f, "spawn move failed");
    editor.removeSpawn(first); require(editor.undo() && editor.document().spawns.size() == 2, "spawn delete undo failed");
}

void goalCrudAndStableIds() {
    aegis::editor::MapEditorModel editor;
    const auto first = editor.addGoal({10.f, 20.f});
    const auto second = editor.addGoal({30.f, 40.f});
    require(first != second && editor.document().goals.size() == 2, "multiple goal IDs failed");
    editor.moveGoal(second, {70.f, 80.f}); require(editor.document().goals.back().position.y == 80.f, "goal move failed");
    editor.removeGoal(second); require(editor.undo() && editor.document().goals.size() == 2, "goal delete undo failed");
}

void zoneCrudUndoRedo() {
    aegis::editor::MapEditorModel editor;
    const auto index = editor.addZone(aegis::core::ZoneType::Water, {10.f, 20.f, 100.f, 80.f});
    editor.moveZone(index, {40.f, 50.f}); editor.resizeZone(index, {120.f, 90.f});
    require(editor.document().zones[index].rect.x == 40.f && editor.document().zones[index].rect.w == 120.f, "zone edit failed");
    editor.removeZone(index); require(editor.undo() && editor.document().zones.size() == 1, "zone delete undo failed");
    require(editor.redo() && editor.document().zones.empty(), "zone delete redo failed");
}

void branchedHistoryDropsRedo() {
    aegis::editor::MapEditorModel editor;
    editor.addPathNode({0.f, 50.f}); editor.addPathNode({300.f, 50.f});
    require(editor.undo(), "undo unavailable");
    editor.addPathNode({500.f, 80.f});
    require(!editor.redo(), "branched edit retained redo");
    require(editor.document().paths.front().nodes.back().x == 500.f, "branched edit lost");
}

void dirtyTracksSavedRevision() {
    aegis::editor::MapEditorModel editor;
    require(!editor.dirty(), "new map started dirty");
    editor.addPathNode({0.f, 10.f}); require(editor.dirty(), "edit did not mark dirty");
    const auto path = std::filesystem::temp_directory_path() / "aegis_editor_dirty.aegismap";
    std::string error; require(editor.save(path.string(), &error), error); require(!editor.dirty(), "save did not clear dirty");
    editor.addPathNode({10.f, 10.f}); require(editor.dirty(), "second edit not dirty");
    require(editor.undo() && !editor.dirty(), "undo to saved revision stayed dirty");
    std::filesystem::remove(path);
}

void metadataEditParticipatesInHistory() {
    aegis::editor::MapEditorModel editor;
    editor.editDocument("Name ändern").metadata.name = "Geändert";
    require(editor.undo() && editor.document().metadata.name == "Neue Karte", "metadata undo failed");
    require(editor.redo() && editor.document().metadata.name == "Geändert", "metadata redo failed");
}
}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"path create", pathCreateUndoRedo}, {"path point operations", pathPointAddInsertRemoveMove},
        {"path move history", pathMoveUndoRedo}, {"path delete history", pathDeleteUndoRedo},
        {"spawn CRUD", spawnCrudAndStableIds}, {"goal CRUD", goalCrudAndStableIds},
        {"zone CRUD", zoneCrudUndoRedo}, {"branched history", branchedHistoryDropsRedo},
        {"dirty revision", dirtyTracksSavedRevision}, {"metadata history", metadataEditParticipatesInHistory}
    };
    try { for (const auto& test : tests) test.second(); std::cout << tests.size() << " editor test cases passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << "TEST FAILURE: " << error.what() << '\n'; return 1; }
}
