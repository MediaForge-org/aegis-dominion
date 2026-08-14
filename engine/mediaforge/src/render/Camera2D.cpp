#include <mediaforge/render/Camera2D.hpp>

#include <algorithm>

namespace mf {

void Camera2D::setOrthographicSize(Vec2 size) noexcept {
    orthographicSize_ = {std::max(size.x, 0.001F), std::max(size.y, 0.001F)};
}

void Camera2D::setZoom(float zoomValue) noexcept { zoom_ = std::max(zoomValue, 0.001F); }

void Camera2D::setViewport(Viewport viewportValue) noexcept {
    viewport_ = viewportValue;
    viewport_.width = std::max(viewport_.width, 0.001F);
    viewport_.height = std::max(viewport_.height, 0.001F);
}

Vec2 Camera2D::visibleSize() const noexcept { return orthographicSize_ * (1.0F / zoom_); }

Vec2 Camera2D::worldToScreen(Vec2 world) const noexcept {
    const Vec2 visible = visibleSize();
    const Vec2 normalized{(world.x - center_.x) / visible.x + 0.5F,
                          (world.y - center_.y) / visible.y + 0.5F};
    return {viewport_.x + normalized.x * viewport_.width,
            viewport_.y + normalized.y * viewport_.height};
}

Vec2 Camera2D::screenToWorld(Vec2 screen) const noexcept {
    const Vec2 visible = visibleSize();
    const Vec2 normalized{(screen.x - viewport_.x) / viewport_.width - 0.5F,
                          (screen.y - viewport_.y) / viewport_.height - 0.5F};
    return {center_.x + normalized.x * visible.x, center_.y + normalized.y * visible.y};
}

Viewport Camera2D::letterbox(Vec2 contentSize, Vec2 targetSize) noexcept {
    if (contentSize.x <= 0.0F || contentSize.y <= 0.0F || targetSize.x <= 0.0F || targetSize.y <= 0.0F) {
        return {};
    }
    const float scaleFactor = std::min(targetSize.x / contentSize.x, targetSize.y / contentSize.y);
    const Vec2 fitted = contentSize * scaleFactor;
    return {(targetSize.x - fitted.x) * 0.5F, (targetSize.y - fitted.y) * 0.5F, fitted.x, fitted.y};
}

} // namespace mf
