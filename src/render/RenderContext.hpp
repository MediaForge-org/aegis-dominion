#pragma once

#include "RenderLayer.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <functional>
#include <vector>

namespace aegis::render {

class RenderContext {
public:
    explicit RenderContext(sf::RenderTarget& target) : target_(target) {}
    sf::RenderTarget& target() { return target_; }
    Layer layer() const { return layer_; }
    void setLayer(Layer layer) { layer_ = layer; }

private:
    sf::RenderTarget& target_;
    Layer layer_ = Layer::Terrain;
};

class RenderQueue {
public:
    using DrawCall = std::function<void()>;
    void submit(Layer layer, DrawCall draw);
    void execute();
    std::size_t size() const;

private:
    std::array<std::vector<DrawCall>, static_cast<std::size_t>(Layer::Count)> layers_;
};

} // namespace aegis::render
