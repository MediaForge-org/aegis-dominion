#include "MapEditorModel.hpp"

#include <algorithm>
#include <utility>

namespace aegis::editor {

namespace {
template <typename Container, typename Predicate>
bool eraseFirst(Container& container, Predicate predicate) {
    const auto found = std::find_if(container.begin(), container.end(), predicate);
    if (found == container.end()) return false;
    container.erase(found);
    return true;
}
}

const char* toolName(Tool tool) {
    switch (tool) {
        case Tool::Select: return "SELECT";
        case Tool::Path: return "PATH";
        case Tool::Spawn: return "SPAWN";
        case Tool::Goal: return "GOAL";
        case Tool::BuildZone: return "BUILD ZONE";
        case Tool::BlockZone: return "BLOCKED ZONE";
        case Tool::Water: return "WATER";
        case Tool::Erase: return "ERASER";
    }
    return "SELECT";
}

MapEditorModel::MapEditorModel() { newMap("untitled", "Neue Karte"); }

void MapEditorModel::newMap(const std::string& id, const std::string& name, float width, float height) {
    document_ = {};
    document_.metadata.id = id;
    document_.metadata.name = name;
    document_.metadata.subtitle = "Benutzerkarte";
    document_.metadata.description = "Mit AEGIS DOMINION MAP FORGE erstellt.";
    document_.width = width;
    document_.height = height;
    document_.paths.push_back({"main", {}});
    currentFile_.clear();
    revision_ = savedRevision_ = 0;
    nextRevision_ = 1;
    clearHistory();
}

bool MapEditorModel::load(const std::string& file, std::string* error) {
    auto loaded = core::MapDocument::load(file, error);
    if (!loaded) return false;
    document_ = std::move(*loaded);
    currentFile_ = file;
    revision_ = savedRevision_ = 0;
    nextRevision_ = 1;
    clearHistory();
    return true;
}

bool MapEditorModel::save(const std::string& file, std::string* error) {
    if (!document_.save(file, error)) return false;
    currentFile_ = file;
    savedRevision_ = revision_;
    return true;
}

core::MapDocument& MapEditorModel::editDocument(const std::string& label) {
    checkpoint(label);
    changed();
    return document_;
}

core::Path* MapEditorModel::findPath(const std::string& id) {
    const auto it = std::find_if(document_.paths.begin(), document_.paths.end(), [&](const core::Path& path) { return path.id == id; });
    return it == document_.paths.end() ? nullptr : &*it;
}

const core::Path* MapEditorModel::findPath(const std::string& id) const {
    const auto it = std::find_if(document_.paths.begin(), document_.paths.end(), [&](const core::Path& path) { return path.id == id; });
    return it == document_.paths.end() ? nullptr : &*it;
}

core::Path& MapEditorModel::ensurePath(const std::string& id) {
    if (auto* path = findPath(id)) return *path;
    document_.paths.push_back({id, {}});
    return document_.paths.back();
}

std::string MapEditorModel::nextId(const std::string& prefix) const {
    for (std::size_t suffix = 0;; ++suffix) {
        const auto candidate = prefix + std::to_string(suffix);
        const auto usedByPath = std::any_of(document_.paths.begin(), document_.paths.end(), [&](const core::Path& path) { return path.id == candidate; });
        const auto usedBySpawn = std::any_of(document_.spawns.begin(), document_.spawns.end(), [&](const core::SpawnPoint& point) { return point.id == candidate; });
        const auto usedByGoal = std::any_of(document_.goals.begin(), document_.goals.end(), [&](const core::GoalPoint& point) { return point.id == candidate; });
        if (!usedByPath && !usedBySpawn && !usedByGoal) return candidate;
    }
}

void MapEditorModel::checkpoint(const std::string& label) {
    undo_.push_back({document_, revision_, label});
    if (undo_.size() > MaxHistory) undo_.erase(undo_.begin());
    redo_.clear();
}

void MapEditorModel::changed() { revision_ = nextRevision_++; }

std::string MapEditorModel::createPath() {
    checkpoint("Pfad erstellen");
    const auto id = nextId("path_");
    document_.paths.push_back({id, {}});
    changed();
    return id;
}

bool MapEditorModel::deletePath(const std::string& pathId) {
    if (!findPath(pathId)) return false;
    checkpoint("Pfad löschen");
    eraseFirst(document_.paths, [&](const core::Path& path) { return path.id == pathId; });
    changed();
    return true;
}

void MapEditorModel::addPathNode(core::Vec2 point, const std::string& pathId) {
    checkpoint("Pfadpunkt hinzufügen");
    ensurePath(pathId).nodes.push_back(point);
    changed();
}

bool MapEditorModel::insertPathNode(const std::string& pathId, std::size_t index, core::Vec2 point) {
    auto* path = findPath(pathId);
    if (!path || index > path->nodes.size()) return false;
    checkpoint("Pfadpunkt einfügen");
    path = findPath(pathId);
    path->nodes.insert(path->nodes.begin() + static_cast<std::ptrdiff_t>(index), point);
    changed();
    return true;
}

bool MapEditorModel::movePathNode(const std::string& pathId, std::size_t index, core::Vec2 point) {
    auto* path = findPath(pathId);
    if (!path || index >= path->nodes.size()) return false;
    checkpoint("Pfadpunkt verschieben");
    findPath(pathId)->nodes[index] = point;
    changed();
    return true;
}

bool MapEditorModel::removePathNode(const std::string& pathId, std::size_t index) {
    auto* path = findPath(pathId);
    if (!path || index >= path->nodes.size()) return false;
    checkpoint("Pfadpunkt löschen");
    path = findPath(pathId);
    path->nodes.erase(path->nodes.begin() + static_cast<std::ptrdiff_t>(index));
    changed();
    return true;
}

std::string MapEditorModel::addSpawn(core::Vec2 point, const std::string& pathId) {
    checkpoint("Spawn erstellen");
    const auto id = nextId("spawn_");
    document_.spawns.push_back({id, point, pathId});
    changed();
    return id;
}

bool MapEditorModel::moveSpawn(const std::string& id, core::Vec2 point) {
    const auto found = std::find_if(document_.spawns.begin(), document_.spawns.end(), [&](const core::SpawnPoint& spawn) { return spawn.id == id; });
    if (found == document_.spawns.end()) return false;
    checkpoint("Spawn verschieben");
    std::find_if(document_.spawns.begin(), document_.spawns.end(), [&](const core::SpawnPoint& spawn) { return spawn.id == id; })->position = point;
    changed();
    return true;
}

bool MapEditorModel::removeSpawn(const std::string& id) {
    if (std::none_of(document_.spawns.begin(), document_.spawns.end(), [&](const core::SpawnPoint& spawn) { return spawn.id == id; })) return false;
    checkpoint("Spawn löschen");
    eraseFirst(document_.spawns, [&](const core::SpawnPoint& spawn) { return spawn.id == id; });
    changed();
    return true;
}

std::string MapEditorModel::addGoal(core::Vec2 point, const std::string& pathId) {
    checkpoint("Ziel erstellen");
    const auto id = nextId("goal_");
    document_.goals.push_back({id, point, pathId});
    changed();
    return id;
}

bool MapEditorModel::moveGoal(const std::string& id, core::Vec2 point) {
    const auto found = std::find_if(document_.goals.begin(), document_.goals.end(), [&](const core::GoalPoint& goal) { return goal.id == id; });
    if (found == document_.goals.end()) return false;
    checkpoint("Ziel verschieben");
    std::find_if(document_.goals.begin(), document_.goals.end(), [&](const core::GoalPoint& goal) { return goal.id == id; })->position = point;
    changed();
    return true;
}

bool MapEditorModel::removeGoal(const std::string& id) {
    if (std::none_of(document_.goals.begin(), document_.goals.end(), [&](const core::GoalPoint& goal) { return goal.id == id; })) return false;
    checkpoint("Ziel löschen");
    eraseFirst(document_.goals, [&](const core::GoalPoint& goal) { return goal.id == id; });
    changed();
    return true;
}

std::size_t MapEditorModel::addZone(core::ZoneType type, core::Rect rect) {
    checkpoint("Zone erstellen");
    document_.zones.push_back({type, rect});
    changed();
    return document_.zones.size() - 1;
}

bool MapEditorModel::moveZone(std::size_t index, core::Vec2 position) {
    if (index >= document_.zones.size()) return false;
    checkpoint("Zone verschieben");
    document_.zones[index].rect.x = position.x;
    document_.zones[index].rect.y = position.y;
    changed();
    return true;
}

bool MapEditorModel::resizeZone(std::size_t index, core::Vec2 size) {
    if (index >= document_.zones.size()) return false;
    checkpoint("Zone skalieren");
    document_.zones[index].rect.w = size.x;
    document_.zones[index].rect.h = size.y;
    changed();
    return true;
}

bool MapEditorModel::setZoneType(std::size_t index, core::ZoneType type) {
    if (index >= document_.zones.size()) return false;
    checkpoint("Zonentyp ändern");
    document_.zones[index].type = type;
    changed();
    return true;
}

bool MapEditorModel::removeZone(std::size_t index) {
    if (index >= document_.zones.size()) return false;
    checkpoint("Zone löschen");
    document_.zones.erase(document_.zones.begin() + static_cast<std::ptrdiff_t>(index));
    changed();
    return true;
}

void MapEditorModel::addDecoration(std::string assetId, core::Vec2 point, float rotation, float scale, int layer) {
    checkpoint("Dekoration hinzufügen");
    document_.decorations.push_back({std::move(assetId), point, rotation, scale, layer});
    changed();
}

bool MapEditorModel::undo() {
    if (undo_.empty()) return false;
    redo_.push_back({document_, revision_, "Wiederholen"});
    document_ = std::move(undo_.back().document);
    revision_ = undo_.back().revision;
    undo_.pop_back();
    return true;
}

bool MapEditorModel::redo() {
    if (redo_.empty()) return false;
    undo_.push_back({document_, revision_, "Rückgängig"});
    document_ = std::move(redo_.back().document);
    revision_ = redo_.back().revision;
    redo_.pop_back();
    return true;
}

void MapEditorModel::clearHistory() {
    undo_.clear();
    redo_.clear();
}

} // namespace aegis::editor
