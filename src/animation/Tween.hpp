#pragma once

#include <algorithm>
#include <cmath>

namespace aegis::animation {

enum class Easing { Linear, EaseOut, SmoothStep };

class Tween {
public:
    Tween(float from = 0.f, float to = 1.f, float duration = .2f, Easing easing = Easing::EaseOut)
        : from_(from), to_(to), duration_(std::max(.001f, duration)), easing_(easing) {}
    void restart(bool reverse = false) { elapsed_ = 0.f; reverse_ = reverse; running_ = true; }
    void update(float deltaSeconds) { if (running_) { elapsed_ = std::min(duration_, elapsed_ + std::max(0.f, deltaSeconds)); running_ = elapsed_ < duration_; } }
    float value() const;
    bool finished() const { return !running_; }

private:
    float from_, to_, duration_, elapsed_ = 0.f;
    Easing easing_;
    bool reverse_ = false, running_ = false;
};

} // namespace aegis::animation
