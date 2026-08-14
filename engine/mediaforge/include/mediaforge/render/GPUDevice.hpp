#pragma once

#include <mediaforge/foundation/Config.hpp>
#include <mediaforge/foundation/Error.hpp>
#include <mediaforge/render/GPUResources.hpp>

#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace mf {

class Window;

enum class PresentMode { vsync, immediate, mailbox };

class GPUDevice {
public:
    GPUDevice() noexcept;
    ~GPUDevice();
    GPUDevice(GPUDevice&&) noexcept;
    GPUDevice& operator=(GPUDevice&&) noexcept;
    GPUDevice(const GPUDevice&) = delete;
    GPUDevice& operator=(const GPUDevice&) = delete;

    [[nodiscard]] static Result<GPUDevice> create(Window& window, const EngineConfig& config = {});
    [[nodiscard]] std::string backendName() const;
    [[nodiscard]] Result<void> setPresentMode(Window& window, PresentMode mode);
    [[nodiscard]] bool supportsPresentMode(const Window& window, PresentMode mode) const noexcept;

    [[nodiscard]] Result<Buffer> createBuffer(BufferUsage usage, std::span<const std::byte> initialData,
                                               std::string debugName = {});
    [[nodiscard]] Result<Texture> createTexture(const TextureDescription& description,
                                                std::span<const std::byte> rgbaPixels);
    [[nodiscard]] Result<Sampler> createSampler(const SamplerDescription& description = {});
    [[nodiscard]] Result<Shader> createShader(const std::filesystem::path& spirvPath,
                                              const ShaderDescription& description);
    [[nodiscard]] Result<GraphicsPipeline> createGraphicsPipeline(
        Window& window, const Shader& vertexShader, const Shader& fragmentShader,
        std::uint32_t vertexStride, std::span<const VertexAttribute> attributes,
        bool alphaBlending = false);

private:
    struct Impl;
    explicit GPUDevice(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class detail::NativeAccess;
};

} // namespace mf
