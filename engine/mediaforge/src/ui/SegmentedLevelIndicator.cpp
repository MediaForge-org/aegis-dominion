#include <mediaforge/ui/SegmentedLevelIndicator.hpp>

#include <algorithm>

namespace mf::ui {
namespace {

void submitSolid(Renderer2D& renderer, Rect bounds, Color color, std::int32_t layer) {
    Sprite2D sprite;
    sprite.position = {bounds.position.x + bounds.size.x * 0.5F,
                       bounds.position.y + bounds.size.y * 0.5F};
    sprite.size = bounds.size;
    sprite.color = color;
    sprite.blend = BlendMode::alpha;
    sprite.layer = layer;
    sprite.space = CoordinateSpace::screen;
    renderer.submit(sprite);
}

Color accented(Color base, Color accent, float amount) noexcept {
    return {std::clamp(base.r + accent.r * amount, 0.0F, 1.0F),
            std::clamp(base.g + accent.g * amount, 0.0F, 1.0F),
            std::clamp(base.b + accent.b * amount, 0.0F, 1.0F),
            std::clamp(base.a + accent.a * amount, 0.0F, 1.0F)};
}

} // namespace

SegmentedLevelIndicatorLayout segmentedLevelIndicatorLayout(
    Rect bounds, const SegmentedLevelIndicator& indicator,
    const SegmentedLevelIndicatorStyle& style) noexcept {
    SegmentedLevelIndicatorLayout result;
    result.availableBounds = bounds;
    result.segmentCount = indicator.segmentCount;
    result.gap = std::max(style.gap, 0.0F);
    result.y = bounds.position.y;
    if (indicator.segmentCount == 0 || bounds.size.x <= 0.0F || bounds.size.y <= 0.0F) return result;

    const float totalGap = result.gap * static_cast<float>(indicator.segmentCount - 1);
    const float availableForSegments = std::max(bounds.size.x - totalGap, 0.0F);
    result.segmentWidth = std::min(std::max(style.maximumSegmentWidth, 0.0F),
                                   availableForSegments / static_cast<float>(indicator.segmentCount));
    const float usedWidth = result.segmentWidth * static_cast<float>(indicator.segmentCount) + totalGap;
    result.startX = bounds.position.x + (bounds.size.x - usedWidth) * 0.5F;
    const float padding = std::max(style.backgroundPadding, 0.0F);
    result.backgroundBounds = {{result.startX - padding, bounds.position.y - padding},
                               {usedWidth + padding * 2.0F, bounds.size.y + padding * 2.0F}};
    return result;
}

Color segmentedLevelIndicatorSegmentColor(
    const SegmentedLevelIndicator& indicator, std::size_t segmentIndex,
    WidgetState state, float changePulse,
    const SegmentedLevelIndicatorStyle& style) noexcept {
    const bool active = indicator.segmentActive(segmentIndex);
    Color color = active ? style.active : style.inactive;
    const bool highlighted = state == WidgetState::hovered || state == WidgetState::focused;
    color = accented(color, style.interactionAccent, highlighted ? 1.0F : 0.0F);
    if (active) color = accented(color, style.pulseAccent, std::clamp(changePulse, 0.0F, 1.0F));
    return color;
}

void submitSegmentedLevelIndicator(Renderer2D& renderer, Rect bounds,
                                   const SegmentedLevelIndicator& indicator,
                                   WidgetState state, float changePulse,
                                   const SegmentedLevelIndicatorStyle& style,
                                   std::int32_t layer) {
    bounds.size.y = std::max(style.height, 0.0F);
    const auto layout = segmentedLevelIndicatorLayout(bounds, indicator, style);
    if (layout.segmentCount == 0 || layout.segmentWidth <= 0.0F) return;

    submitSolid(renderer, layout.backgroundBounds, style.background, layer);
    for (std::size_t index = 0; index < indicator.segmentCount; ++index) {
        submitSolid(renderer, layout.segmentBounds(index),
                    segmentedLevelIndicatorSegmentColor(indicator, index, state, changePulse, style),
                    layer + 1);
    }
}

} // namespace mf::ui
