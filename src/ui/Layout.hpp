#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <span>
#include <vector>

namespace aegis::ui {

enum class Axis { Horizontal, Vertical };
enum class Alignment { Start, Center, End, Stretch };
struct LayoutItem { float fixed = 0.f; float flex = 0.f; };

class LinearLayout {
public:
    LinearLayout(Axis axis, float gap = 0.f, float padding = 0.f, Alignment alignment = Alignment::Stretch)
        : axis_(axis), gap_(gap), padding_(padding), alignment_(alignment) {}
    std::vector<sf::FloatRect> calculate(const sf::FloatRect& bounds, std::span<const LayoutItem> items) const;

private:
    Axis axis_;
    float gap_;
    float padding_;
    Alignment alignment_;
};

class UiScale {
public:
    static constexpr float ReferenceWidth = 1600.f;
    static constexpr float ReferenceHeight = 900.f;
    void update(unsigned width, unsigned height);
    float factor() const { return factor_; }
    sf::FloatRect viewport() const { return viewport_; }
    float pixels(float referenceValue) const { return referenceValue * factor_; }

private:
    float factor_ = 1.f;
    sf::FloatRect viewport_{0.f, 0.f, ReferenceWidth, ReferenceHeight};
};

} // namespace aegis::ui
