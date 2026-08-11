#include "EditorCamera.hpp"

#include <algorithm>
#include <cmath>

namespace aegis::editor {

void EditorCamera::fit(float mapWidth, float mapHeight) {
    const float safeWidth = std::max(1.f, mapWidth);
    const float safeHeight = std::max(1.f, mapHeight);
    baseScale_ = std::min((viewport_.width - 48.f) / safeWidth, (viewport_.height - 48.f) / safeHeight);
    scale_ = std::max(.05f, baseScale_);
    center_ = {mapWidth / 2.f, mapHeight / 2.f};
}

void EditorCamera::reset(float mapWidth, float mapHeight) { fit(mapWidth, mapHeight); }

void EditorCamera::panScreen(sf::Vector2f screenDelta) {
    center_.x -= screenDelta.x / scale_;
    center_.y -= screenDelta.y / scale_;
}

void EditorCamera::zoomAt(float wheelDelta, sf::Vector2f screenPosition) {
    const auto before = screenToWorld(screenPosition);
    const float factor = std::pow(1.15f, wheelDelta);
    scale_ = std::clamp(scale_ * factor, baseScale_ * .25f, baseScale_ * 6.f);
    const auto after = screenToWorld(screenPosition);
    center_.x += before.x - after.x;
    center_.y += before.y - after.y;
}

core::Vec2 EditorCamera::screenToWorld(sf::Vector2f screen) const {
    const sf::Vector2f viewportCenter(viewport_.left + viewport_.width / 2.f, viewport_.top + viewport_.height / 2.f);
    return {center_.x + (screen.x - viewportCenter.x) / scale_, center_.y + (screen.y - viewportCenter.y) / scale_};
}

sf::Vector2f EditorCamera::worldToScreen(core::Vec2 world) const {
    const sf::Vector2f viewportCenter(viewport_.left + viewport_.width / 2.f, viewport_.top + viewport_.height / 2.f);
    return {viewportCenter.x + (world.x - center_.x) * scale_, viewportCenter.y + (world.y - center_.y) * scale_};
}

} // namespace aegis::editor
