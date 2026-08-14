#pragma once

#include <mediaforge/render/Renderer2D.hpp>
#include <mediaforge/ui/Ui.hpp>

#include <cstddef>
#include <cstdint>

namespace mf::ui {

struct SegmentedLevelIndicator {
    std::size_t segmentCount{};
    std::size_t currentIndex{};

    [[nodiscard]] constexpr std::size_t activeSegmentCount() const noexcept {
        return segmentCount == 0 ? 0 : (currentIndex < segmentCount ? currentIndex : segmentCount - 1) + 1;
    }

    [[nodiscard]] constexpr bool segmentActive(std::size_t index) const noexcept {
        return index < segmentCount && index < activeSegmentCount();
    }
};

struct SegmentedLevelIndicatorStyle {
    float gap{8.0F};
    float height{7.0F};
    float maximumSegmentWidth{50.0F};
    float backgroundPadding{4.0F};
    Color background{0.015F, 0.055F, 0.068F, 0.90F};
    Color active{0.08F, 0.64F, 0.72F, 0.96F};
    Color inactive{0.16F, 0.25F, 0.28F, 0.88F};
    Color interactionAccent{0.03F, 0.08F, 0.09F, 0.0F};
    Color pulseAccent{0.10F, 0.18F, 0.20F, 0.0F};
};

struct SegmentedLevelIndicatorLayout {
    Rect availableBounds{};
    Rect backgroundBounds{};
    float segmentWidth{};
    float gap{};
    float startX{};
    float y{};
    std::size_t segmentCount{};

    [[nodiscard]] constexpr Rect segmentBounds(std::size_t index) const noexcept {
        if (index >= segmentCount) return {};
        return {{startX + static_cast<float>(index) * (segmentWidth + gap), y},
                {segmentWidth, availableBounds.size.y}};
    }
};

[[nodiscard]] SegmentedLevelIndicatorLayout segmentedLevelIndicatorLayout(
    Rect bounds, const SegmentedLevelIndicator& indicator,
    const SegmentedLevelIndicatorStyle& style = {}) noexcept;

[[nodiscard]] Color segmentedLevelIndicatorSegmentColor(
    const SegmentedLevelIndicator& indicator, std::size_t segmentIndex,
    WidgetState state, float changePulse,
    const SegmentedLevelIndicatorStyle& style = {}) noexcept;

void submitSegmentedLevelIndicator(Renderer2D& renderer, Rect bounds,
                                   const SegmentedLevelIndicator& indicator,
                                   WidgetState state, float changePulse,
                                   const SegmentedLevelIndicatorStyle& style = {},
                                   std::int32_t layer = 0);

} // namespace mf::ui
