#include "PathCurve.hpp"

#include <algorithm>
#include <cmath>

namespace aegis::core {
namespace {

Vec2 interpolate(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return {
        .5f * ((2.f * p1.x) + (-p0.x + p2.x) * t + (2.f * p0.x - 5.f * p1.x + 4.f * p2.x - p3.x) * t2 + (-p0.x + 3.f * p1.x - 3.f * p2.x + p3.x) * t3),
        .5f * ((2.f * p1.y) + (-p0.y + p2.y) * t + (2.f * p0.y - 5.f * p1.y + 4.f * p2.y - p3.y) * t2 + (-p0.y + 3.f * p1.y - 3.f * p2.y + p3.y) * t3)};
}

float distance(Vec2 a, Vec2 b) {
    const float x = b.x - a.x;
    const float y = b.y - a.y;
    return std::sqrt(x * x + y * y);
}

} // namespace

std::vector<Vec2> samplePathCurve(const std::vector<Vec2>& nodes, float targetSpacing) {
    if (nodes.size() < 2) return nodes;
    targetSpacing = std::max(2.f, targetSpacing);
    std::vector<Vec2> result;
    result.reserve(nodes.size() * 8);
    result.push_back(nodes.front());
    for (std::size_t i = 0; i + 1 < nodes.size(); ++i) {
        const Vec2 p0 = i == 0 ? nodes[i] : nodes[i - 1];
        const Vec2 p1 = nodes[i];
        const Vec2 p2 = nodes[i + 1];
        const Vec2 p3 = i + 2 < nodes.size() ? nodes[i + 2] : nodes[i + 1];
        const int subdivisions = std::max(2, static_cast<int>(std::ceil(distance(p1, p2) / targetSpacing)));
        for (int step = 1; step <= subdivisions; ++step)
            result.push_back(interpolate(p0, p1, p2, p3, static_cast<float>(step) / static_cast<float>(subdivisions)));
    }
    result.front() = nodes.front();
    result.back() = nodes.back();
    return result;
}

} // namespace aegis::core
