#include <mediaforge/render/RenderQueue2D.hpp>

#include <algorithm>
#include <numeric>

namespace mf {
namespace {

bool compatible(const SubmissionState2D& left, const SubmissionState2D& right) noexcept {
    return left.layer == right.layer && left.order == right.order && left.pipeline == right.pipeline &&
           left.texture == right.texture && left.blend == right.blend;
}

} // namespace

BatchPlan2D buildBatchPlan(std::span<const SubmissionState2D> submissions) {
    BatchPlan2D result;
    buildBatchPlan(submissions, result);
    return result;
}

void buildBatchPlan(std::span<const SubmissionState2D> submissions, BatchPlan2D& result) {
    result.orderedIndices.resize(submissions.size());
    result.batches.clear();
    result.batches.reserve(submissions.size());
    std::iota(result.orderedIndices.begin(), result.orderedIndices.end(), std::size_t{});
    std::sort(result.orderedIndices.begin(), result.orderedIndices.end(), [&](std::size_t left, std::size_t right) {
        const auto& a = submissions[left];
        const auto& b = submissions[right];
        if (a.layer != b.layer) return a.layer < b.layer;
        if (a.order != b.order) return a.order < b.order;
        return a.sequence < b.sequence;
    });
    std::size_t cursor{};
    while (cursor < result.orderedIndices.size()) {
        const auto& first = submissions[result.orderedIndices[cursor]];
        std::size_t groupEnd = cursor + 1;
        while (groupEnd < result.orderedIndices.size()) {
            const auto& candidate = submissions[result.orderedIndices[groupEnd]];
            if (candidate.layer != first.layer || candidate.order != first.order) break;
            ++groupEnd;
        }
        std::size_t runStart = cursor;
        while (runStart < groupEnd) {
            while (runStart < groupEnd && submissions[result.orderedIndices[runStart]].blend == BlendMode::alpha) ++runStart;
            std::size_t runEnd = runStart;
            while (runEnd < groupEnd && submissions[result.orderedIndices[runEnd]].blend != BlendMode::alpha) ++runEnd;
            std::sort(result.orderedIndices.begin() + static_cast<std::ptrdiff_t>(runStart),
                      result.orderedIndices.begin() + static_cast<std::ptrdiff_t>(runEnd), [&](std::size_t left, std::size_t right) {
                const auto& a = submissions[left];
                const auto& b = submissions[right];
                if (a.pipeline != b.pipeline) return a.pipeline < b.pipeline;
                if (a.texture != b.texture) return a.texture < b.texture;
                if (a.blend != b.blend) return a.blend < b.blend;
                return a.sequence < b.sequence;
            });
            runStart = runEnd;
        }
        cursor = groupEnd;
    }
    for (std::size_t sorted = 0; sorted < result.orderedIndices.size(); ++sorted) {
        const auto& state = submissions[result.orderedIndices[sorted]];
        if (result.batches.empty() || !compatible(result.batches.back().state, state)) {
            result.batches.push_back({sorted, 1, state});
        } else {
            ++result.batches.back().count;
        }
    }
}

} // namespace mf
