#pragma once

#include <mediaforge/input/InputState.hpp>
#include <mediaforge/math/Math.hpp>

#include <cstdint>

namespace mf::ui {

using WidgetId = std::uint64_t;

struct Rect {
    Vec2 position{};
    Vec2 size{};

    [[nodiscard]] bool contains(Vec2 point) const noexcept;
    friend constexpr bool operator==(Rect, Rect) noexcept = default;
};

struct CanvasPoint {
    Vec2 position{};
    bool inside{};
};

class Canvas {
public:
    explicit Canvas(Vec2 contentSize = {1600.0F, 900.0F}) noexcept;

    void setTargetSize(Vec2 targetSize) noexcept;
    [[nodiscard]] Vec2 contentSize() const noexcept { return contentSize_; }
    [[nodiscard]] Rect viewport() const noexcept { return viewport_; }
    [[nodiscard]] CanvasPoint mapToCanvas(Vec2 targetPoint) const noexcept;

private:
    Vec2 contentSize_;
    Rect viewport_;
};

enum class WidgetState : std::uint8_t { normal, hovered, pressed, disabled, focused };

struct ButtonResult {
    WidgetState state{WidgetState::normal};
    bool activated{};
    bool consumed{};
};

class Context {
public:
    void beginFrame(const InputState& input, const Canvas& canvas) noexcept;
    void beginFrame(Vec2 pointerPosition, bool pointerInside, bool pointerDown,
                    bool pointerPressed, bool pointerReleased, bool keyboardActivate = false) noexcept;
    [[nodiscard]] ButtonResult button(WidgetId id, Rect bounds, bool enabled = true) noexcept;
    void endFrame() noexcept;

    void setKeyboardFocus(WidgetId id) noexcept { keyboardFocus_ = id; }
    [[nodiscard]] WidgetId keyboardFocus() const noexcept { return keyboardFocus_; }
    [[nodiscard]] bool inputConsumed() const noexcept { return inputConsumed_; }

private:
    Vec2 pointerPosition_{};
    WidgetId activePointer_{};
    WidgetId keyboardFocus_{};
    bool pointerInside_{};
    bool pointerDown_{};
    bool pointerPressed_{};
    bool pointerReleased_{};
    bool keyboardActivate_{};
    bool inputConsumed_{};
};

} // namespace mf::ui
