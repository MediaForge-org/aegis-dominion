#pragma once

#include <mediaforge/foundation/Error.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace mf {

namespace detail { class NativeAccess; }

enum class BufferUsage { vertex, index };
enum class ShaderStage { vertex, fragment };
enum class VertexFormat { float2, float4 };

struct TextureDescription {
    std::uint32_t width{};
    std::uint32_t height{};
    std::string debugName;
};

struct ShaderDescription {
    ShaderStage stage{ShaderStage::vertex};
    std::uint32_t samplerCount{};
    std::string debugName;
};

struct VertexAttribute {
    std::uint32_t location{};
    VertexFormat format{VertexFormat::float2};
    std::uint32_t offset{};
};

class Buffer {
public:
    Buffer() noexcept;
    ~Buffer();
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
private:
    struct Impl;
    explicit Buffer(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

class Texture {
public:
    Texture() noexcept;
    ~Texture();
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
private:
    struct Impl;
    explicit Texture(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

class Sampler {
public:
    Sampler() noexcept;
    ~Sampler();
    Sampler(Sampler&&) noexcept;
    Sampler& operator=(Sampler&&) noexcept;
    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
private:
    struct Impl;
    explicit Sampler(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

class Shader {
public:
    Shader() noexcept;
    ~Shader();
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
private:
    struct Impl;
    explicit Shader(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

class GraphicsPipeline {
public:
    GraphicsPipeline() noexcept;
    ~GraphicsPipeline();
    GraphicsPipeline(GraphicsPipeline&&) noexcept;
    GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
private:
    struct Impl;
    explicit GraphicsPipeline(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

} // namespace mf
