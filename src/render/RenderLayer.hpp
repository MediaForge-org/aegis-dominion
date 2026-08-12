#pragma once

#include <array>
#include <cstddef>

namespace aegis::render {

enum class Layer {
    Terrain, Water, Road, Environment, Zones, Enemies, Towers, Projectiles,
    Effects, WorldUi, ScreenUi, ModalUi, Count
};

constexpr std::array<Layer, static_cast<std::size_t>(Layer::Count)> LayerOrder = {
    Layer::Terrain, Layer::Water, Layer::Road, Layer::Environment, Layer::Zones,
    Layer::Enemies, Layer::Towers, Layer::Projectiles, Layer::Effects,
    Layer::WorldUi, Layer::ScreenUi, Layer::ModalUi};

} // namespace aegis::render
