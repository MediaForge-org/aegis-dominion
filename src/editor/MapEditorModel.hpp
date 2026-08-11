#pragma once
#include "core/MapDocument.hpp"
#include <vector>
#include <string>

namespace aegis::editor {

enum class Tool {
    Select,
    Path,
    Spawn,
    Goal,
    BuildZone,
    BlockZone,
    Water,
    Decoration,
    Erase
};

struct EditorSnapshot {
    core::MapDocument document;
    std::string label;
};

class MapEditorModel {
public:
    MapEditorModel();

    const core::MapDocument& document() const { return document_; }
    core::MapDocument& document() { return document_; }

    Tool tool() const { return tool_; }
    void setTool(Tool value) { tool_ = value; }

    void newMap(const std::string& id, const std::string& name, float width = 1200.f, float height = 900.f);
    bool load(const std::string& file, std::string* error = nullptr);
    bool save(const std::string& file, std::string* error = nullptr) const;

    void addPathNode(core::Vec2 p, const std::string& pathId = "main");
    void setSpawn(core::Vec2 p, const std::string& pathId = "main");
    void setGoal(core::Vec2 p, const std::string& pathId = "main");
    void addZone(core::ZoneType type, core::Rect rect);
    void addDecoration(std::string assetId, core::Vec2 p, float rotation = 0.f, float scale = 1.f, int layer = 0);

    void checkpoint(const std::string& label);
    bool undo();
    bool redo();
    void clearHistory();

    std::vector<std::string> validate() const { return document_.validate(); }

private:
    core::Path& pathById(const std::string& id);

    core::MapDocument document_;
    Tool tool_ = Tool::Select;
    std::vector<EditorSnapshot> undo_;
    std::vector<EditorSnapshot> redo_;
    static constexpr std::size_t MaxHistory = 64;
};

} // namespace aegis::editor
