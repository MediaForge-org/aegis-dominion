#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <functional>
#include <vector>

namespace aegis::render {

enum class Layer {
    Terrain, Water, Road, Environment, Zones, Enemies, Towers, Projectiles,
    Effects, WorldUi, ScreenUi, ModalUi, Count
};

constexpr std::array<Layer, static_cast<std::size_t>(Layer::Count)> LayerOrder = {
    Layer::Terrain, Layer::Water, Layer::Road, Layer::Environment, Layer::Zones,
    Layer::Enemies, Layer::Towers, Layer::Projectiles, Layer::Effects,
    Layer::WorldUi, Layer::ScreenUi, Layer::ModalUi};

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

class WorldRenderer {
public:
    explicit WorldRenderer(RenderContext& context) : context_(context) {}
    void begin(Layer layer) { context_.setLayer(layer); }
private:
    RenderContext& context_;
};

} // namespace aegis::render
