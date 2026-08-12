#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

namespace aegis::input {

enum class Context { Menu, Gameplay, MapForge, Modal };
enum class Action {
    MenuConfirm, MenuBack, Pause, GameSpeedUp, GameSpeedDown, StartWave,
    Tower1, Tower2, Tower3, Tower4, Tower5, Tower6,
    UpgradeTower, SellTower, ChangeTargeting, OpenHelp,
    EditorSelect, EditorPath, EditorSpawn, EditorGoal, EditorBuildZone, EditorBlockedZone,
    EditorWater, EditorErase, EditorUndo, EditorRedo, EditorSave, EditorSaveAs,
    EditorOpen, EditorNew, EditorPlaytest, EditorGrid, EditorSnap, EditorFit,
    DeleteSelection
};

struct Binding {
    sf::Keyboard::Key key = sf::Keyboard::Unknown;
    bool control = false;
    bool shift = false;
    bool alt = false;
    auto operator<=>(const Binding&) const = default;
};

class InputSystem {
public:
    InputSystem();

    void setContext(Context context) { context_ = context; }
    Context context() const { return context_; }
    void bind(Context context, Action action, Binding binding);
    void bindAlternate(Context context, Action action, Binding binding);
    std::optional<Binding> binding(Context context, Action action) const;
    bool triggered(Action action, const sf::Event& event) const;

    void beginFrame();
    void handleEvent(const sf::Event& event);
    bool pointerPressed(sf::Mouse::Button button) const;
    bool pointerReleased(sf::Mouse::Button button) const;
    bool consumePointerPress(sf::Mouse::Button button);
    void capturePointer(bool captured) { pointerCaptured_ = captured; }
    bool pointerCaptured() const { return pointerCaptured_; }
    float wheelDelta() const { return wheelDelta_; }

private:
    static std::size_t key(Context context, Action action);
    std::unordered_map<std::size_t, std::vector<Binding>> bindings_;
    Context context_ = Context::Menu;
    std::vector<sf::Mouse::Button> pressedButtons_;
    std::vector<sf::Mouse::Button> releasedButtons_;
    std::vector<sf::Mouse::Button> consumedButtons_;
    bool pointerCaptured_ = false;
    float wheelDelta_ = 0.f;
};

} // namespace aegis::input
