#include "InputSystem.hpp"

#include <algorithm>

namespace aegis::input {

std::size_t InputSystem::key(Context context, Action action) {
    return static_cast<std::size_t>(context) * 256u + static_cast<std::size_t>(action);
}

InputSystem::InputSystem() {
    bind(Context::Menu, Action::MenuConfirm, {sf::Keyboard::Enter});
    bind(Context::Menu, Action::MenuBack, {sf::Keyboard::Escape});
    bind(Context::Modal, Action::MenuConfirm, {sf::Keyboard::Enter});
    bind(Context::Modal, Action::MenuBack, {sf::Keyboard::Escape});

    bind(Context::Gameplay, Action::Pause, {sf::Keyboard::Escape});
    bindAlternate(Context::Gameplay, Action::Pause, {sf::Keyboard::P});
    bind(Context::Gameplay, Action::StartWave, {sf::Keyboard::Space});
    bind(Context::Gameplay, Action::GameSpeedUp, {sf::Keyboard::F});
    bind(Context::Gameplay, Action::UpgradeTower, {sf::Keyboard::U});
    bind(Context::Gameplay, Action::ChangeTargeting, {sf::Keyboard::T});
    bind(Context::Gameplay, Action::SellTower, {sf::Keyboard::S});
    bind(Context::Gameplay, Action::OpenHelp, {sf::Keyboard::F1});
    const Action towerActions[] = {Action::Tower1, Action::Tower2, Action::Tower3, Action::Tower4, Action::Tower5, Action::Tower6};
    const sf::Keyboard::Key towerKeys[] = {sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4, sf::Keyboard::Num5, sf::Keyboard::Num6};
    for (std::size_t i = 0; i < 6; ++i) bind(Context::Gameplay, towerActions[i], {towerKeys[i]});

    bind(Context::MapForge, Action::MenuBack, {sf::Keyboard::Escape});
    bind(Context::MapForge, Action::EditorUndo, {sf::Keyboard::Z, true});
    bind(Context::MapForge, Action::EditorRedo, {sf::Keyboard::Y, true});
    bind(Context::MapForge, Action::EditorSave, {sf::Keyboard::S, true});
    bind(Context::MapForge, Action::EditorSaveAs, {sf::Keyboard::S, true, true});
    bind(Context::MapForge, Action::EditorOpen, {sf::Keyboard::O, true});
    bind(Context::MapForge, Action::EditorNew, {sf::Keyboard::N, true});
    bind(Context::MapForge, Action::DeleteSelection, {sf::Keyboard::Delete});
    bind(Context::MapForge, Action::EditorGrid, {sf::Keyboard::G});
    bind(Context::MapForge, Action::EditorSnap, {sf::Keyboard::H});
    bind(Context::MapForge, Action::EditorFit, {sf::Keyboard::F});
    const Action tools[] = {Action::EditorSelect, Action::EditorPath, Action::EditorSpawn, Action::EditorGoal,
                            Action::EditorBuildZone, Action::EditorBlockedZone, Action::EditorWater, Action::EditorErase};
    const sf::Keyboard::Key toolKeys[] = {sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4,
                                         sf::Keyboard::Num5, sf::Keyboard::Num6, sf::Keyboard::Num7, sf::Keyboard::Num8};
    for (std::size_t i = 0; i < 8; ++i) bind(Context::MapForge, tools[i], {toolKeys[i]});
}

void InputSystem::bind(Context context, Action action, Binding value) { bindings_[key(context, action)] = {value}; }
void InputSystem::bindAlternate(Context context, Action action, Binding value) { bindings_[key(context, action)].push_back(value); }

std::optional<Binding> InputSystem::binding(Context context, Action action) const {
    const auto found = bindings_.find(key(context, action));
    if (found == bindings_.end()) return std::nullopt;
    if (found->second.empty()) return std::nullopt;
    return found->second.front();
}

bool InputSystem::triggered(Action action, const sf::Event& event) const {
    if (event.type != sf::Event::KeyPressed) return false;
    const auto found = bindings_.find(key(context_, action));
    if (found == bindings_.end()) return false;
    return std::ranges::any_of(found->second, [&](const Binding& value) {
        return event.key.code == value.key && event.key.control == value.control && event.key.shift == value.shift && event.key.alt == value.alt;
    });
}

void InputSystem::beginFrame() {
    pressedButtons_.clear();
    releasedButtons_.clear();
    consumedButtons_.clear();
    wheelDelta_ = 0.f;
}

void InputSystem::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) pressedButtons_.push_back(event.mouseButton.button);
    else if (event.type == sf::Event::MouseButtonReleased) releasedButtons_.push_back(event.mouseButton.button);
    else if (event.type == sf::Event::MouseWheelScrolled) wheelDelta_ += event.mouseWheelScroll.delta;
}

bool InputSystem::pointerPressed(sf::Mouse::Button button) const {
    return std::ranges::find(pressedButtons_, button) != pressedButtons_.end() &&
           std::ranges::find(consumedButtons_, button) == consumedButtons_.end();
}

bool InputSystem::pointerReleased(sf::Mouse::Button button) const {
    return std::ranges::find(releasedButtons_, button) != releasedButtons_.end();
}

bool InputSystem::consumePointerPress(sf::Mouse::Button button) {
    if (!pointerPressed(button)) return false;
    consumedButtons_.push_back(button);
    return true;
}

} // namespace aegis::input
