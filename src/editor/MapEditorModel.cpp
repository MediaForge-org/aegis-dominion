#include "MapEditorModel.hpp"
#include <algorithm>
#include <utility>

namespace aegis::editor {

MapEditorModel::MapEditorModel() {
    newMap("untitled", "Neue Karte");
}

void MapEditorModel::newMap(const std::string& id, const std::string& name, float width, float height) {
    document_ = {};
    document_.metadata.id = id;
    document_.metadata.name = name;
    document_.metadata.subtitle = "Benutzerkarte";
    document_.metadata.description = "Mit AEGIS DOMINION MAP FORGE erstellt.";
    document_.width = width;
    document_.height = height;
    document_.paths.push_back({"main", {}});
    clearHistory();
}

bool MapEditorModel::load(const std::string& file, std::string* error) {
    auto loaded = core::MapDocument::load(file, error);
    if (!loaded) return false;
    document_ = std::move(*loaded);
    clearHistory();
    return true;
}

bool MapEditorModel::save(const std::string& file, std::string* error) const {
    return document_.save(file, error);
}

core::Path& MapEditorModel::pathById(const std::string& id) {
    auto it = std::find_if(document_.paths.begin(), document_.paths.end(), [&](const core::Path& p) { return p.id == id; });
    if (it != document_.paths.end()) return *it;
    document_.paths.push_back({id, {}});
    return document_.paths.back();
}

void MapEditorModel::addPathNode(core::Vec2 p, const std::string& pathId) {
    checkpoint("Pfadpunkt hinzufügen");
    pathById(pathId).nodes.push_back(p);
}

void MapEditorModel::setSpawn(core::Vec2 p, const std::string& pathId) {
    checkpoint("Spawn setzen");
    auto it = std::find_if(document_.spawns.begin(), document_.spawns.end(), [&](const core::SpawnPoint& s) { return s.pathId == pathId; });
    if (it == document_.spawns.end()) document_.spawns.push_back({"spawn_" + std::to_string(document_.spawns.size()), p, pathId});
    else it->position = p;
}

void MapEditorModel::setGoal(core::Vec2 p, const std::string& pathId) {
    checkpoint("Ziel setzen");
    auto it = std::find_if(document_.goals.begin(), document_.goals.end(), [&](const core::GoalPoint& g) { return g.pathId == pathId; });
    if (it == document_.goals.end()) document_.goals.push_back({"goal_" + std::to_string(document_.goals.size()), p, pathId});
    else it->position = p;
}

void MapEditorModel::addZone(core::ZoneType type, core::Rect rect) {
    checkpoint("Zone hinzufügen");
    document_.zones.push_back({type, rect});
}

void MapEditorModel::addDecoration(std::string assetId, core::Vec2 p, float rotation, float scale, int layer) {
    checkpoint("Dekoration hinzufügen");
    document_.decorations.push_back({std::move(assetId), p, rotation, scale, layer});
}

void MapEditorModel::checkpoint(const std::string& label) {
    undo_.push_back({document_, label});
    if (undo_.size() > MaxHistory) undo_.erase(undo_.begin());
    redo_.clear();
}

bool MapEditorModel::undo() {
    if (undo_.empty()) return false;
    redo_.push_back({document_, "redo"});
    document_ = std::move(undo_.back().document);
    undo_.pop_back();
    return true;
}

bool MapEditorModel::redo() {
    if (redo_.empty()) return false;
    undo_.push_back({document_, "undo"});
    document_ = std::move(redo_.back().document);
    redo_.pop_back();
    return true;
}

void MapEditorModel::clearHistory() {
    undo_.clear();
    redo_.clear();
}

} // namespace aegis::editor
