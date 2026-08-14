#include <mediaforge/foundation/Config.hpp>
#include <mediaforge/foundation/Log.hpp>
#include <mediaforge/foundation/Version.hpp>
#include <mediaforge/input/Event.hpp>
#include <mediaforge/input/InputState.hpp>
#include <mediaforge/platform/Window.hpp>
#include <mediaforge/render/GPUDevice.hpp>

#include "../render/NativeAccess.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <span>
#include <string_view>

namespace {

struct Vertex {
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
};

template <typename T, std::size_t Size>
std::span<const std::byte> bytes(const std::array<T, Size>& values) {
    return std::as_bytes(std::span(values));
}

[[maybe_unused]] int report(const mf::Error& error) {
    std::cerr << "MediaForge GPU smoke failed: " << error.message << '\n';
    return 1;
}

} // namespace

int main() {
#if !MEDIAFORGE_SMOKE_SHADERS_AVAILABLE
    std::cerr << "mediaforge_gpu_smoke was built without compiled shaders. Install glslc "
                 "2026.1 and rebuild this target.\n";
    return 2;
#else
    mf::log(mf::LogLevel::info, std::string("MediaForge Engine startup: ") + std::string(mf::versionString));
    const int runtimeVersion = SDL_GetVersion();
    mf::log(mf::LogLevel::info, "SDL runtime version: " + std::to_string(SDL_VERSIONNUM_MAJOR(runtimeVersion)) + "." +
        std::to_string(SDL_VERSIONNUM_MINOR(runtimeVersion)) + "." + std::to_string(SDL_VERSIONNUM_MICRO(runtimeVersion)));

    auto windowResult = mf::Window::create({"MediaForge E1 GPU Smoke", 1280, 720, true, true});
    if (!windowResult) { return report(windowResult.error()); }
    mf::Window window = std::move(*windowResult);

    mf::EngineConfig config{"MediaForge GPU Smoke", mf::GpuBackendPreference::preferVulkan,
#ifndef NDEBUG
        true
#else
        false
#endif
    };
    auto deviceResult = mf::GPUDevice::create(window, config);
    if (!deviceResult) { return report(deviceResult.error()); }
    mf::GPUDevice device = std::move(*deviceResult);

    constexpr std::array triangle{
        Vertex{-0.85F, 0.72F, 0.15F, 0.85F, 1.0F, 1.0F, 0.0F, 0.0F},
        Vertex{-0.25F, 0.72F, 0.70F, 0.30F, 1.0F, 1.0F, 0.0F, 0.0F},
        Vertex{-0.55F, 0.05F, 0.20F, 1.00F, 0.45F, 1.0F, 0.0F, 0.0F},
    };
    constexpr std::array quad{
        Vertex{0.05F, 0.65F, 1, 1, 1, 0.92F, 0, 0}, Vertex{0.80F, 0.65F, 1, 1, 1, 0.92F, 1, 0},
        Vertex{0.80F,-0.45F, 1, 1, 1, 0.92F, 1, 1}, Vertex{0.05F, 0.65F, 1, 1, 1, 0.92F, 0, 0},
        Vertex{0.80F,-0.45F, 1, 1, 1, 0.92F, 1, 1}, Vertex{0.05F,-0.45F, 1, 1, 1, 0.92F, 0, 1},
    };
    constexpr std::array<std::uint8_t, 4 * 4 * 4> checker{
        35,45,62,255, 32,210,190,255, 35,45,62,255, 32,210,190,255,
        32,210,190,255, 35,45,62,255, 32,210,190,255, 35,45,62,255,
        35,45,62,255, 32,210,190,255, 35,45,62,255, 32,210,190,255,
        32,210,190,255, 35,45,62,255, 32,210,190,255, 35,45,62,255,
    };
    const std::array attributes{
        mf::VertexAttribute{0, mf::VertexFormat::float2, offsetof(Vertex, x)},
        mf::VertexAttribute{1, mf::VertexFormat::float4, offsetof(Vertex, r)},
        mf::VertexAttribute{2, mf::VertexFormat::float2, offsetof(Vertex, u)},
    };

    auto triangleBufferResult = device.createBuffer(mf::BufferUsage::vertex, bytes(triangle), "SmokeTriangleVertices");
    auto quadBufferResult = device.createBuffer(mf::BufferUsage::vertex, bytes(quad), "TexturedQuadVertices");
    auto textureResult = device.createTexture({4, 4, "SmokeCheckerTexture"}, std::as_bytes(std::span(checker)));
    auto samplerResult = device.createSampler({mf::FilterMode::nearest, mf::WrapMode::repeat, mf::WrapMode::repeat,
                                                "SmokeNearestSampler"});
    if (!triangleBufferResult) return report(triangleBufferResult.error());
    if (!quadBufferResult) return report(quadBufferResult.error());
    if (!textureResult) return report(textureResult.error());
    if (!samplerResult) return report(samplerResult.error());
    mf::Buffer triangleBuffer = std::move(*triangleBufferResult);
    mf::Buffer quadBuffer = std::move(*quadBufferResult);
    mf::Texture texture = std::move(*textureResult);
    mf::Sampler sampler = std::move(*samplerResult);

    const std::filesystem::path shaderDirectory = MEDIAFORGE_SHADER_DIRECTORY;
    auto vertexShaderResult = device.createShader(shaderDirectory / "smoke.vert.spv", {mf::ShaderStage::vertex, 0, "SmokeVertex"});
    auto colorShaderResult = device.createShader(shaderDirectory / "color.frag.spv", {mf::ShaderStage::fragment, 0, "ColorFragment"});
    auto spriteShaderResult = device.createShader(shaderDirectory / "sprite.frag.spv", {mf::ShaderStage::fragment, 1, "SpriteFragment"});
    if (!vertexShaderResult) return report(vertexShaderResult.error());
    if (!colorShaderResult) return report(colorShaderResult.error());
    if (!spriteShaderResult) return report(spriteShaderResult.error());
    mf::Shader vertexShader = std::move(*vertexShaderResult);
    mf::Shader colorShader = std::move(*colorShaderResult);
    mf::Shader spriteShader = std::move(*spriteShaderResult);

    auto trianglePipelineResult = device.createGraphicsPipeline(window, vertexShader, colorShader, sizeof(Vertex), attributes);
    auto spritePipelineResult = device.createGraphicsPipeline(window, vertexShader, spriteShader, sizeof(Vertex), attributes, true);
    if (!trianglePipelineResult) return report(trianglePipelineResult.error());
    if (!spritePipelineResult) return report(spritePipelineResult.error());
    mf::GraphicsPipeline trianglePipeline = std::move(*trianglePipelineResult);
    mf::GraphicsPipeline spritePipeline = std::move(*spritePipelineResult);

    mf::EventPump eventPump;
    mf::InputState input;
    while (!window.closeRequested()) {
        input.beginFrame();
        for (const auto& event : eventPump.poll()) {
            input.consume(event);
            if (std::holds_alternative<mf::QuitEvent>(event)) { window.requestClose(); }
        }
        if (input.keyPressed(mf::Key::escape)) { window.requestClose(); }
        if (window.closeRequested()) { break; }

        SDL_GPUDevice* nativeDevice = mf::detail::NativeAccess::device(device);
        SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(nativeDevice);
        if (!commands) { return report({mf::ErrorCode::gpuError, SDL_GetError()}); }
        SDL_PushGPUDebugGroup(commands, "Frame");
        SDL_GPUTexture* swapchain{};
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commands, mf::detail::NativeAccess::window(window), &swapchain, nullptr, nullptr)) {
            SDL_CancelGPUCommandBuffer(commands);
            return report({mf::ErrorCode::gpuError, SDL_GetError()});
        }
        if (swapchain != nullptr) {
            SDL_GPUColorTargetInfo target{};
            target.texture = swapchain;
            target.clear_color = {0.018F, 0.026F, 0.055F, 1.0F};
            target.load_op = SDL_GPU_LOADOP_CLEAR;
            target.store_op = SDL_GPU_STOREOP_STORE;
            SDL_PushGPUDebugGroup(commands, "MainRenderPass");
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(pass, mf::detail::NativeAccess::pipeline(trianglePipeline));
            const SDL_GPUBufferBinding triangleBinding{mf::detail::NativeAccess::buffer(triangleBuffer), 0};
            SDL_BindGPUVertexBuffers(pass, 0, &triangleBinding, 1);
            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

            SDL_InsertGPUDebugLabel(commands, "TexturedQuad");
            SDL_BindGPUGraphicsPipeline(pass, mf::detail::NativeAccess::pipeline(spritePipeline));
            const SDL_GPUBufferBinding quadBinding{mf::detail::NativeAccess::buffer(quadBuffer), 0};
            const SDL_GPUTextureSamplerBinding textureBinding{mf::detail::NativeAccess::texture(texture),
                                                               mf::detail::NativeAccess::sampler(sampler)};
            SDL_BindGPUVertexBuffers(pass, 0, &quadBinding, 1);
            SDL_BindGPUFragmentSamplers(pass, 0, &textureBinding, 1);
            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
            SDL_EndGPURenderPass(pass);
            SDL_PopGPUDebugGroup(commands);
        }
        SDL_PopGPUDebugGroup(commands);
        if (!SDL_SubmitGPUCommandBuffer(commands)) { return report({mf::ErrorCode::gpuError, SDL_GetError()}); }
    }
    return 0;
#endif
}
