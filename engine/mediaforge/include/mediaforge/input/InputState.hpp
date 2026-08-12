#pragma once

#include <mediaforge/input/Event.hpp>

#include <array>

namespace mf {

class InputState {
public:
    void beginFrame() noexcept;
    void consume(const Event& event) noexcept;

    [[nodiscard]] bool keyDown(Key key) const noexcept;
    [[nodiscard]] bool keyPressed(Key key) const noexcept;
    [[nodiscard]] bool keyReleased(Key key) const noexcept;
    [[nodiscard]] bool mouseButtonDown(MouseButton button) const noexcept;
    [[nodiscard]] bool mouseButtonPressed(MouseButton button) const noexcept;
    [[nodiscard]] bool mouseButtonReleased(MouseButton button) const noexcept;
    [[nodiscard]] Vec2 mousePosition() const noexcept { return mousePosition_; }
    [[nodiscard]] Vec2 mouseDelta() const noexcept { return mouseDelta_; }
    [[nodiscard]] Vec2 wheelDelta() const noexcept { return wheelDelta_; }

private:
    static constexpr std::size_t keyCount = 512;
    static constexpr std::size_t mouseButtonCount = 8;
    std::array<bool, keyCount> keysDown_{};
    std::array<bool, keyCount> keysPressed_{};
    std::array<bool, keyCount> keysReleased_{};
    std::array<bool, mouseButtonCount> mouseDown_{};
    std::array<bool, mouseButtonCount> mousePressed_{};
    std::array<bool, mouseButtonCount> mouseReleased_{};
    Vec2 mousePosition_{};
    Vec2 mouseDelta_{};
    Vec2 wheelDelta_{};
};

} // namespace mf
