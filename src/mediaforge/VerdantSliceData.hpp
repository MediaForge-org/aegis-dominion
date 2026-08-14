#pragma once

#include "render/RenderSnapshot.hpp"

#include <mediaforge/render/RenderQueue2D.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace aegis::mediaforge {

enum class SliceVisualKind : std::uint8_t { environment, shadow, entity, emissive, projectile, decal };

struct SliceVisual {
    std::string logicalId;
    render::RenderVec2 position{};
    render::RenderVec2 size{};
    float rotationRadians{};
    float opacity{1.0F};
    std::int32_t layer{};
    mf::BlendMode blend{mf::BlendMode::alpha};
    SliceVisualKind kind{SliceVisualKind::entity};
};

[[nodiscard]] render::WorldRenderSnapshot makeVerdantSliceSnapshot(float elapsedSeconds);
[[nodiscard]] std::vector<SliceVisual> translateVerdantSlice(const render::WorldRenderSnapshot& snapshot);
[[nodiscard]] std::vector<SliceVisual> deterministicVerdantDetails(unsigned seed);

} // namespace aegis::mediaforge
