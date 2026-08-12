#include "RenderContext.hpp"

namespace aegis::render {

void RenderQueue::submit(Layer layer, DrawCall draw) { layers_[static_cast<std::size_t>(layer)].push_back(std::move(draw)); }

void RenderQueue::execute() {
    for (const auto layer : LayerOrder) {
        auto& calls = layers_[static_cast<std::size_t>(layer)];
        for (auto& call : calls) call();
        calls.clear();
    }
}

std::size_t RenderQueue::size() const {
    std::size_t result = 0;
    for (const auto& layer : layers_) result += layer.size();
    return result;
}

} // namespace aegis::render
