#include <mediaforge/input/InputState.hpp>

#include <algorithm>
#include <type_traits>

namespace mf {
namespace {

template <typename Enum, std::size_t Size>
std::size_t indexOf(Enum value) noexcept {
    return std::min<std::size_t>(static_cast<std::size_t>(value), Size - 1);
}

} // namespace

void InputState::beginFrame() noexcept {
    keysPressed_.fill(false);
    keysReleased_.fill(false);
    mousePressed_.fill(false);
    mouseReleased_.fill(false);
    mouseDelta_ = {};
    wheelDelta_ = {};
}

void InputState::consume(const Event& event) noexcept {
    std::visit([this](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, KeyboardEvent>) {
            const auto index = indexOf<Key, keyCount>(value.key);
            (value.pressed ? keysPressed_ : keysReleased_)[index] = true;
            keysDown_[index] = value.pressed;
        } else if constexpr (std::is_same_v<T, MouseButtonEvent>) {
            const auto index = indexOf<MouseButton, mouseButtonCount>(value.button);
            (value.pressed ? mousePressed_ : mouseReleased_)[index] = true;
            mouseDown_[index] = value.pressed;
            mousePosition_ = value.position;
        } else if constexpr (std::is_same_v<T, MouseMoveEvent>) {
            mousePosition_ = value.position;
            mouseDelta_ = mouseDelta_ + value.delta;
        } else if constexpr (std::is_same_v<T, MouseWheelEvent>) {
            wheelDelta_ = wheelDelta_ + value.delta;
        }
    }, event);
}

bool InputState::keyDown(Key key) const noexcept { return keysDown_[indexOf<Key, keyCount>(key)]; }
bool InputState::keyPressed(Key key) const noexcept { return keysPressed_[indexOf<Key, keyCount>(key)]; }
bool InputState::keyReleased(Key key) const noexcept { return keysReleased_[indexOf<Key, keyCount>(key)]; }
bool InputState::mouseButtonDown(MouseButton button) const noexcept { return mouseDown_[indexOf<MouseButton, mouseButtonCount>(button)]; }
bool InputState::mouseButtonPressed(MouseButton button) const noexcept { return mousePressed_[indexOf<MouseButton, mouseButtonCount>(button)]; }
bool InputState::mouseButtonReleased(MouseButton button) const noexcept { return mouseReleased_[indexOf<MouseButton, mouseButtonCount>(button)]; }

} // namespace mf
