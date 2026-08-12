#pragma once

#include <chrono>

namespace mf {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::duration<double>;

constexpr float seconds(Duration duration) noexcept {
    return static_cast<float>(duration.count());
}

} // namespace mf
