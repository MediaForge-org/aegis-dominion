#pragma once

#include "core/MapDocument.hpp"

#include <SFML/Graphics.hpp>

namespace aegis::editor {

class EditorCamera {
public:
    void setViewport(sf::FloatRect viewport) { viewport_ = viewport; }
    const sf::FloatRect& viewport() const { return viewport_; }

    void fit(float mapWidth, float mapHeight);
    void reset(float mapWidth, float mapHeight);
    void panScreen(sf::Vector2f screenDelta);
    void zoomAt(float wheelDelta, sf::Vector2f screenPosition);

    core::Vec2 screenToWorld(sf::Vector2f screen) const;
    sf::Vector2f worldToScreen(core::Vec2 world) const;
    float scale() const { return scale_; }
    float zoomPercent() const { return baseScale_ > 0.f ? scale_ / baseScale_ * 100.f : 100.f; }

private:
    sf::FloatRect viewport_{180.f, 74.f, 1040.f, 776.f};
    core::Vec2 center_{600.f, 450.f};
    float scale_ = 1.f;
    float baseScale_ = 1.f;
};

} // namespace aegis::editor
