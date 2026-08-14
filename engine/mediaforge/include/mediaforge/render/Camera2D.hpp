#pragma once

#include <mediaforge/math/Math.hpp>

#include <cstdint>

namespace mf {

struct Viewport {
    float x{};
    float y{};
    float width{};
    float height{};
    friend constexpr bool operator==(Viewport, Viewport) noexcept = default;
};

class Camera2D {
public:
    void setCenter(Vec2 center) noexcept { center_ = center; }
    void setOrthographicSize(Vec2 size) noexcept;
    void setZoom(float zoom) noexcept;
    void setViewport(Viewport viewport) noexcept;

    [[nodiscard]] Vec2 center() const noexcept { return center_; }
    [[nodiscard]] Vec2 orthographicSize() const noexcept { return orthographicSize_; }
    [[nodiscard]] float zoom() const noexcept { return zoom_; }
    [[nodiscard]] Viewport viewport() const noexcept { return viewport_; }
    [[nodiscard]] Vec2 visibleSize() const noexcept;
    [[nodiscard]] Vec2 worldToScreen(Vec2 world) const noexcept;
    [[nodiscard]] Vec2 screenToWorld(Vec2 screen) const noexcept;
    [[nodiscard]] static Viewport letterbox(Vec2 contentSize, Vec2 targetSize) noexcept;

private:
    Vec2 center_{};
    Vec2 orthographicSize_{1600.0F, 900.0F};
    float zoom_{1.0F};
    Viewport viewport_{0.0F, 0.0F, 1600.0F, 900.0F};
};

} // namespace mf
