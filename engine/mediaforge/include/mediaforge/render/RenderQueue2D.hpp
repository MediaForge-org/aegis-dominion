#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace mf {

enum class BlendMode : std::uint8_t { opaque, alpha, additive };

struct SubmissionState2D {
    std::int32_t layer{};
    std::int32_t order{};
    std::uint64_t pipeline{};
    std::uint64_t texture{};
    BlendMode blend{BlendMode::alpha};
    std::uint64_t sequence{};
};

struct Batch2D {
    std::size_t first{};
    std::size_t count{};
    SubmissionState2D state{};
};

struct BatchPlan2D {
    std::vector<std::size_t> orderedIndices;
    std::vector<Batch2D> batches;
};

[[nodiscard]] BatchPlan2D buildBatchPlan(std::span<const SubmissionState2D> submissions);
void buildBatchPlan(std::span<const SubmissionState2D> submissions, BatchPlan2D& reusablePlan);

} // namespace mf
