#include <mediaforge/ui/Ui.hpp>

#include <algorithm>

namespace mf::ui {

bool Rect::contains(Vec2 point) const noexcept {
    return size.x >= 0.0F && size.y >= 0.0F && point.x >= position.x && point.y >= position.y &&
           point.x <= position.x + size.x && point.y <= position.y + size.y;
}

Canvas::Canvas(Vec2 contentSize) noexcept
    : contentSize_{std::max(contentSize.x, 0.001F), std::max(contentSize.y, 0.001F)},
      viewport_{{}, contentSize_} {}

void Canvas::setTargetSize(Vec2 targetSize) noexcept {
    if (targetSize.x <= 0.0F || targetSize.y <= 0.0F) {
        viewport_ = {};
        return;
    }
    const float scale = std::min(targetSize.x / contentSize_.x, targetSize.y / contentSize_.y);
    viewport_.size = contentSize_ * scale;
    viewport_.position = {(targetSize.x - viewport_.size.x) * 0.5F,
                          (targetSize.y - viewport_.size.y) * 0.5F};
}

CanvasPoint Canvas::mapToCanvas(Vec2 targetPoint) const noexcept {
    if (viewport_.size.x <= 0.0F || viewport_.size.y <= 0.0F || !viewport_.contains(targetPoint)) {
        return {{}, false};
    }
    const Vec2 normalized{(targetPoint.x - viewport_.position.x) / viewport_.size.x,
                          (targetPoint.y - viewport_.position.y) / viewport_.size.y};
    return {{normalized.x * contentSize_.x, normalized.y * contentSize_.y}, true};
}

void Context::beginFrame(const InputState& input, const Canvas& canvas) noexcept {
    const auto mapped = canvas.mapToCanvas(input.mousePosition());
    beginFrame(mapped.position, mapped.inside, input.mouseButtonDown(MouseButton::left),
               input.mouseButtonPressed(MouseButton::left), input.mouseButtonReleased(MouseButton::left),
               input.keyPressed(Key::enter) || input.keyPressed(Key::space));
}

void Context::beginFrame(Vec2 pointerPosition, bool pointerInside, bool pointerDown,
                         bool pointerPressed, bool pointerReleased, bool keyboardActivate) noexcept {
    pointerPosition_ = pointerPosition;
    pointerInside_ = pointerInside;
    pointerDown_ = pointerDown;
    pointerPressed_ = pointerPressed;
    pointerReleased_ = pointerReleased;
    keyboardActivate_ = keyboardActivate;
    inputConsumed_ = false;
}

ButtonResult Context::button(WidgetId id, Rect bounds, bool enabled) noexcept {
    const bool hovered = pointerInside_ && bounds.contains(pointerPosition_);
    const bool focused = keyboardFocus_ == id;
    ButtonResult result;

    if (hovered || activePointer_ == id) {
        result.consumed = true;
        inputConsumed_ = true;
    }
    if (!enabled) {
        result.state = WidgetState::disabled;
        return result;
    }

    if (pointerPressed_ && hovered && activePointer_ == 0) {
        activePointer_ = id;
        keyboardFocus_ = id;
        result.consumed = true;
        inputConsumed_ = true;
    }
    if (pointerReleased_ && activePointer_ == id) {
        result.activated = hovered;
        result.consumed = true;
        inputConsumed_ = true;
    }
    if (focused && keyboardActivate_) {
        result.activated = true;
        result.consumed = true;
        inputConsumed_ = true;
    }

    if (activePointer_ == id && pointerDown_ && hovered) result.state = WidgetState::pressed;
    else if (hovered) result.state = WidgetState::hovered;
    else if (focused) result.state = WidgetState::focused;
    return result;
}

void Context::endFrame() noexcept {
    if (pointerReleased_ || (!pointerDown_ && !pointerPressed_)) activePointer_ = 0;
}

} // namespace mf::ui
