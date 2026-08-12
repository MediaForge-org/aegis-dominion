#include "MapForgeScreen.hpp"

#include "Common.hpp"
#include "app/ScreenManager.hpp"
#include "core/PlayableMap.hpp"
#include "render/RenderContext.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace aegis::editor {
namespace {
const sf::FloatRect Toolbar(0.f, 0.f, 1600.f, 66.f);
const sf::FloatRect ToolPanel(0.f, 66.f, 180.f, 784.f);
const sf::FloatRect Canvas(180.f, 66.f, 1040.f, 784.f);
const sf::FloatRect Inspector(1220.f, 66.f, 380.f, 784.f);
const sf::FloatRect StatusBar(0.f, 850.f, 1600.f, 50.f);
const std::array<Tool, 8> Tools = {Tool::Select, Tool::Path, Tool::Spawn, Tool::Goal, Tool::BuildZone, Tool::BlockZone, Tool::Water, Tool::Erase};

sf::FloatRect toolRect(std::size_t index) { return {12.f, 106.f + static_cast<float>(index) * 58.f, 156.f, 46.f}; }
float squaredDistance(sf::Vector2f a, sf::Vector2f b) { const auto d = a - b; return d.x * d.x + d.y * d.y; }

float distanceToSegment(sf::Vector2f point, sf::Vector2f a, sf::Vector2f b) {
    const auto ab = b - a;
    const float denominator = ab.x * ab.x + ab.y * ab.y;
    if (denominator < .001f) return std::sqrt(squaredDistance(point, a));
    const auto ap = point - a;
    const float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / denominator, 0.f, 1.f);
    return std::sqrt(squaredDistance(point, a + ab * t));
}

core::ZoneType toolZoneType(Tool tool) {
    if (tool == Tool::BuildZone) return core::ZoneType::Buildable;
    if (tool == Tool::Water) return core::ZoneType::Water;
    return core::ZoneType::Blocked;
}

sf::Color zoneColor(core::ZoneType type) {
    switch (type) {
        case core::ZoneType::Buildable: return sf::Color(72, 220, 139);
        case core::ZoneType::Blocked: return sf::Color(255, 92, 107);
        case core::ZoneType::Water: return sf::Color(74, 165, 255);
        case core::ZoneType::DecorationOnly: return sf::Color(203, 139, 255);
    }
    return ui::Muted;
}

}

MapForgeScreen::MapForgeScreen(app::ScreenContext context) : Screen(context), ui_(context.window, context.assets) {
    context_.input.setContext(input::Context::MapForge);
    camera_.setViewport(Canvas);
    camera_.fit(model_.document().width, model_.document().height);
    refreshMapFiles();
}

void MapForgeScreen::onResume() { context_.input.setContext(input::Context::MapForge); setStatus("Playtest beendet – Editorzustand wiederhergestellt"); }

void MapForgeScreen::handleEvent(const sf::Event& event) {
    context_.input.setContext(dialog_ == Dialog::None ? input::Context::MapForge : input::Context::Modal);
    if (event.type == sf::Event::Closed) { confirmOrRun(PendingAction::Quit); return; }
    if (event.type == sf::Event::KeyPressed) { handleKey(event.key); return; }
    if (event.type == sf::Event::TextEntered) { handleText(event.text.unicode); return; }
    if (event.type == sf::Event::MouseWheelScrolled) {
        const auto screen = context_.window.mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        if (dialog_ == Dialog::Open) {
            if (event.mouseWheelScroll.delta > 0.f && mapScroll_ > 0) --mapScroll_;
            else if (event.mouseWheelScroll.delta < 0.f && mapScroll_ + 8 < mapFiles_.size()) ++mapScroll_;
        } else if (dialog_ == Dialog::None && Canvas.contains(screen)) camera_.zoomAt(event.mouseWheelScroll.delta, screen);
        return;
    }
    if (event.type == sf::Event::MouseButtonPressed)
        mousePressed(context_.window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}), event.mouseButton.button);
    else if (event.type == sf::Event::MouseButtonReleased)
        mouseReleased(context_.window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}), event.mouseButton.button);
    else if (event.type == sf::Event::MouseMoved)
        mouseMoved(context_.window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y}));
}

void MapForgeScreen::update(float deltaSeconds) {
    if (statusTimer_ > 0.f) statusTimer_ -= deltaSeconds;
}

void MapForgeScreen::render() {
    render::RenderContext renderContext(context_.window);
    render::WorldRenderer renderer(renderContext);
    renderer.begin(render::Layer::Terrain);
    drawCanvas();
    renderer.begin(render::Layer::ScreenUi);
    drawToolbar();
    drawTools();
    drawInspector();
    drawStatusBar();
    if (dialog_ != Dialog::None) { renderer.begin(render::Layer::ModalUi); drawDialog(); }
}

void MapForgeScreen::handleKey(const sf::Event::KeyEvent& key) {
    if (dialog_ != Dialog::None) {
        if (key.code == sf::Keyboard::Escape) { dialog_ = Dialog::None; pendingAction_ = PendingAction::None; }
        else if (key.code == sf::Keyboard::Enter && dialog_ == Dialog::SaveAs) {
            if (saveAs(filenameInput_)) { dialog_ = Dialog::None; performPendingAction(); }
        }
        return;
    }
    if (textField_ != TextField::None) {
        if (key.code == sf::Keyboard::Enter) commitTextField();
        else if (key.code == sf::Keyboard::Escape) { textField_ = TextField::None; textBuffer_.clear(); }
        return;
    }
    sf::Event event{}; event.type = sf::Event::KeyPressed; event.key = key;
    if (context_.input.triggered(input::Action::EditorUndo, event)) { if (model_.undo()) { selection_.clear(); setStatus("Rückgängig"); } return; }
    if (context_.input.triggered(input::Action::EditorRedo, event)) { if (model_.redo()) { selection_.clear(); setStatus("Wiederholt"); } return; }
    if (context_.input.triggered(input::Action::EditorSaveAs, event)) { dialog_ = Dialog::SaveAs; return; }
    if (context_.input.triggered(input::Action::EditorSave, event)) { save(); return; }
    if (context_.input.triggered(input::Action::EditorOpen, event)) { dialog_ = Dialog::Open; refreshMapFiles(); return; }
    if (context_.input.triggered(input::Action::EditorNew, event)) { requestNew(); return; }
    if (key.code == sf::Keyboard::Escape) { if (!selection_.empty()) selection_.clear(); else requestExit(); return; }
    if (key.code == sf::Keyboard::Delete || key.code == sf::Keyboard::BackSpace) { deleteSelected(); return; }
    if (context_.input.triggered(input::Action::EditorGrid, event)) { gridVisible_ = !gridVisible_; return; }
    if (context_.input.triggered(input::Action::EditorSnap, event)) { snapEnabled_ = !snapEnabled_; return; }
    if (context_.input.triggered(input::Action::EditorFit, event)) { camera_.fit(model_.document().width, model_.document().height); return; }
    const input::Action toolActions[] = {input::Action::EditorSelect, input::Action::EditorPath, input::Action::EditorSpawn, input::Action::EditorGoal,
                                         input::Action::EditorBuildZone, input::Action::EditorBlockedZone, input::Action::EditorWater, input::Action::EditorErase};
    for (std::size_t i = 0; i < Tools.size(); ++i) if (context_.input.triggered(toolActions[i], event)) { model_.setTool(Tools[i]); return; }

    core::Vec2 target{};
    bool canNudge = true;
    const auto& document = model_.document();
    if (selection_.type == SelectionType::PathPoint) {
        const auto path = std::find_if(document.paths.begin(), document.paths.end(), [&](const core::Path& value) { return value.id == selection_.id; });
        if (path == document.paths.end() || selection_.subIndex >= path->nodes.size()) canNudge = false;
        else target = path->nodes[selection_.subIndex];
    } else if (selection_.type == SelectionType::Spawn) {
        const auto point = std::find_if(document.spawns.begin(), document.spawns.end(), [&](const core::SpawnPoint& value) { return value.id == selection_.id; });
        if (point == document.spawns.end()) canNudge = false; else target = point->position;
    } else if (selection_.type == SelectionType::Goal) {
        const auto point = std::find_if(document.goals.begin(), document.goals.end(), [&](const core::GoalPoint& value) { return value.id == selection_.id; });
        if (point == document.goals.end()) canNudge = false; else target = point->position;
    } else if (selection_.type == SelectionType::Zone && selection_.index < document.zones.size()) {
        target = {document.zones[selection_.index].rect.x, document.zones[selection_.index].rect.y};
    } else canNudge = false;
    const float step = snapEnabled_ ? gridSize_ : 1.f;
    if (key.code == sf::Keyboard::Left) target.x -= step;
    else if (key.code == sf::Keyboard::Right) target.x += step;
    else if (key.code == sf::Keyboard::Up) target.y -= step;
    else if (key.code == sf::Keyboard::Down) target.y += step;
    else canNudge = false;
    if (canNudge) commitDrag(clamped(target));
}

void MapForgeScreen::handleText(sf::Uint32 unicode) {
    std::string* target = nullptr;
    if (dialog_ == Dialog::SaveAs) target = &filenameInput_;
    else if (textField_ != TextField::None) target = &textBuffer_;
    if (!target) return;
    if (unicode == 8) {
        if (!target->empty()) {
            std::size_t start = target->size() - 1;
            while (start > 0 && (static_cast<unsigned char>((*target)[start]) & 0xC0u) == 0x80u) --start;
            target->erase(start);
        }
    } else if (unicode >= 32 && unicode != 127) {
        const auto encoded = sf::String(unicode).toUtf8();
        target->append(encoded.begin(), encoded.end());
    }
}

void MapForgeScreen::mousePressed(sf::Vector2f screen, sf::Mouse::Button button) {
    lastMouseScreen_ = screen;
    mouseWorld_ = camera_.screenToWorld(screen);
    if (dialog_ != Dialog::None) { context_.input.capturePointer(true); context_.input.consumePointerPress(button); if (button == sf::Mouse::Left) dialogPressed(screen); return; }
    if (button == sf::Mouse::Middle || (button == sf::Mouse::Right && Canvas.contains(screen))) { panning_ = true; return; }
    if (button != sf::Mouse::Left) return;
    if (Toolbar.contains(screen)) { context_.input.consumePointerPress(button); toolbarPressed(screen); }
    else if (ToolPanel.contains(screen)) { context_.input.consumePointerPress(button); toolsPressed(screen); }
    else if (Inspector.contains(screen)) { context_.input.consumePointerPress(button); inspectorPressed(screen); }
    else if (Canvas.contains(screen)) canvasPressed(screen);
}

void MapForgeScreen::mouseReleased(sf::Vector2f screen, sf::Mouse::Button button) {
    if (dialog_ == Dialog::None) context_.input.capturePointer(false);
    if (button == sf::Mouse::Middle || button == sf::Mouse::Right) panning_ = false;
    if (button != sf::Mouse::Left) return;
    const auto world = clamped(snapped(camera_.screenToWorld(screen)));
    if (creatingZone_) {
        creatingZone_ = false;
        const float x = std::min(zoneStart_.x, world.x);
        const float y = std::min(zoneStart_.y, world.y);
        const float width = std::abs(zoneStart_.x - world.x);
        const float height = std::abs(zoneStart_.y - world.y);
        if (width >= 4.f && height >= 4.f) {
            const auto index = model_.addZone(toolZoneType(model_.tool()), {x, y, width, height});
            selection_ = {SelectionType::Zone, {}, index, 0};
            setStatus("Zone erstellt");
        }
    } else if (dragging_) {
        dragging_ = false;
        auto target = world;
        if (selection_.type == SelectionType::Zone && selection_.index < model_.document().zones.size()) {
            const auto& rect = model_.document().zones[selection_.index].rect;
            target = {rect.x + world.x - dragStartWorld_.x, rect.y + world.y - dragStartWorld_.y};
        }
        commitDrag(target);
    }
}

void MapForgeScreen::mouseMoved(sf::Vector2f screen) {
    if (panning_) camera_.panScreen(screen - lastMouseScreen_);
    lastMouseScreen_ = screen;
    mouseWorld_ = camera_.screenToWorld(screen);
    dragCurrentWorld_ = clamped(snapped(mouseWorld_));
}

void MapForgeScreen::toolbarPressed(sf::Vector2f screen) {
    if (sf::FloatRect(8.f, 10.f, 104.f, 44.f).contains(screen)) requestExit();
    else if (sf::FloatRect(120.f, 10.f, 68.f, 44.f).contains(screen)) requestNew();
    else if (sf::FloatRect(194.f, 10.f, 78.f, 44.f).contains(screen)) { dialog_ = Dialog::Open; refreshMapFiles(); }
    else if (sf::FloatRect(278.f, 10.f, 86.f, 44.f).contains(screen)) save();
    else if (sf::FloatRect(370.f, 10.f, 108.f, 44.f).contains(screen)) dialog_ = Dialog::SaveAs;
    else if (sf::FloatRect(486.f, 10.f, 54.f, 44.f).contains(screen)) { model_.undo(); selection_.clear(); }
    else if (sf::FloatRect(546.f, 10.f, 54.f, 44.f).contains(screen)) { model_.redo(); selection_.clear(); }
    else if (sf::FloatRect(608.f, 10.f, 70.f, 44.f).contains(screen)) gridVisible_ = !gridVisible_;
    else if (sf::FloatRect(684.f, 10.f, 70.f, 44.f).contains(screen)) snapEnabled_ = !snapEnabled_;
    else if (sf::FloatRect(760.f, 10.f, 44.f, 44.f).contains(screen)) gridSize_ = std::max(8.f, gridSize_ / 2.f);
    else if (sf::FloatRect(810.f, 10.f, 44.f, 44.f).contains(screen)) gridSize_ = std::min(128.f, gridSize_ * 2.f);
    else if (sf::FloatRect(862.f, 10.f, 72.f, 44.f).contains(screen)) camera_.fit(model_.document().width, model_.document().height);
    else if (sf::FloatRect(944.f, 10.f, 148.f, 44.f).contains(screen)) {
        std::string error;
        auto launch = core::makeMapForgePlaytestLaunch(model_.document(), &error);
        if (launch) {
            app::ScreenRequest request{app::ScreenType::Game};
            request.gameLaunch = std::move(*launch);
            context_.screens.push(std::move(request));
        } else setStatus("Playtest blockiert: " + error.substr(0, error.find('\n')));
    }
}

void MapForgeScreen::toolsPressed(sf::Vector2f screen) {
    for (std::size_t i = 0; i < Tools.size(); ++i) {
        if (!toolRect(i).contains(screen)) continue;
        model_.setTool(Tools[i]);
        selection_.clear();
        setStatus(std::string("Werkzeug: ") + toolName(Tools[i]));
        return;
    }
}

void MapForgeScreen::canvasPressed(sf::Vector2f screen) {
    const auto world = clamped(snapped(camera_.screenToWorld(screen)));
    const auto hit = hitTest(screen);
    if (model_.tool() == Tool::Erase) { if (!hit.empty()) eraseSelection(hit); return; }
    if (model_.tool() == Tool::Select) {
        selection_ = hit;
        if (!selection_.empty()) { dragging_ = true; dragStartWorld_ = world; dragCurrentWorld_ = world; }
        return;
    }
    if (model_.tool() == Tool::Path) {
        if (hit.type == SelectionType::PathPoint) {
            selection_ = hit;
            activePathId_ = hit.id;
            dragging_ = true;
            dragStartWorld_ = world;
            dragCurrentWorld_ = world;
            return;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
            if (const auto segment = segmentHit(screen)) {
                model_.insertPathNode(segment->first, segment->second + 1, world);
                activePathId_ = segment->first;
                selection_ = {SelectionType::PathPoint, activePathId_, 0, segment->second + 1};
                return;
            }
        }
        if (std::none_of(model_.document().paths.begin(), model_.document().paths.end(), [&](const core::Path& path) { return path.id == activePathId_; }))
            activePathId_ = model_.createPath();
        model_.addPathNode(world, activePathId_);
        const auto* path = &*std::find_if(model_.document().paths.begin(), model_.document().paths.end(), [&](const core::Path& value) { return value.id == activePathId_; });
        selection_ = {SelectionType::PathPoint, activePathId_, 0, path->nodes.size() - 1};
        return;
    }
    if (model_.tool() == Tool::Spawn) {
        const auto id = model_.addSpawn(world, selectedPathId());
        selection_ = {SelectionType::Spawn, id, 0, 0};
        return;
    }
    if (model_.tool() == Tool::Goal) {
        const auto id = model_.addGoal(world, selectedPathId());
        selection_ = {SelectionType::Goal, id, 0, 0};
        return;
    }
    creatingZone_ = true;
    zoneStart_ = world;
    dragCurrentWorld_ = world;
}

void MapForgeScreen::inspectorPressed(sf::Vector2f screen) {
    const auto& document = model_.document();
    const auto issues = model_.validateDetailed();
    for (std::size_t i = 0; i < std::min<std::size_t>(issues.size(), 7); ++i) {
        if (sf::FloatRect(1234.f, 646.f + static_cast<float>(i) * 27.f, 350.f, 24.f).contains(screen)) { focusIssue(issues[i]); return; }
    }
    if (selection_.empty()) {
        if (sf::FloatRect(1240.f, 145.f, 340.f, 38.f).contains(screen)) beginTextField(TextField::MapName, document.metadata.name);
        else if (sf::FloatRect(1240.f, 215.f, 340.f, 54.f).contains(screen)) beginTextField(TextField::Description, document.metadata.description);
        else if (sf::FloatRect(1240.f, 301.f, 340.f, 38.f).contains(screen)) beginTextField(TextField::Author, document.metadata.author);
        else if (sf::FloatRect(1240.f, 371.f, 340.f, 38.f).contains(screen)) beginTextField(TextField::Biome, document.metadata.biome);
        else if (sf::FloatRect(1450.f, 430.f, 54.f, 34.f).contains(screen)) model_.editDocument("Kartenbreite ändern").width = std::max(320.f, document.width - gridSize_);
        else if (sf::FloatRect(1510.f, 430.f, 54.f, 34.f).contains(screen)) model_.editDocument("Kartenbreite ändern").width = document.width + gridSize_;
        else if (sf::FloatRect(1450.f, 472.f, 54.f, 34.f).contains(screen)) model_.editDocument("Kartenhöhe ändern").height = std::max(240.f, document.height - gridSize_);
        else if (sf::FloatRect(1510.f, 472.f, 54.f, 34.f).contains(screen)) model_.editDocument("Kartenhöhe ändern").height = document.height + gridSize_;
        return;
    }
    if (selection_.type == SelectionType::Path || selection_.type == SelectionType::PathPoint) {
        if (sf::FloatRect(1240.f, 350.f, 160.f, 40.f).contains(screen)) { activePathId_ = model_.createPath(); selection_ = {SelectionType::Path, activePathId_, 0, 0}; }
        else if (sf::FloatRect(1410.f, 350.f, 170.f, 40.f).contains(screen)) { model_.deletePath(selection_.id); selection_.clear(); }
    } else if (selection_.type == SelectionType::Zone && selection_.index < document.zones.size()) {
        const auto rect = document.zones[selection_.index].rect;
        if (sf::FloatRect(1240.f, 455.f, 340.f, 38.f).contains(screen)) {
            auto next = static_cast<int>(document.zones[selection_.index].type) + 1;
            if (next > static_cast<int>(core::ZoneType::Water)) next = 0;
            model_.setZoneType(selection_.index, static_cast<core::ZoneType>(next));
        } else if (sf::FloatRect(1450.f, 340.f, 54.f, 32.f).contains(screen)) model_.resizeZone(selection_.index, {std::max(gridSize_, rect.w - gridSize_), rect.h});
        else if (sf::FloatRect(1510.f, 340.f, 54.f, 32.f).contains(screen)) model_.resizeZone(selection_.index, {rect.w + gridSize_, rect.h});
        else if (sf::FloatRect(1450.f, 388.f, 54.f, 32.f).contains(screen)) model_.resizeZone(selection_.index, {rect.w, std::max(gridSize_, rect.h - gridSize_)});
        else if (sf::FloatRect(1510.f, 388.f, 54.f, 32.f).contains(screen)) model_.resizeZone(selection_.index, {rect.w, rect.h + gridSize_});
    }
}

core::Vec2 MapForgeScreen::snapped(core::Vec2 point) const {
    if (!snapEnabled_) return point;
    return {std::round(point.x / gridSize_) * gridSize_, std::round(point.y / gridSize_) * gridSize_};
}

core::Vec2 MapForgeScreen::clamped(core::Vec2 point) const {
    return {std::clamp(point.x, 0.f, model_.document().width), std::clamp(point.y, 0.f, model_.document().height)};
}

EditorSelection MapForgeScreen::hitTest(sf::Vector2f screen) const {
    const auto& document = model_.document();
    for (const auto& path : document.paths) {
        for (std::size_t i = 0; i < path.nodes.size(); ++i)
            if (squaredDistance(screen, camera_.worldToScreen(path.nodes[i])) <= 12.f * 12.f) return {SelectionType::PathPoint, path.id, 0, i};
    }
    for (const auto& spawn : document.spawns)
        if (squaredDistance(screen, camera_.worldToScreen(spawn.position)) <= 16.f * 16.f) return {SelectionType::Spawn, spawn.id, 0, 0};
    for (const auto& goal : document.goals)
        if (squaredDistance(screen, camera_.worldToScreen(goal.position)) <= 16.f * 16.f) return {SelectionType::Goal, goal.id, 0, 0};
    const auto world = camera_.screenToWorld(screen);
    for (std::size_t reverse = document.zones.size(); reverse > 0; --reverse) {
        const auto& rect = document.zones[reverse - 1].rect;
        if (world.x >= rect.x && world.y >= rect.y && world.x <= rect.x + rect.w && world.y <= rect.y + rect.h)
            return {SelectionType::Zone, {}, reverse - 1, 0};
    }
    if (const auto segment = segmentHit(screen)) return {SelectionType::Path, segment->first, 0, 0};
    return {};
}

std::optional<std::pair<std::string, std::size_t>> MapForgeScreen::segmentHit(sf::Vector2f screen) const {
    for (const auto& path : model_.document().paths) {
        for (std::size_t i = 0; i + 1 < path.nodes.size(); ++i) {
            if (distanceToSegment(screen, camera_.worldToScreen(path.nodes[i]), camera_.worldToScreen(path.nodes[i + 1])) <= 9.f)
                return std::make_pair(path.id, i);
        }
    }
    return std::nullopt;
}

void MapForgeScreen::eraseSelection(const EditorSelection& selection) {
    selection_ = selection;
    deleteSelected();
}

void MapForgeScreen::deleteSelected() {
    bool removed = false;
    if (selection_.type == SelectionType::PathPoint) removed = model_.removePathNode(selection_.id, selection_.subIndex);
    else if (selection_.type == SelectionType::Path) removed = model_.deletePath(selection_.id);
    else if (selection_.type == SelectionType::Spawn) removed = model_.removeSpawn(selection_.id);
    else if (selection_.type == SelectionType::Goal) removed = model_.removeGoal(selection_.id);
    else if (selection_.type == SelectionType::Zone) removed = model_.removeZone(selection_.index);
    if (removed) { selection_.clear(); setStatus("Objekt gelöscht"); }
}

void MapForgeScreen::commitDrag(core::Vec2 target) {
    bool moved = false;
    if (selection_.type == SelectionType::PathPoint) moved = model_.movePathNode(selection_.id, selection_.subIndex, target);
    else if (selection_.type == SelectionType::Spawn) moved = model_.moveSpawn(selection_.id, target);
    else if (selection_.type == SelectionType::Goal) moved = model_.moveGoal(selection_.id, target);
    else if (selection_.type == SelectionType::Zone && selection_.index < model_.document().zones.size()) {
        const auto& rect = model_.document().zones[selection_.index].rect;
        target.x = std::clamp(target.x, 0.f, model_.document().width - rect.w);
        target.y = std::clamp(target.y, 0.f, model_.document().height - rect.h);
        moved = model_.moveZone(selection_.index, target);
    }
    if (moved) setStatus("Objekt verschoben");
}

std::string MapForgeScreen::selectedPathId() const {
    if (!activePathId_.empty() && std::any_of(model_.document().paths.begin(), model_.document().paths.end(), [&](const core::Path& path) { return path.id == activePathId_; })) return activePathId_;
    if (!model_.document().paths.empty()) return model_.document().paths.front().id;
    return "main";
}

void MapForgeScreen::focusIssue(const core::ValidationIssue& issue) {
    switch (issue.objectType) {
        case core::MapObjectType::Path: selection_ = {SelectionType::Path, issue.objectId, issue.index, 0}; break;
        case core::MapObjectType::PathPoint: selection_ = {SelectionType::PathPoint, issue.objectId, 0, issue.index}; break;
        case core::MapObjectType::Spawn: selection_ = {SelectionType::Spawn, issue.objectId, issue.index, 0}; break;
        case core::MapObjectType::Goal: selection_ = {SelectionType::Goal, issue.objectId, issue.index, 0}; break;
        case core::MapObjectType::Zone: selection_ = {SelectionType::Zone, {}, issue.index, 0}; break;
        default: selection_.clear(); break;
    }
    setStatus(issue.message);
}

void MapForgeScreen::requestExit() { confirmOrRun(PendingAction::Exit); }
void MapForgeScreen::requestNew() { confirmOrRun(PendingAction::NewMap); }
void MapForgeScreen::requestOpen(const std::filesystem::path& path) { pendingFile_ = path; confirmOrRun(PendingAction::OpenFile); }

void MapForgeScreen::confirmOrRun(PendingAction action) {
    pendingAction_ = action;
    if (model_.dirty()) dialog_ = Dialog::ConfirmDiscard;
    else performPendingAction();
}

void MapForgeScreen::performPendingAction() {
    const auto action = pendingAction_;
    pendingAction_ = PendingAction::None;
    if (action == PendingAction::Exit) context_.screens.replace({app::ScreenType::MainMenu});
    else if (action == PendingAction::Quit) context_.screens.quit();
    else if (action == PendingAction::NewMap) {
        model_.newMap("untitled", "Neue Karte");
        activePathId_ = "main";
        selection_.clear();
        camera_.fit(model_.document().width, model_.document().height);
        setStatus("Neue Karte erstellt");
    } else if (action == PendingAction::OpenFile) {
        std::string error;
        if (model_.load(pendingFile_.string(), &error)) {
            activePathId_ = model_.document().paths.empty() ? "main" : model_.document().paths.front().id;
            selection_.clear();
            camera_.fit(model_.document().width, model_.document().height);
            setStatus("Karte geladen: " + pendingFile_.filename().string());
        } else setStatus(error);
    }
}

bool MapForgeScreen::save() {
    if (model_.currentFile().empty()) { dialog_ = Dialog::SaveAs; return false; }
    std::string error;
    if (!model_.save(model_.currentFile(), &error)) { setStatus(error); return false; }
    setStatus("Gespeichert: " + std::filesystem::path(model_.currentFile()).filename().string());
    refreshMapFiles();
    return true;
}

bool MapForgeScreen::saveAs(const std::string& filename) {
    if (filename.empty()) { setStatus("Dateiname fehlt"); return false; }
    std::filesystem::path path = std::filesystem::path("maps") / std::filesystem::path(filename).filename();
    if (path.extension() != ".aegismap") path += ".aegismap";
    std::error_code errorCode;
    std::filesystem::create_directories(path.parent_path(), errorCode);
    std::string error;
    if (!model_.save(path.string(), &error)) { setStatus(error); return false; }
    filenameInput_ = path.filename().string();
    setStatus("Gespeichert unter: " + path.string());
    refreshMapFiles();
    return true;
}

void MapForgeScreen::refreshMapFiles() {
    mapFiles_.clear();
    std::error_code error;
    if (!std::filesystem::exists("maps", error)) return;
    for (const auto& entry : std::filesystem::directory_iterator("maps", error))
        if (entry.is_regular_file() && entry.path().extension() == ".aegismap") mapFiles_.push_back(entry.path());
    std::sort(mapFiles_.begin(), mapFiles_.end());
    if (mapScroll_ >= mapFiles_.size()) mapScroll_ = 0;
}

void MapForgeScreen::beginTextField(TextField field, const std::string& value) { textField_ = field; textBuffer_ = value; }

void MapForgeScreen::commitTextField() {
    auto& document = model_.editDocument("Map-Eigenschaft ändern");
    if (textField_ == TextField::MapName) document.metadata.name = textBuffer_;
    else if (textField_ == TextField::Description) document.metadata.description = textBuffer_;
    else if (textField_ == TextField::Author) document.metadata.author = textBuffer_;
    else if (textField_ == TextField::Biome) document.metadata.biome = textBuffer_;
    textField_ = TextField::None;
    textBuffer_.clear();
    setStatus("Eigenschaft geändert");
}

void MapForgeScreen::setStatus(std::string value) { statusMessage_ = std::move(value); statusTimer_ = 5.f; }

void MapForgeScreen::drawToolbar() {
    ui_.card(Toolbar, ui::Orange, false);
    ui_.iconButton({8.f, 10.f, 104.f, 44.f}, "MENÜ", ui::UiIcon::ArrowLeft,
                   ui::IconPlacement::Left, ui::Cyan, false, 14);
    ui_.button({120.f, 10.f, 68.f, 44.f}, "NEU", ui::Green, false, 13);
    ui_.button({194.f, 10.f, 78.f, 44.f}, "ÖFFNEN", ui::Cyan, false, 13);
    ui_.button({278.f, 10.f, 86.f, 44.f}, "SPEICHERN", ui::Gold, false, 12);
    ui_.button({370.f, 10.f, 108.f, 44.f}, "SPEICHERN UNTER", ui::Gold, false, 10);
    ui_.iconButton({486.f, 10.f, 54.f, 44.f}, "", ui::UiIcon::Undo,
                   ui::IconPlacement::Center, ui::Cyan, model_.canUndo());
    ui_.iconButton({546.f, 10.f, 54.f, 44.f}, "", ui::UiIcon::Redo,
                   ui::IconPlacement::Center, ui::Cyan, model_.canRedo());
    ui_.button({608.f, 10.f, 70.f, 44.f}, "GRID", ui::Cyan, gridVisible_, 12);
    ui_.button({684.f, 10.f, 70.f, 44.f}, "SNAP", ui::Green, snapEnabled_, 12);
    ui_.button({760.f, 10.f, 44.f, 44.f}, "G-", ui::Muted, false, 12);
    ui_.button({810.f, 10.f, 44.f, 44.f}, "G+", ui::Muted, false, 12);
    ui_.button({862.f, 10.f, 72.f, 44.f}, "FIT", ui::Purple, false, 12);
    ui_.button({944.f, 10.f, 148.f, 44.f}, "KARTE TESTEN", ui::Orange, false, 13);
    ui_.text("MAP FORGE", 18, {1340.f, 32.f}, ui::Orange, true, true);
    ui_.tooltip({120.f, 10.f, 68.f, 44.f}, "Neue Karte [Ctrl+N]", ui::Green);
    ui_.tooltip({194.f, 10.f, 78.f, 44.f}, "Karte öffnen [Ctrl+O]", ui::Cyan);
    ui_.tooltip({278.f, 10.f, 86.f, 44.f}, "Speichern [Ctrl+S]", ui::Gold);
    ui_.tooltip({486.f, 10.f, 54.f, 44.f}, "Rückgängig [Ctrl+Z]", ui::Cyan);
    ui_.tooltip({546.f, 10.f, 54.f, 44.f}, "Wiederholen [Ctrl+Y]", ui::Cyan);
    ui_.tooltip({944.f, 10.f, 148.f, 44.f}, "Aktuelle Karte sofort testen", ui::Orange);
}

void MapForgeScreen::drawTools() {
    ui_.card(ToolPanel, ui::Cyan, false);
    ui_.text("WERKZEUGE", 14, {90.f, 86.f}, ui::Muted, true, true);
    const std::array icons = {ui::UiIcon::Check, ui::UiIcon::ArrowRight, ui::UiIcon::Play, ui::UiIcon::Modified,
                              ui::UiIcon::Settings, ui::UiIcon::Close, ui::UiIcon::ArrowDown, ui::UiIcon::Delete};
    for (std::size_t i = 0; i < Tools.size(); ++i) {
        ui_.iconButton(toolRect(i), std::to_string(i + 1) + "  " + toolName(Tools[i]), icons[i], ui::IconPlacement::Left,
                       Tools[i] == Tool::Erase ? ui::Red : ui::Cyan, model_.tool() == Tools[i], 13);
        ui_.tooltip(toolRect(i), std::string(toolName(Tools[i])) + " [" + std::to_string(i + 1) + "]", Tools[i] == Tool::Erase ? ui::Red : ui::Cyan);
    }
    ui_.text("VORBEREITET", 12, {90.f, 598.f}, ui::Muted, true, true);
    ui_.panel({12.f, 618.f, 156.f, 84.f}, sf::Color(15, 25, 36), sf::Color(55, 70, 84));
    ui_.text("TERRAIN", 12, {28.f, 634.f}, ui::Muted, true);
    ui_.text("HEIGHT", 12, {28.f, 657.f}, ui::Muted, true);
    ui_.text("DECORATION", 12, {28.f, 680.f}, ui::Muted, true);
    ui_.wrapped("Für spätere Content-Phasen vorbereitet", 11, {16.f, 730.f, 150.f, 50.f}, ui::Muted, 3.f);
}

void MapForgeScreen::drawCanvas() {
    ui_.panel(Canvas, sf::Color(15, 24, 34), sf::Color(55, 89, 111), 1.f);
    const auto& document = model_.document();
    const auto topLeft = camera_.worldToScreen({0.f, 0.f});
    const auto bottomRight = camera_.worldToScreen({document.width, document.height});
    sf::RectangleShape mapShape({bottomRight.x - topLeft.x, bottomRight.y - topLeft.y});
    mapShape.setPosition(topLeft);
    sf::Color mapFill(24, 45, 43);
    if (document.metadata.biome == "frost") mapFill = sf::Color(27, 43, 55);
    else if (document.metadata.biome == "ember") mapFill = sf::Color(52, 35, 31);
    mapShape.setFillColor(mapFill);
    mapShape.setOutlineColor(sf::Color(106, 151, 166));
    mapShape.setOutlineThickness(2.f);
    context_.window.draw(mapShape);
    if (gridVisible_) drawGrid();
    drawZones();
    drawPaths();
    drawEndpoints();
    if (creatingZone_) {
        const auto a = camera_.worldToScreen(zoneStart_);
        const auto b = camera_.worldToScreen(dragCurrentWorld_);
        sf::RectangleShape preview({std::abs(b.x - a.x), std::abs(b.y - a.y)});
        preview.setPosition(std::min(a.x, b.x), std::min(a.y, b.y));
        const auto color = zoneColor(toolZoneType(model_.tool()));
        preview.setFillColor(withAlpha(color, 45)); preview.setOutlineColor(color); preview.setOutlineThickness(2.f);
        context_.window.draw(preview);
    }
}

void MapForgeScreen::drawGrid() {
    const auto& document = model_.document();
    const auto topLeft = camera_.worldToScreen({0.f, 0.f});
    const auto bottomRight = camera_.worldToScreen({document.width, document.height});
    sf::VertexArray lines(sf::Lines);
    const auto color = sf::Color(130, 170, 180, camera_.scale() * gridSize_ < 12.f ? 18 : 34);
    for (float x = 0.f; x <= document.width; x += gridSize_) {
        const float screenX = camera_.worldToScreen({x, 0.f}).x;
        lines.append(sf::Vertex({screenX, topLeft.y}, color)); lines.append(sf::Vertex({screenX, bottomRight.y}, color));
    }
    for (float y = 0.f; y <= document.height; y += gridSize_) {
        const float screenY = camera_.worldToScreen({0.f, y}).y;
        lines.append(sf::Vertex({topLeft.x, screenY}, color)); lines.append(sf::Vertex({bottomRight.x, screenY}, color));
    }
    context_.window.draw(lines);
}

void MapForgeScreen::drawZones() {
    const auto& zones = model_.document().zones;
    for (std::size_t i = 0; i < zones.size(); ++i) {
        const auto& zone = zones[i];
        const auto a = camera_.worldToScreen({zone.rect.x, zone.rect.y});
        const auto b = camera_.worldToScreen({zone.rect.x + zone.rect.w, zone.rect.y + zone.rect.h});
        sf::RectangleShape shape({b.x - a.x, b.y - a.y});
        shape.setPosition(a);
        const auto color = zoneColor(zone.type);
        const bool selected = selection_.type == SelectionType::Zone && selection_.index == i;
        shape.setFillColor(withAlpha(color, selected ? 75 : 38));
        shape.setOutlineColor(selected ? sf::Color::White : withAlpha(color, 190));
        shape.setOutlineThickness(selected ? 3.f : 1.5f);
        context_.window.draw(shape);
    }
}

void MapForgeScreen::drawPaths() {
    for (const auto& path : model_.document().paths) {
        const bool selectedPath = selection_.id == path.id;
        for (std::size_t i = 0; i + 1 < path.nodes.size(); ++i) {
            const auto a = camera_.worldToScreen(path.nodes[i]);
            const auto b = camera_.worldToScreen(path.nodes[i + 1]);
            const auto delta = b - a;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            const float angle = std::atan2(delta.y, delta.x) * 180.f / PI_F;
            sf::RectangleShape underlay({length, selectedPath ? 12.f : 10.f});
            underlay.setOrigin(0.f, underlay.getSize().y / 2.f); underlay.setPosition(a); underlay.setRotation(angle); underlay.setFillColor(sf::Color(5, 13, 18, 210)); context_.window.draw(underlay);
            sf::RectangleShape line({length, selectedPath ? 5.f : 4.f});
            line.setOrigin(0.f, line.getSize().y / 2.f); line.setPosition(a); line.setRotation(angle); line.setFillColor(selectedPath ? ui::Gold : ui::Cyan); context_.window.draw(line);
        }
        for (std::size_t i = 0; i < path.nodes.size(); ++i) {
            const bool selected = selection_.type == SelectionType::PathPoint && selection_.id == path.id && selection_.subIndex == i;
            const auto fill = i == 0 ? ui::Green : (i + 1 == path.nodes.size() ? ui::Orange : ui::Cyan);
            drawMarker(path.nodes[i], selected ? 10.f : 7.f, fill, selected ? sf::Color::White : sf::Color(8, 20, 28));
        }
    }
}

void MapForgeScreen::drawEndpoints() {
    for (const auto& spawn : model_.document().spawns)
        drawMarker(spawn.position, selection_.type == SelectionType::Spawn && selection_.id == spawn.id ? 15.f : 12.f, ui::Green, sf::Color::White, "S");
    for (const auto& goal : model_.document().goals)
        drawMarker(goal.position, selection_.type == SelectionType::Goal && selection_.id == goal.id ? 15.f : 12.f, ui::Red, sf::Color::White, "G");
}

void MapForgeScreen::drawMarker(core::Vec2 point, float radius, sf::Color fill, sf::Color outline, const std::string& label) {
    sf::CircleShape marker(radius);
    marker.setOrigin(radius, radius); marker.setPosition(camera_.worldToScreen(point)); marker.setFillColor(fill); marker.setOutlineColor(outline); marker.setOutlineThickness(2.f); context_.window.draw(marker);
    if (!label.empty()) ui_.text(label, static_cast<unsigned>(radius), camera_.worldToScreen(point), sf::Color(8, 18, 24), true, true);
}

void MapForgeScreen::drawInspector() {
    ui_.card(Inspector, ui::Purple, false);
    ui_.text(selection_.empty() ? "MAP PROPERTIES" : "INSPECTOR", 18, {1240.f, 88.f}, ui::Text, true);
    const auto& document = model_.document();
    if (selection_.empty()) {
        ui_.text("Name", 12, {1240.f, 122.f}, ui::Muted, true);
        ui_.panel({1240.f, 145.f, 340.f, 38.f}, sf::Color(18, 30, 43), textField_ == TextField::MapName ? ui::Cyan : sf::Color(55, 84, 102));
        ui_.text(textField_ == TextField::MapName ? textBuffer_ : document.metadata.name, 15, {1250.f, 155.f}, ui::Text);
        ui_.text("Beschreibung", 12, {1240.f, 192.f}, ui::Muted, true);
        ui_.panel({1240.f, 215.f, 340.f, 54.f}, sf::Color(18, 30, 43), textField_ == TextField::Description ? ui::Cyan : sf::Color(55, 84, 102));
        ui_.wrapped(textField_ == TextField::Description ? textBuffer_ : document.metadata.description, 12, {1250.f, 225.f, 320.f, 36.f}, ui::Text, 2.f);
        ui_.text("Autor", 12, {1240.f, 280.f}, ui::Muted, true);
        ui_.panel({1240.f, 301.f, 340.f, 38.f}, sf::Color(18, 30, 43), textField_ == TextField::Author ? ui::Cyan : sf::Color(55, 84, 102));
        ui_.text(textField_ == TextField::Author ? textBuffer_ : document.metadata.author, 14, {1250.f, 311.f}, ui::Text);
        ui_.text("Biome / Material-ID", 12, {1240.f, 350.f}, ui::Muted, true);
        ui_.panel({1240.f, 371.f, 340.f, 38.f}, sf::Color(18, 30, 43), textField_ == TextField::Biome ? ui::Cyan : sf::Color(55, 84, 102));
        ui_.text(textField_ == TextField::Biome ? textBuffer_ : document.metadata.biome, 14, {1250.f, 381.f}, ui::Text);
        ui_.text("Breite  " + std::to_string(static_cast<int>(document.width)), 13, {1240.f, 439.f}, ui::Text, true);
        ui_.button({1450.f, 430.f, 54.f, 34.f}, "-", ui::Muted, false, 18); ui_.button({1510.f, 430.f, 54.f, 34.f}, "+", ui::Green, false, 18);
        ui_.text("Höhe    " + std::to_string(static_cast<int>(document.height)), 13, {1240.f, 481.f}, ui::Text, true);
        ui_.button({1450.f, 472.f, 54.f, 34.f}, "-", ui::Muted, false, 18); ui_.button({1510.f, 472.f, 54.f, 34.f}, "+", ui::Green, false, 18);
    } else if (selection_.type == SelectionType::Path || selection_.type == SelectionType::PathPoint) {
        ui_.text("Pfad-ID", 12, {1240.f, 130.f}, ui::Muted, true); ui_.text(selection_.id, 18, {1240.f, 154.f}, ui::Cyan, true);
        const auto path = std::find_if(document.paths.begin(), document.paths.end(), [&](const core::Path& value) { return value.id == selection_.id; });
        if (path != document.paths.end()) ui_.text("Punkte: " + std::to_string(path->nodes.size()), 14, {1240.f, 194.f}, ui::Text);
        if (selection_.type == SelectionType::PathPoint && path != document.paths.end() && selection_.subIndex < path->nodes.size()) {
            const auto point = path->nodes[selection_.subIndex];
            ui_.text("Punkt " + std::to_string(selection_.subIndex), 14, {1240.f, 232.f}, ui::Gold, true);
            ui_.text("X  " + std::to_string(static_cast<int>(point.x)), 14, {1240.f, 264.f}, ui::Text);
            ui_.text("Y  " + std::to_string(static_cast<int>(point.y)), 14, {1240.f, 294.f}, ui::Text);
        }
        ui_.button({1240.f, 350.f, 160.f, 40.f}, "NEUER PFAD", ui::Green, false, 12);
        ui_.button({1410.f, 350.f, 170.f, 40.f}, "PFAD LÖSCHEN", ui::Red, false, 12);
    } else if (selection_.type == SelectionType::Spawn || selection_.type == SelectionType::Goal) {
        const bool spawn = selection_.type == SelectionType::Spawn;
        ui_.text(spawn ? "SPAWN" : "GOAL", 18, {1240.f, 134.f}, spawn ? ui::Green : ui::Red, true);
        ui_.text("ID  " + selection_.id, 14, {1240.f, 174.f}, ui::Text);
        core::Vec2 position{}; std::string pathId;
        if (spawn) { const auto it = std::find_if(document.spawns.begin(), document.spawns.end(), [&](const core::SpawnPoint& value) { return value.id == selection_.id; }); if (it != document.spawns.end()) { position = it->position; pathId = it->pathId; } }
        else { const auto it = std::find_if(document.goals.begin(), document.goals.end(), [&](const core::GoalPoint& value) { return value.id == selection_.id; }); if (it != document.goals.end()) { position = it->position; pathId = it->pathId; } }
        ui_.text("Pfad  " + pathId, 14, {1240.f, 212.f}, ui::Cyan);
        ui_.text("Position  " + std::to_string(static_cast<int>(position.x)) + " / " + std::to_string(static_cast<int>(position.y)), 14, {1240.f, 250.f}, ui::Text);
        ui_.wrapped("Ziehen oder Pfeiltasten zum Verschieben · ENTF zum Löschen", 13, {1240.f, 300.f, 330.f, 80.f}, ui::Muted, 4.f);
    } else if (selection_.type == SelectionType::Zone && selection_.index < document.zones.size()) {
        const auto& zone = document.zones[selection_.index];
        ui_.text("ZONE " + std::to_string(selection_.index), 18, {1240.f, 134.f}, zoneColor(zone.type), true);
        ui_.text("Typ  " + std::string(core::toString(zone.type)), 14, {1240.f, 176.f}, ui::Text);
        ui_.text("Position  " + std::to_string(static_cast<int>(zone.rect.x)) + " / " + std::to_string(static_cast<int>(zone.rect.y)), 14, {1240.f, 218.f}, ui::Text);
        ui_.text("Breite  " + std::to_string(static_cast<int>(zone.rect.w)), 14, {1240.f, 270.f}, ui::Text);
        ui_.button({1450.f, 340.f, 54.f, 32.f}, "W-", ui::Muted, false, 12); ui_.button({1510.f, 340.f, 54.f, 32.f}, "W+", ui::Green, false, 12);
        ui_.text("Höhe    " + std::to_string(static_cast<int>(zone.rect.h)), 14, {1240.f, 318.f}, ui::Text);
        ui_.button({1450.f, 388.f, 54.f, 32.f}, "H-", ui::Muted, false, 12); ui_.button({1510.f, 388.f, 54.f, 32.f}, "H+", ui::Green, false, 12);
        ui_.button({1240.f, 455.f, 340.f, 38.f}, "ZONENTYP WECHSELN", zoneColor(zone.type), false, 12);
    }
    drawValidation();
}

void MapForgeScreen::drawValidation() {
    const auto issues = model_.validateDetailed();
    const auto errors = std::count_if(issues.begin(), issues.end(), [](const core::ValidationIssue& issue) { return issue.severity == core::ValidationSeverity::Error; });
    const auto warnings = static_cast<int>(issues.size()) - static_cast<int>(errors);
    ui_.panel({1230.f, 590.f, 360.f, 250.f}, sf::Color(8, 16, 25, 235), sf::Color(48, 78, 98));
    ui_.text("VALIDIERUNG", 15, {1242.f, 606.f}, ui::Text, true);
    ui_.text(std::to_string(errors) + " ERRORS", 12, {1395.f, 608.f}, errors ? ui::Red : ui::Green, true);
    ui_.text(std::to_string(warnings) + " WARNINGS", 12, {1490.f, 608.f}, warnings ? ui::Gold : ui::Muted, true);
    if (issues.empty()) ui_.text("Map ist spielbereit.", 14, {1242.f, 654.f}, ui::Green, true);
    for (std::size_t i = 0; i < std::min<std::size_t>(issues.size(), 7); ++i) {
        const auto& issue = issues[i];
        const auto color = issue.severity == core::ValidationSeverity::Error ? ui::Red : ui::Gold;
        if (issue.severity == core::ValidationSeverity::Error)
            ui_.icon(ui::UiIcon::Close, {1249.f, 660.f + static_cast<float>(i) * 27.f}, 10.f, color);
        else
            ui_.text("!", 14, {1242.f, 650.f + static_cast<float>(i) * 27.f}, color, true);
        std::string message = issue.message;
        if (message.size() > 42) message = message.substr(0, 39) + "…";
        ui_.text(message, 11, {1262.f, 652.f + static_cast<float>(i) * 27.f}, ui::Text);
    }
}

void MapForgeScreen::drawStatusBar() {
    ui_.panel(StatusBar, sf::Color(7, 14, 22, 252), sf::Color(48, 78, 98));
    std::ostringstream coordinate;
    coordinate << std::fixed << std::setprecision(1) << mouseWorld_.x << " / " << mouseWorld_.y;
    ui_.text(std::string("TOOL  ") + toolName(model_.tool()), 12, {16.f, 868.f}, ui::Cyan, true);
    ui_.text("WORLD  " + coordinate.str(), 12, {190.f, 868.f}, ui::Text);
    ui_.text("ZOOM  " + std::to_string(static_cast<int>(camera_.zoomPercent())) + "%", 12, {390.f, 868.f}, ui::Text);
    ui_.text("GRID  " + std::to_string(static_cast<int>(gridSize_)) + (gridVisible_ ? " ON" : " OFF"), 12, {505.f, 868.f}, gridVisible_ ? ui::Cyan : ui::Muted);
    ui_.text(std::string("SNAP  ") + (snapEnabled_ ? "ON" : "OFF"), 12, {640.f, 868.f}, snapEnabled_ ? ui::Green : ui::Muted);
    const auto stateColor = model_.dirty() ? ui::Gold : ui::Green;
    ui_.icon(model_.dirty() ? ui::UiIcon::Modified : ui::UiIcon::Check, {762.f, 875.f}, 11.f, stateColor);
    ui_.text(model_.dirty() ? "GEÄNDERT" : "GESPEICHERT", 12, {775.f, 868.f}, stateColor, true);
    ui_.text(statusTimer_ > 0.f ? statusMessage_ : (model_.currentFile().empty() ? "Ungespeicherte Karte" : model_.currentFile()), 12, {920.f, 868.f}, ui::Muted);
}

void MapForgeScreen::drawDialog() {
    ui_.panel({0.f, 0.f, 1600.f, 900.f}, sf::Color(0, 0, 0, 175));
    ui_.panel({480.f, 170.f, 640.f, 560.f}, sf::Color(10, 19, 29, 255), sf::Color(86, 151, 184), 2.f);
    if (dialog_ == Dialog::Open) {
        ui_.text("KARTE ÖFFNEN", 28, {800.f, 215.f}, ui::Cyan, true, true);
        ui_.text("maps/", 14, {520.f, 255.f}, ui::Muted, true);
        const auto visible = std::min<std::size_t>(mapFiles_.size() - std::min(mapScroll_, mapFiles_.size()), 8);
        for (std::size_t row = 0; row < visible; ++row)
            ui_.button({520.f, 285.f + static_cast<float>(row) * 48.f, 560.f, 40.f}, mapFiles_[mapScroll_ + row].filename().string(), ui::Cyan, false, 14);
        if (mapFiles_.empty()) ui_.text("Keine .aegismap-Dateien gefunden.", 16, {800.f, 360.f}, ui::Muted, false, true);
        if (mapFiles_.size() > 8) ui_.text("Mausrad: weitere Karten", 12, {540.f, 680.f}, ui::Muted);
        ui_.button({820.f, 670.f, 220.f, 42.f}, "ABBRECHEN", ui::Muted, false, 14);
    } else if (dialog_ == Dialog::SaveAs) {
        ui_.text("SPEICHERN UNTER", 28, {800.f, 235.f}, ui::Gold, true, true);
        ui_.text("Dateiname im maps/-Ordner", 14, {540.f, 305.f}, ui::Muted, true);
        ui_.panel({540.f, 335.f, 520.f, 48.f}, sf::Color(18, 30, 43), ui::Gold, 2.f);
        ui_.text(filenameInput_, 17, {555.f, 349.f}, ui::Text);
        ui_.button({555.f, 430.f, 235.f, 48.f}, "SPEICHERN", ui::Green, false, 15);
        ui_.button({810.f, 430.f, 235.f, 48.f}, "ABBRECHEN", ui::Muted, false, 15);
    } else {
        ui_.text("UNGESPEICHERTE ÄNDERUNGEN", 26, {800.f, 245.f}, ui::Gold, true, true);
        ui_.wrapped("Die Karte wurde geändert. Speichere sie, verwirf die Änderungen oder kehre zum Editor zurück.", 18, {560.f, 315.f, 480.f, 100.f}, ui::Text, 7.f);
        ui_.button({535.f, 455.f, 165.f, 50.f}, "SPEICHERN", ui::Green, false, 14);
        ui_.button({717.f, 455.f, 165.f, 50.f}, "VERWERFEN", ui::Red, false, 14);
        ui_.button({900.f, 455.f, 165.f, 50.f}, "ABBRECHEN", ui::Muted, false, 14);
    }
}

void MapForgeScreen::dialogPressed(sf::Vector2f screen) {
    if (dialog_ == Dialog::Open) {
        const auto visible = std::min<std::size_t>(mapFiles_.size() - std::min(mapScroll_, mapFiles_.size()), 8);
        for (std::size_t row = 0; row < visible; ++row) {
            if (sf::FloatRect(520.f, 285.f + static_cast<float>(row) * 48.f, 560.f, 40.f).contains(screen)) {
                dialog_ = Dialog::None;
                requestOpen(mapFiles_[mapScroll_ + row]);
                return;
            }
        }
        if (sf::FloatRect(820.f, 670.f, 220.f, 42.f).contains(screen)) dialog_ = Dialog::None;
    } else if (dialog_ == Dialog::SaveAs) {
        if (sf::FloatRect(555.f, 430.f, 235.f, 48.f).contains(screen)) {
            if (saveAs(filenameInput_)) { dialog_ = Dialog::None; performPendingAction(); }
        } else if (sf::FloatRect(810.f, 430.f, 235.f, 48.f).contains(screen)) { dialog_ = Dialog::None; pendingAction_ = PendingAction::None; }
    } else {
        if (sf::FloatRect(535.f, 455.f, 165.f, 50.f).contains(screen)) {
            if (model_.currentFile().empty()) dialog_ = Dialog::SaveAs;
            else if (save()) { dialog_ = Dialog::None; performPendingAction(); }
        } else if (sf::FloatRect(717.f, 455.f, 165.f, 50.f).contains(screen)) { dialog_ = Dialog::None; performPendingAction(); }
        else if (sf::FloatRect(900.f, 455.f, 165.f, 50.f).contains(screen)) { dialog_ = Dialog::None; pendingAction_ = PendingAction::None; }
    }
}

} // namespace aegis::editor
