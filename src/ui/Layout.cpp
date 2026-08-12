#include "Layout.hpp"

#include <algorithm>

namespace aegis::ui {

std::vector<sf::FloatRect> LinearLayout::calculate(const sf::FloatRect& bounds, std::span<const LayoutItem> items) const {
    std::vector<sf::FloatRect> result;
    if (items.empty()) return result;
    const float mainSize = axis_ == Axis::Horizontal ? bounds.width : bounds.height;
    const float crossSize = axis_ == Axis::Horizontal ? bounds.height : bounds.width;
    const float totalGap = gap_ * static_cast<float>(items.size() - 1);
    float fixed = 0.f, flex = 0.f;
    for (const auto& item : items) { fixed += std::max(0.f, item.fixed); flex += std::max(0.f, item.flex); }
    const float available = std::max(0.f, mainSize - padding_ * 2.f - totalGap - fixed);
    float cursor = (axis_ == Axis::Horizontal ? bounds.left : bounds.top) + padding_;
    for (const auto& item : items) {
        const float itemMain = item.fixed > 0.f ? item.fixed : (flex > 0.f ? available * item.flex / flex : 0.f);
        const float itemCross = alignment_ == Alignment::Stretch ? std::max(0.f, crossSize - padding_ * 2.f) : 0.f;
        const float crossStart = (axis_ == Axis::Horizontal ? bounds.top : bounds.left) + padding_;
        if (axis_ == Axis::Horizontal) result.emplace_back(cursor, crossStart, itemMain, itemCross);
        else result.emplace_back(crossStart, cursor, itemCross, itemMain);
        cursor += itemMain + gap_;
    }
    return result;
}

void UiScale::update(unsigned width, unsigned height) {
    factor_ = std::min(static_cast<float>(width) / ReferenceWidth, static_cast<float>(height) / ReferenceHeight);
    const float usedWidth = ReferenceWidth * factor_, usedHeight = ReferenceHeight * factor_;
    viewport_ = {(static_cast<float>(width) - usedWidth) * .5f, (static_cast<float>(height) - usedHeight) * .5f, usedWidth, usedHeight};
}

} // namespace aegis::ui
