#pragma once

#include <mediaforge/platform/Window.hpp>
#include <mediaforge/render/GPUDevice.hpp>

#include <SDL3/SDL.h>

namespace mf::detail {

class NativeAccess {
public:
    static SDL_Window* window(const Window& value) noexcept;
    static SDL_GPUDevice* device(const GPUDevice& value) noexcept;
    static SDL_GPUBuffer* buffer(const Buffer& value) noexcept;
    static SDL_GPUTexture* texture(const Texture& value) noexcept;
    static SDL_GPUSampler* sampler(const Sampler& value) noexcept;
    static SDL_GPUGraphicsPipeline* pipeline(const GraphicsPipeline& value) noexcept;
};

} // namespace mf::detail
