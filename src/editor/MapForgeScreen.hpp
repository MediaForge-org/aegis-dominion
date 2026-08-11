#pragma once

#include "EditorCamera.hpp"
#include "MapEditorModel.hpp"
#include "app/Screen.hpp"
#include "ui/UiRenderer.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace aegis::editor {

class MapForgeScreen final : public app::Screen {
public:
    explicit MapForgeScreen(app::ScreenContext context);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render() override;
    void onResume() override;

private:
    enum class Dialog { None, Open, SaveAs, ConfirmDiscard };
    enum class PendingAction { None, Exit, Quit, NewMap, OpenFile };
    enum class TextField { None, MapName, Description, Author, Biome };

    void handleKey(const sf::Event::KeyEvent& key);
    void handleText(sf::Uint32 unicode);
    void mousePressed(sf::Vector2f screen, sf::Mouse::Button button);
    void mouseReleased(sf::Vector2f screen, sf::Mouse::Button button);
    void mouseMoved(sf::Vector2f screen);
    void canvasPressed(sf::Vector2f screen);
    void toolbarPressed(sf::Vector2f screen);
    void toolsPressed(sf::Vector2f screen);
    void inspectorPressed(sf::Vector2f screen);
    void dialogPressed(sf::Vector2f screen);

    core::Vec2 snapped(core::Vec2 point) const;
    core::Vec2 clamped(core::Vec2 point) const;
    EditorSelection hitTest(sf::Vector2f screen) const;
    std::optional<std::pair<std::string, std::size_t>> segmentHit(sf::Vector2f screen) const;
    void eraseSelection(const EditorSelection& selection);
    void deleteSelected();
    void commitDrag(core::Vec2 target);
    void focusIssue(const core::ValidationIssue& issue);
    std::string selectedPathId() const;

    void requestExit();
    void requestNew();
    void requestOpen(const std::filesystem::path& path);
    void confirmOrRun(PendingAction action);
    void performPendingAction();
    bool save();
    bool saveAs(const std::string& filename);
    void refreshMapFiles();
    void beginTextField(TextField field, const std::string& value);
    void commitTextField();
    void setStatus(std::string value);

    void drawToolbar();
    void drawTools();
    void drawCanvas();
    void drawGrid();
    void drawZones();
    void drawPaths();
    void drawEndpoints();
    void drawInspector();
    void drawValidation();
    void drawStatusBar();
    void drawDialog();
    void drawMarker(core::Vec2 point, float radius, sf::Color fill, sf::Color outline, const std::string& label = {});

    ui::UiRenderer ui_;
    MapEditorModel model_;
    EditorCamera camera_;
    EditorSelection selection_;
    std::string activePathId_ = "main";
    bool gridVisible_ = true;
    bool snapEnabled_ = true;
    float gridSize_ = 32.f;
    bool panning_ = false;
    bool dragging_ = false;
    bool creatingZone_ = false;
    sf::Vector2f lastMouseScreen_{};
    core::Vec2 mouseWorld_{};
    core::Vec2 dragStartWorld_{};
    core::Vec2 dragCurrentWorld_{};
    core::Vec2 zoneStart_{};
    Dialog dialog_ = Dialog::None;
    PendingAction pendingAction_ = PendingAction::None;
    std::filesystem::path pendingFile_;
    std::vector<std::filesystem::path> mapFiles_;
    std::size_t mapScroll_ = 0;
    std::string filenameInput_ = "neue_karte.aegismap";
    TextField textField_ = TextField::None;
    std::string textBuffer_;
    std::string statusMessage_ = "Bereit";
    float statusTimer_ = 0.f;
};

} // namespace aegis::editor
