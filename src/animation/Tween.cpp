#include "Tween.hpp"

namespace aegis::animation {

float Tween::value() const {
    float t = std::clamp(elapsed_ / duration_, 0.f, 1.f);
    if (easing_ == Easing::EaseOut) t = 1.f - (1.f - t) * (1.f - t);
    else if (easing_ == Easing::SmoothStep) t = t * t * (3.f - 2.f * t);
    if (reverse_) t = 1.f - t;
    return from_ + (to_ - from_) * t;
}

} // namespace aegis::animation
