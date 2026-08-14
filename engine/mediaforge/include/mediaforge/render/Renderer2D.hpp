#pragma once

#include <mediaforge/foundation/Error.hpp>
#include <mediaforge/math/Math.hpp>
#include <mediaforge/render/Camera2D.hpp>
#include <mediaforge/render/GPUDevice.hpp>
#include <mediaforge/render/RenderQueue2D.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace mf {

class Window;

struct Color {
    float r{1.0F};
    float g{1.0F};
    float b{1.0F};
    float a{1.0F};
    friend constexpr bool operator==(Color, Color) noexcept = default;
};

struct UvRegion {
    float left{};
    float top{};
    float right{1.0F};
    float bottom{1.0F};
};

enum class CoordinateSpace : std::uint8_t { world, screen };

struct Sprite2D {
    const Texture* texture{};
    const Sampler* sampler{};
    Vec2 position{};
    Vec2 size{1.0F, 1.0F};
    Vec2 scale{1.0F, 1.0F};
    Vec2 pivot{0.5F, 0.5F};
    float rotationRadians{};
    UvRegion uv{};
    Color color{};
    float opacity{1.0F};
    BlendMode blend{BlendMode::alpha};
    std::int32_t layer{};
    std::int32_t order{};
    CoordinateSpace space{CoordinateSpace::world};
};

[[nodiscard]] bool spriteBoundsVisible(Vec2 screenPosition, Vec2 screenSize, Vec2 pivot,
                                       float rotationRadians, Viewport viewport) noexcept;

struct GeometryVertex2D {
    Vec2 position{};
    Color color{};
    Vec2 uv{};
};

struct Geometry2D {
    std::span<const GeometryVertex2D> vertices;
    const Texture* texture{};
    const Sampler* sampler{};
    BlendMode blend{BlendMode::alpha};
    std::int32_t layer{};
    std::int32_t order{};
    CoordinateSpace space{CoordinateSpace::world};
};

struct RenderTargetDescription {
    enum class Format : std::uint8_t { rgba16Float, rgba8Unorm };
    std::uint32_t width{};
    std::uint32_t height{};
    std::string debugName;
    Format format{Format::rgba16Float};
};

[[nodiscard]] bool validRenderTargetDescription(const RenderTargetDescription& description) noexcept;
[[nodiscard]] std::uint64_t approximateRenderTargetBytes(const RenderTargetDescription& description) noexcept;

class RenderTarget {
public:
    RenderTarget() noexcept;
    ~RenderTarget();
    RenderTarget(RenderTarget&&) noexcept;
    RenderTarget& operator=(RenderTarget&&) noexcept;
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;
    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] std::uint32_t width() const noexcept;
    [[nodiscard]] std::uint32_t height() const noexcept;
    [[nodiscard]] RenderTargetDescription::Format format() const noexcept;
    [[nodiscard]] std::uint64_t approximateMemoryBytes() const noexcept;

private:
    struct Impl;
    explicit RenderTarget(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
    friend class Renderer2D;
};

struct RenderPass2D {
    RenderTarget* target{};
    Camera2D camera{};
    Color clearColor{0.0F, 0.0F, 0.0F, 0.0F};
    bool clear{true};
    std::string_view label;
};

struct PostProcessSettings {
    float bloomStrength{0.42F};
    float bloomRadius{1.2F};
    float vignetteStrength{0.22F};
    float exposure{1.04F};
    float saturation{1.03F};
    Color colorGrade{0.99F, 1.02F, 0.98F, 1.0F};
};

struct Composite2D {
    const RenderTarget* world{};
    const RenderTarget* emissive{};
    const RenderTarget* overlay{};
    PostProcessSettings settings{};
};

struct RendererStatistics {
    std::uint64_t submittedSprites{};
    std::uint64_t submittedGeometry{};
    std::uint64_t submittedParticles{};
    std::uint64_t cachedSprites{};
    std::uint64_t batches{};
    std::uint64_t drawCalls{};
    std::uint64_t triangles{};
    std::uint64_t culledSprites{};
    std::uint64_t renderTargetBytes{};
    std::uint64_t frameResourceBytes{};
    std::uint64_t frameStorageGrowths{};
    float cpuFrameMilliseconds{};
    float cpuUpdateMilliseconds{};
    float spriteSubmissionMilliseconds{};
    float particleUpdateMilliseconds{};
    float batchConstructionMilliseconds{};
    float commandSubmissionMilliseconds{};
    float presentWaitMilliseconds{};
    float frameLimitWaitMilliseconds{};
};

class Renderer2D {
public:
    Renderer2D() noexcept;
    ~Renderer2D();
    Renderer2D(Renderer2D&&) noexcept;
    Renderer2D& operator=(Renderer2D&&) noexcept;
    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    [[nodiscard]] static Result<Renderer2D> create(GPUDevice& device, Window& window,
                                                   const std::filesystem::path& shaderDirectory,
                                                   std::size_t maximumVertices = 600'000);
    [[nodiscard]] Result<RenderTarget> createRenderTarget(const RenderTargetDescription& description);
    [[nodiscard]] Result<void> resizeRenderTarget(RenderTarget& target, std::uint32_t width, std::uint32_t height);

    void beginFrame();
    [[nodiscard]] Result<void> beginPass(const RenderPass2D& pass);
    void submit(const Sprite2D& sprite);
    void submit(const Geometry2D& geometry);
    void addSubmittedParticles(std::size_t count) noexcept;
    void addCachedSprites(std::size_t count) noexcept;
    void setApplicationTimings(float updateMilliseconds, float spriteSubmissionMilliseconds,
                               float particleUpdateMilliseconds) noexcept;
    void setFrameLimitWait(float milliseconds) noexcept;
    [[nodiscard]] Result<void> endPass();
    void setComposite(const Composite2D& composite);
    [[nodiscard]] Result<void> present();

    [[nodiscard]] const RendererStatistics& statistics() const noexcept;
    [[nodiscard]] const Texture& whiteTexture() const noexcept;
    [[nodiscard]] const Sampler& defaultSampler() const noexcept;

private:
    struct Impl;
    explicit Renderer2D(std::unique_ptr<Impl>) noexcept;
    std::unique_ptr<Impl> impl_;
};

} // namespace mf
