#pragma once

#include "core/MapDocument.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aegis::editor {

enum class Tool {
    Select,
    Path,
    Spawn,
    Goal,
    BuildZone,
    BlockZone,
    Water,
    Erase
};

enum class SelectionType { None, Path, PathPoint, Spawn, Goal, Zone };

struct EditorSelection {
    SelectionType type = SelectionType::None;
    std::string id;
    std::size_t index = 0;
    std::size_t subIndex = 0;

    bool empty() const { return type == SelectionType::None; }
    void clear() { *this = {}; }
};

struct EditorSnapshot {
    core::MapDocument document;
    std::uint64_t revision = 0;
    std::string label;
};

class MapEditorModel {
public:
    MapEditorModel();

    const core::MapDocument& document() const { return document_; }
    core::MapDocument& editDocument(const std::string& label);

    Tool tool() const { return tool_; }
    void setTool(Tool value) { tool_ = value; }

    void newMap(const std::string& id, const std::string& name, float width = 1200.f, float height = 900.f);
    bool load(const std::string& file, std::string* error = nullptr);
    bool save(const std::string& file, std::string* error = nullptr);

    const std::string& currentFile() const { return currentFile_; }
    bool dirty() const { return revision_ != savedRevision_; }
    bool canUndo() const { return !undo_.empty(); }
    bool canRedo() const { return !redo_.empty(); }

    std::string createPath();
    bool deletePath(const std::string& pathId);
    void addPathNode(core::Vec2 point, const std::string& pathId = "main");
    bool insertPathNode(const std::string& pathId, std::size_t index, core::Vec2 point);
    bool movePathNode(const std::string& pathId, std::size_t index, core::Vec2 point);
    bool removePathNode(const std::string& pathId, std::size_t index);

    std::string addSpawn(core::Vec2 point, const std::string& pathId = "main");
    void setSpawn(core::Vec2 point, const std::string& pathId = "main") { addSpawn(point, pathId); }
    bool moveSpawn(const std::string& id, core::Vec2 point);
    bool removeSpawn(const std::string& id);

    std::string addGoal(core::Vec2 point, const std::string& pathId = "main");
    void setGoal(core::Vec2 point, const std::string& pathId = "main") { addGoal(point, pathId); }
    bool moveGoal(const std::string& id, core::Vec2 point);
    bool removeGoal(const std::string& id);

    std::size_t addZone(core::ZoneType type, core::Rect rect);
    bool moveZone(std::size_t index, core::Vec2 position);
    bool resizeZone(std::size_t index, core::Vec2 size);
    bool setZoneType(std::size_t index, core::ZoneType type);
    bool removeZone(std::size_t index);

    void addDecoration(std::string assetId, core::Vec2 point, float rotation = 0.f, float scale = 1.f, int layer = 0);

    bool undo();
    bool redo();
    void clearHistory();

    std::vector<core::ValidationIssue> validateDetailed() const { return document_.validateDetailed(); }
    std::vector<std::string> validate() const { return document_.validate(); }

private:
    core::Path* findPath(const std::string& id);
    const core::Path* findPath(const std::string& id) const;
    core::Path& ensurePath(const std::string& id);
    std::string nextId(const std::string& prefix) const;
    void checkpoint(const std::string& label);
    void changed();

    core::MapDocument document_;
    Tool tool_ = Tool::Select;
    std::string currentFile_;
    std::vector<EditorSnapshot> undo_;
    std::vector<EditorSnapshot> redo_;
    std::uint64_t revision_ = 0;
    std::uint64_t savedRevision_ = 0;
    std::uint64_t nextRevision_ = 1;
    static constexpr std::size_t MaxHistory = 128;
};

const char* toolName(Tool tool);

} // namespace aegis::editor
