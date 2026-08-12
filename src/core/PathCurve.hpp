#pragma once

#include "MapDocument.hpp"

#include <vector>

namespace aegis::core {

// Samples a logical node path into a deterministic Catmull-Rom curve. Endpoints
// remain exact; the returned points are suitable for both simulation and views.
std::vector<Vec2> samplePathCurve(const std::vector<Vec2>& nodes, float targetSpacing = 14.f);

} // namespace aegis::core
