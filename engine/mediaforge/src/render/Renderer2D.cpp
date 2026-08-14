#include <mediaforge/render/Renderer2D.hpp>

#include "NativeAccess.hpp"

#include <mediaforge/platform/Window.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <format>
#include <utility>
#include <vector>

namespace mf {
namespace {

constexpr std::size_t maximumPasses = 12;

SDL_GPUTextureFormat nativeFormat(RenderTargetDescription::Format format) {
    return format == RenderTargetDescription::Format::rgba16Float
        ? SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT : SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
}

std::uint64_t bytesPerPixel(RenderTargetDescription::Format format) {
    return format == RenderTargetDescription::Format::rgba16Float ? 8U : 4U;
}

struct GPUVertex {
    float x{};
    float y{};
    float r{1.0F};
    float g{1.0F};
    float b{1.0F};
    float a{1.0F};
    float u{};
    float v{};
};

struct Item {
    SubmissionState2D state;
    std::size_t sourceFirst{};
    std::size_t vertexCount{};
};

struct PassRecord {
    RenderPass2D description;
    std::vector<Item> items;
    std::vector<SubmissionState2D> states;
    BatchPlan2D plan;
};

struct FrameResource {
    SDL_GPUBuffer* vertices{};
    SDL_GPUTransferBuffer* upload{};
};

float ndcX(float pixel, float width) { return pixel / width * 2.0F - 1.0F; }
float ndcY(float pixel, float height) { return 1.0F - pixel / height * 2.0F; }

SDL_GPUColorTargetBlendState blendState(BlendMode blend) {
    SDL_GPUColorTargetBlendState result{};
    if (blend == BlendMode::opaque) return result;
    result.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    result.dst_color_blendfactor = blend == BlendMode::additive ? SDL_GPU_BLENDFACTOR_ONE
                                                                 : SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    result.color_blend_op = SDL_GPU_BLENDOP_ADD;
    result.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    result.dst_alpha_blendfactor = blend == BlendMode::additive ? SDL_GPU_BLENDFACTOR_ONE
                                                                 : SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    result.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    result.enable_blend = true;
    return result;
}

SDL_GPUGraphicsPipeline* createPipeline(SDL_GPUDevice* device, SDL_GPUShader* vertexShader,
                                        SDL_GPUShader* fragmentShader, SDL_GPUTextureFormat targetFormat,
                                        BlendMode blend) {
    const SDL_GPUVertexBufferDescription bufferDescription{0, sizeof(GPUVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
    const std::array attributes{
        SDL_GPUVertexAttribute{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(GPUVertex, x)},
        SDL_GPUVertexAttribute{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(GPUVertex, r)},
        SDL_GPUVertexAttribute{2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(GPUVertex, u)},
    };
    SDL_GPUColorTargetDescription colorTarget{targetFormat, blendState(blend)};
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader;
    info.fragment_shader = fragmentShader;
    info.vertex_input_state = {&bufferDescription, 1, attributes.data(), static_cast<Uint32>(attributes.size())};
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    return SDL_CreateGPUGraphicsPipeline(device, &info);
}

} // namespace

struct RenderTarget::Impl {
    SDL_GPUDevice* device{};
    SDL_GPUTexture* texture{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::string debugName;
    RenderTargetDescription::Format format{RenderTargetDescription::Format::rgba16Float};
    ~Impl() { if (texture) SDL_ReleaseGPUTexture(device, texture); }
};

RenderTarget::RenderTarget() noexcept = default;
RenderTarget::~RenderTarget() = default;
RenderTarget::RenderTarget(RenderTarget&&) noexcept = default;
RenderTarget& RenderTarget::operator=(RenderTarget&&) noexcept = default;
RenderTarget::RenderTarget(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {}
RenderTarget::operator bool() const noexcept { return impl_ && impl_->texture; }
std::uint32_t RenderTarget::width() const noexcept { return impl_ ? impl_->width : 0; }
std::uint32_t RenderTarget::height() const noexcept { return impl_ ? impl_->height : 0; }
RenderTargetDescription::Format RenderTarget::format() const noexcept {
    return impl_ ? impl_->format : RenderTargetDescription::Format::rgba16Float;
}
std::uint64_t RenderTarget::approximateMemoryBytes() const noexcept {
    return impl_ ? static_cast<std::uint64_t>(impl_->width) * impl_->height * bytesPerPixel(impl_->format) : 0;
}

bool validRenderTargetDescription(const RenderTargetDescription& description) noexcept {
    constexpr std::uint32_t maximumDimension = 16'384;
    return description.width > 0 && description.height > 0 && description.width <= maximumDimension &&
           description.height <= maximumDimension;
}

bool spriteBoundsVisible(Vec2 screenPosition, Vec2 screenSize, Vec2 pivot,
                         float rotationRadians, Viewport viewport) noexcept {
    const float cosine = std::cos(rotationRadians);
    const float sine = std::sin(rotationRadians);
    const float halfWidth = (std::abs(cosine) * screenSize.x + std::abs(sine) * screenSize.y) * 0.5F;
    const float halfHeight = (std::abs(sine) * screenSize.x + std::abs(cosine) * screenSize.y) * 0.5F;
    const Vec2 pivotOffset{(0.5F - pivot.x) * screenSize.x, (0.5F - pivot.y) * screenSize.y};
    const Vec2 boundsCenter = screenPosition + Vec2{pivotOffset.x * cosine - pivotOffset.y * sine,
                                                    pivotOffset.x * sine + pivotOffset.y * cosine};
    return boundsCenter.x + halfWidth >= viewport.x && boundsCenter.y + halfHeight >= viewport.y &&
           boundsCenter.x - halfWidth <= viewport.x + viewport.width &&
           boundsCenter.y - halfHeight <= viewport.y + viewport.height;
}

std::uint64_t approximateRenderTargetBytes(const RenderTargetDescription& description) noexcept {
    return validRenderTargetDescription(description)
        ? static_cast<std::uint64_t>(description.width) * description.height * bytesPerPixel(description.format) : 0;
}

struct Renderer2D::Impl {
    GPUDevice* owner{};
    Window* window{};
    SDL_GPUDevice* device{};
    std::size_t maximumVertices{};
    std::array<FrameResource, 2> frames{};
    std::size_t frameIndex{};
    std::array<std::array<SDL_GPUGraphicsPipeline*, 3>, 2> spritePipelines{};
    SDL_GPUGraphicsPipeline* compositePipeline{};
    Shader vertexShader;
    Shader spriteShader;
    Shader compositeShader;
    Texture white;
    Sampler sampler;
    std::vector<GPUVertex> sourceVertices;
    std::vector<GPUVertex> stagingVertices;
    std::array<PassRecord, maximumPasses> passes;
    std::size_t passCount{};
    PassRecord* activePass{};
    Composite2D composite{};
    RendererStatistics stats{};
    std::uint64_t residentRenderTargetBytes{};
    std::uint64_t sequence{};
    std::chrono::steady_clock::time_point frameStart{};

    ~Impl() {
        if (!device) return;
        SDL_WaitForGPUIdle(device);
        for (auto& formats : spritePipelines)
            for (auto* pipeline : formats) if (pipeline) SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        if (compositePipeline) SDL_ReleaseGPUGraphicsPipeline(device, compositePipeline);
        for (auto& frame : frames) {
            if (frame.vertices) SDL_ReleaseGPUBuffer(device, frame.vertices);
            if (frame.upload) SDL_ReleaseGPUTransferBuffer(device, frame.upload);
        }
    }

    SDL_GPUGraphicsPipeline* pipeline(RenderTargetDescription::Format format, BlendMode blend) const noexcept {
        return spritePipelines[static_cast<std::size_t>(format)][static_cast<std::size_t>(blend)];
    }
};

Renderer2D::Renderer2D() noexcept = default;
Renderer2D::~Renderer2D() = default;
Renderer2D::Renderer2D(Renderer2D&&) noexcept = default;
Renderer2D& Renderer2D::operator=(Renderer2D&&) noexcept = default;
Renderer2D::Renderer2D(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {}

Result<Renderer2D> Renderer2D::create(GPUDevice& owner, Window& window,
                                      const std::filesystem::path& shaderDirectory, std::size_t maximumVertices) {
    if (maximumVertices < 6) return fail(ErrorCode::invalidArgument, "Renderer vertex capacity is too small");
    auto impl = std::make_unique<Impl>();
    impl->owner = &owner;
    impl->window = &window;
    impl->device = detail::NativeAccess::device(owner);
    impl->maximumVertices = maximumVertices;
    impl->sourceVertices.reserve(maximumVertices);
    impl->stagingVertices.reserve(maximumVertices);
    const std::size_t reservedItemsPerPass = std::min<std::size_t>(maximumVertices / 6U, 4096U);
    for (auto& pass : impl->passes) {
        pass.items.reserve(reservedItemsPerPass);
        pass.states.reserve(reservedItemsPerPass);
        pass.plan.orderedIndices.reserve(reservedItemsPerPass);
        pass.plan.batches.reserve(reservedItemsPerPass);
    }

    auto vertex = owner.createShader(shaderDirectory / "renderer.vert.spv", {ShaderStage::vertex, 0, "Renderer2DVertex"});
    auto sprite = owner.createShader(shaderDirectory / "sprite_linear.frag.spv", {ShaderStage::fragment, 1, "Renderer2DSprite"});
    ShaderDescription compositeDescription{ShaderStage::fragment, 3, "Renderer2DComposite"};
    compositeDescription.uniformBufferCount = 1;
    auto composite = owner.createShader(shaderDirectory / "composite.frag.spv", compositeDescription);
    if (!vertex) return std::unexpected(vertex.error());
    if (!sprite) return std::unexpected(sprite.error());
    if (!composite) return std::unexpected(composite.error());
    impl->vertexShader = std::move(*vertex);
    impl->spriteShader = std::move(*sprite);
    impl->compositeShader = std::move(*composite);

    constexpr std::array<std::uint8_t, 4> whitePixel{255, 255, 255, 255};
    auto white = owner.createTexture({1, 1, "Renderer2DWhite"}, std::as_bytes(std::span(whitePixel)));
    auto sampler = owner.createSampler({FilterMode::linear, WrapMode::clamp, WrapMode::clamp, "Renderer2DLinearClamp"});
    if (!white) return std::unexpected(white.error());
    if (!sampler) return std::unexpected(sampler.error());
    impl->white = std::move(*white);
    impl->sampler = std::move(*sampler);

    for (std::size_t format = 0; format < impl->spritePipelines.size(); ++format) {
        for (std::size_t blend = 0; blend < impl->spritePipelines[format].size(); ++blend) {
            impl->spritePipelines[format][blend] = createPipeline(impl->device,
                detail::NativeAccess::shader(impl->vertexShader), detail::NativeAccess::shader(impl->spriteShader),
                nativeFormat(static_cast<RenderTargetDescription::Format>(format)), static_cast<BlendMode>(blend));
            if (!impl->spritePipelines[format][blend]) {
                return fail(ErrorCode::gpuError, std::format("2D pipeline creation failed: {}", SDL_GetError()));
            }
        }
    }
    const auto swapchainFormat = SDL_GetGPUSwapchainTextureFormat(impl->device, detail::NativeAccess::window(window));
    impl->compositePipeline = createPipeline(impl->device, detail::NativeAccess::shader(impl->vertexShader),
        detail::NativeAccess::shader(impl->compositeShader), swapchainFormat, BlendMode::opaque);
    if (!impl->compositePipeline) return fail(ErrorCode::gpuError, std::format("Composite pipeline creation failed: {}", SDL_GetError()));

    const auto byteCapacity = static_cast<Uint32>(maximumVertices * sizeof(GPUVertex));
    for (auto& frame : impl->frames) {
        const SDL_GPUBufferCreateInfo bufferInfo{SDL_GPU_BUFFERUSAGE_VERTEX, byteCapacity, 0};
        frame.vertices = SDL_CreateGPUBuffer(impl->device, &bufferInfo);
        const SDL_GPUTransferBufferCreateInfo transferInfo{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, byteCapacity, 0};
        frame.upload = SDL_CreateGPUTransferBuffer(impl->device, &transferInfo);
        if (!frame.vertices || !frame.upload) return fail(ErrorCode::gpuError, std::format("Frame resource creation failed: {}", SDL_GetError()));
    }
    return Renderer2D(std::move(impl));
}

Result<RenderTarget> Renderer2D::createRenderTarget(const RenderTargetDescription& description) {
    if (!impl_) return fail(ErrorCode::invalidArgument, "Invalid renderer");
    if (!validRenderTargetDescription(description)) return fail(ErrorCode::invalidArgument, "Invalid render-target dimensions");
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = nativeFormat(description.format);
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.width = description.width;
    info.height = description.height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(impl_->device, &info);
    if (!texture) return fail(ErrorCode::gpuError, std::format("Render-target creation failed: {}", SDL_GetError()));
    if (!description.debugName.empty()) SDL_SetGPUTextureName(impl_->device, texture, description.debugName.c_str());
    auto targetImpl = std::make_unique<RenderTarget::Impl>();
    targetImpl->device = impl_->device;
    targetImpl->texture = texture;
    targetImpl->width = description.width;
    targetImpl->height = description.height;
    targetImpl->debugName = description.debugName;
    targetImpl->format = description.format;
    impl_->residentRenderTargetBytes += static_cast<std::uint64_t>(description.width) * description.height *
                                        bytesPerPixel(description.format);
    return RenderTarget(std::move(targetImpl));
}

Result<void> Renderer2D::resizeRenderTarget(RenderTarget& target, std::uint32_t width, std::uint32_t height) {
    if (!impl_ || !target) return fail(ErrorCode::invalidArgument, "Invalid renderer or render target");
    if (target.width() == width && target.height() == height) return {};
    const auto previousBytes = target.approximateMemoryBytes();
    auto replacement = createRenderTarget({width, height, target.impl_->debugName, target.impl_->format});
    if (!replacement) return std::unexpected(replacement.error());
    SDL_WaitForGPUIdle(impl_->device);
    target = std::move(*replacement);
    impl_->residentRenderTargetBytes -= previousBytes;
    return {};
}

void Renderer2D::beginFrame() {
    if (!impl_) return;
    impl_->stats = {};
    impl_->sourceVertices.clear();
    impl_->stagingVertices.clear();
    impl_->passCount = 0;
    impl_->activePass = nullptr;
    impl_->composite = {};
    impl_->sequence = 0;
    impl_->frameStart = std::chrono::steady_clock::now();
    impl_->stats.renderTargetBytes = impl_->residentRenderTargetBytes;
    impl_->stats.frameResourceBytes = static_cast<std::uint64_t>(impl_->maximumVertices * sizeof(GPUVertex) *
        impl_->frames.size() * 2U);
}

Result<void> Renderer2D::beginPass(const RenderPass2D& pass) {
    if (!impl_ || impl_->activePass) return fail(ErrorCode::invalidArgument, "Render pass nesting is not supported");
    if (!pass.target || !*pass.target) return fail(ErrorCode::invalidArgument, "A valid offscreen target is required");
    if (impl_->passCount >= impl_->passes.size()) return fail(ErrorCode::invalidArgument, "Renderer pass capacity exceeded");
    auto& record = impl_->passes[impl_->passCount++];
    record.description = pass;
    record.items.clear();
    record.states.clear();
    record.plan.orderedIndices.clear();
    record.plan.batches.clear();
    impl_->activePass = &record;
    return {};
}

namespace {

GPUVertex makeVertex(Vec2 screen, Vec2 targetSize, Color color, Vec2 uv) {
    return {ndcX(screen.x, targetSize.x), ndcY(screen.y, targetSize.y), color.r, color.g, color.b, color.a, uv.x, uv.y};
}

} // namespace

void Renderer2D::submit(const Sprite2D& sprite) {
    if (!impl_ || !impl_->activePass) return;
    auto& pass = *impl_->activePass;
    const Vec2 targetSize{static_cast<float>(pass.description.target->width()), static_cast<float>(pass.description.target->height())};
    Vec2 center = sprite.position;
    Vec2 size = {sprite.size.x * sprite.scale.x, sprite.size.y * sprite.scale.y};
    if (sprite.space == CoordinateSpace::world) {
        center = pass.description.camera.worldToScreen(center);
        const Vec2 visible = pass.description.camera.visibleSize();
        const Viewport viewport = pass.description.camera.viewport();
        size = {size.x * viewport.width / visible.x, size.y * viewport.height / visible.y};
    }
    const std::array<Vec2, 4> local{{
        {-sprite.pivot.x * size.x, -sprite.pivot.y * size.y},
        {(1.0F - sprite.pivot.x) * size.x, -sprite.pivot.y * size.y},
        {(1.0F - sprite.pivot.x) * size.x, (1.0F - sprite.pivot.y) * size.y},
        {-sprite.pivot.x * size.x, (1.0F - sprite.pivot.y) * size.y},
    }};
    const float cosine = std::cos(sprite.rotationRadians);
    const float sine = std::sin(sprite.rotationRadians);
    const auto viewport = pass.description.camera.viewport();
    if (!spriteBoundsVisible(center, size, sprite.pivot, sprite.rotationRadians, viewport)) {
        ++impl_->stats.culledSprites;
        return;
    }
    std::array<Vec2, 4> screen{};
    for (std::size_t i = 0; i < local.size(); ++i) {
        screen[i] = center + Vec2{local[i].x * cosine - local[i].y * sine,
                                  local[i].x * sine + local[i].y * cosine};
    }
    const Color tint{sprite.color.r, sprite.color.g, sprite.color.b,
                     clamp(sprite.color.a * sprite.opacity, 0.0F, 1.0F)};
    const std::array<Vec2, 4> uv{{{sprite.uv.left, sprite.uv.top}, {sprite.uv.right, sprite.uv.top},
                                  {sprite.uv.right, sprite.uv.bottom}, {sprite.uv.left, sprite.uv.bottom}}};
    constexpr std::array<std::size_t, 6> indices{0, 1, 2, 0, 2, 3};
    const std::size_t first = impl_->sourceVertices.size();
    if (impl_->sourceVertices.size() + indices.size() > impl_->sourceVertices.capacity()) ++impl_->stats.frameStorageGrowths;
    for (const auto index : indices) impl_->sourceVertices.push_back(makeVertex(screen[index], targetSize, tint, uv[index]));
    const Texture* texture = sprite.texture ? sprite.texture : &impl_->white;
    const Sampler* sampler = sprite.sampler ? sprite.sampler : &impl_->sampler;
    if (pass.items.size() == pass.items.capacity()) ++impl_->stats.frameStorageGrowths;
    pass.items.push_back({{sprite.layer, sprite.order, reinterpret_cast<std::uint64_t>(sampler),
                           reinterpret_cast<std::uint64_t>(texture), sprite.blend, impl_->sequence++}, first, 6});
    ++impl_->stats.submittedSprites;
}

void Renderer2D::submit(const Geometry2D& geometry) {
    if (!impl_ || !impl_->activePass || geometry.vertices.empty()) return;
    auto& pass = *impl_->activePass;
    const Vec2 targetSize{static_cast<float>(pass.description.target->width()), static_cast<float>(pass.description.target->height())};
    const std::size_t first = impl_->sourceVertices.size();
    if (impl_->sourceVertices.size() + geometry.vertices.size() > impl_->sourceVertices.capacity()) {
        ++impl_->stats.frameStorageGrowths;
    }
    for (const auto& vertex : geometry.vertices) {
        const Vec2 screen = geometry.space == CoordinateSpace::world ? pass.description.camera.worldToScreen(vertex.position)
                                                                      : vertex.position;
        impl_->sourceVertices.push_back(makeVertex(screen, targetSize, vertex.color, vertex.uv));
    }
    const Texture* texture = geometry.texture ? geometry.texture : &impl_->white;
    const Sampler* sampler = geometry.sampler ? geometry.sampler : &impl_->sampler;
    if (pass.items.size() == pass.items.capacity()) ++impl_->stats.frameStorageGrowths;
    pass.items.push_back({{geometry.layer, geometry.order, reinterpret_cast<std::uint64_t>(sampler),
                           reinterpret_cast<std::uint64_t>(texture), geometry.blend, impl_->sequence++},
                          first, geometry.vertices.size()});
    ++impl_->stats.submittedGeometry;
}

void Renderer2D::addSubmittedParticles(std::size_t count) noexcept { if (impl_) impl_->stats.submittedParticles += count; }
void Renderer2D::addCachedSprites(std::size_t count) noexcept { if (impl_) impl_->stats.cachedSprites += count; }

void Renderer2D::setApplicationTimings(float updateMilliseconds, float spriteMilliseconds,
                                       float particleMilliseconds) noexcept {
    if (!impl_) return;
    impl_->stats.cpuUpdateMilliseconds = updateMilliseconds;
    impl_->stats.spriteSubmissionMilliseconds = spriteMilliseconds;
    impl_->stats.particleUpdateMilliseconds = particleMilliseconds;
}

void Renderer2D::setFrameLimitWait(float milliseconds) noexcept {
    if (impl_) impl_->stats.frameLimitWaitMilliseconds = milliseconds;
}

Result<void> Renderer2D::endPass() {
    if (!impl_ || !impl_->activePass) return fail(ErrorCode::invalidArgument, "No active render pass");
    impl_->activePass = nullptr;
    return {};
}

void Renderer2D::setComposite(const Composite2D& composite) { if (impl_) impl_->composite = composite; }

Result<void> Renderer2D::present() {
    if (!impl_ || impl_->activePass) return fail(ErrorCode::invalidArgument, "Cannot present with an active render pass");
    if (!impl_->composite.world || !impl_->composite.emissive || !impl_->composite.overlay) {
        return fail(ErrorCode::invalidArgument, "World, emissive and overlay targets are required for composition");
    }

    const auto batchStart = std::chrono::steady_clock::now();
    for (std::size_t passIndex = 0; passIndex < impl_->passCount; ++passIndex) {
        auto& pass = impl_->passes[passIndex];
        pass.states.clear();
        if (pass.items.size() > pass.states.capacity()) ++impl_->stats.frameStorageGrowths;
        for (const auto& item : pass.items) pass.states.push_back(item.state);
        const auto orderedCapacity = pass.plan.orderedIndices.capacity();
        const auto batchCapacity = pass.plan.batches.capacity();
        buildBatchPlan(pass.states, pass.plan);
        impl_->stats.frameStorageGrowths += pass.plan.orderedIndices.capacity() != orderedCapacity;
        impl_->stats.frameStorageGrowths += pass.plan.batches.capacity() != batchCapacity;
        for (const auto& batch : pass.plan.batches) {
            std::size_t batchVertices{};
            for (std::size_t itemIndex = 0; itemIndex < batch.count; ++itemIndex) {
                batchVertices += pass.items[pass.plan.orderedIndices[batch.first + itemIndex]].vertexCount;
            }
            if (impl_->stagingVertices.size() + batchVertices > impl_->stagingVertices.capacity()) {
                ++impl_->stats.frameStorageGrowths;
            }
            for (std::size_t itemIndex = 0; itemIndex < batch.count; ++itemIndex) {
                const Item& item = pass.items[pass.plan.orderedIndices[batch.first + itemIndex]];
                impl_->stagingVertices.insert(impl_->stagingVertices.end(),
                    impl_->sourceVertices.begin() + static_cast<std::ptrdiff_t>(item.sourceFirst),
                    impl_->sourceVertices.begin() + static_cast<std::ptrdiff_t>(item.sourceFirst + item.vertexCount));
            }
        }
    }
    impl_->stats.batchConstructionMilliseconds = std::chrono::duration<float, std::milli>(
        std::chrono::steady_clock::now() - batchStart).count();

    const std::size_t compositeFirst = impl_->stagingVertices.size();
    constexpr std::array<GPUVertex, 6> fullScreen{{
        {-1, 1, 1, 1, 1, 1, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 0}, {1, -1, 1, 1, 1, 1, 1, 1},
        {-1, 1, 1, 1, 1, 1, 0, 0}, {1, -1, 1, 1, 1, 1, 1, 1}, {-1, -1, 1, 1, 1, 1, 0, 1},
    }};
    impl_->stagingVertices.insert(impl_->stagingVertices.end(), fullScreen.begin(), fullScreen.end());
    if (impl_->stagingVertices.size() > impl_->maximumVertices) return fail(ErrorCode::gpuError, "Renderer frame vertex capacity exceeded");

    const auto commandStart = std::chrono::steady_clock::now();
    FrameResource& frame = impl_->frames[impl_->frameIndex++ % impl_->frames.size()];
    void* mapped = SDL_MapGPUTransferBuffer(impl_->device, frame.upload, true);
    if (!mapped) return fail(ErrorCode::gpuError, std::format("Frame upload map failed: {}", SDL_GetError()));
    std::memcpy(mapped, impl_->stagingVertices.data(), impl_->stagingVertices.size() * sizeof(GPUVertex));
    SDL_UnmapGPUTransferBuffer(impl_->device, frame.upload);

    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!commands) return fail(ErrorCode::gpuError, SDL_GetError());
    SDL_GPUTexture* swapchain{};
    Uint32 swapchainWidth{};
    Uint32 swapchainHeight{};
    const auto waitStart = std::chrono::steady_clock::now();
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commands, detail::NativeAccess::window(*impl_->window),
                                                &swapchain, &swapchainWidth, &swapchainHeight)) {
        SDL_CancelGPUCommandBuffer(commands);
        return fail(ErrorCode::gpuError, SDL_GetError());
    }
    impl_->stats.presentWaitMilliseconds = std::chrono::duration<float, std::milli>(
        std::chrono::steady_clock::now() - waitStart).count();
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    const SDL_GPUTransferBufferLocation source{frame.upload, 0};
    const SDL_GPUBufferRegion destination{frame.vertices, 0,
        static_cast<Uint32>(impl_->stagingVertices.size() * sizeof(GPUVertex))};
    SDL_UploadToGPUBuffer(copy, &source, &destination, true);
    SDL_EndGPUCopyPass(copy);

    std::size_t stagedFirst{};
    for (std::size_t passIndex = 0; passIndex < impl_->passCount; ++passIndex) {
        const auto& pass = impl_->passes[passIndex];
        SDL_GPUColorTargetInfo target{};
        target.texture = pass.description.target->impl_->texture;
        target.clear_color = {pass.description.clearColor.r, pass.description.clearColor.g,
                              pass.description.clearColor.b, pass.description.clearColor.a};
        target.load_op = pass.description.clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
        target.store_op = SDL_GPU_STOREOP_STORE;
        SDL_GPURenderPass* nativePass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
        const auto viewportValue = pass.description.camera.viewport();
        const SDL_GPUViewport viewport{viewportValue.x, viewportValue.y, viewportValue.width, viewportValue.height, 0.0F, 1.0F};
        SDL_SetGPUViewport(nativePass, &viewport);
        for (const auto& batch : pass.plan.batches) {
            std::size_t vertexCount{};
            for (std::size_t itemIndex = 0; itemIndex < batch.count; ++itemIndex) {
                vertexCount += pass.items[pass.plan.orderedIndices[batch.first + itemIndex]].vertexCount;
            }
            SDL_BindGPUGraphicsPipeline(nativePass, impl_->pipeline(pass.description.target->format(), batch.state.blend));
            const SDL_GPUBufferBinding vertexBinding{frame.vertices,
                static_cast<Uint32>(stagedFirst * sizeof(GPUVertex))};
            SDL_BindGPUVertexBuffers(nativePass, 0, &vertexBinding, 1);
            const auto* texture = reinterpret_cast<const Texture*>(batch.state.texture);
            const auto* sampler = reinterpret_cast<const Sampler*>(batch.state.pipeline);
            const SDL_GPUTextureSamplerBinding textureBinding{detail::NativeAccess::texture(*texture),
                                                               detail::NativeAccess::sampler(*sampler)};
            SDL_BindGPUFragmentSamplers(nativePass, 0, &textureBinding, 1);
            SDL_DrawGPUPrimitives(nativePass, static_cast<Uint32>(vertexCount), 1, 0, 0);
            stagedFirst += vertexCount;
            ++impl_->stats.batches;
            ++impl_->stats.drawCalls;
            impl_->stats.triangles += vertexCount / 3;
        }
        SDL_EndGPURenderPass(nativePass);
    }

    if (swapchain) {
        SDL_GPUColorTargetInfo target{};
        target.texture = swapchain;
        target.clear_color = {0.005F, 0.008F, 0.012F, 1.0F};
        target.load_op = SDL_GPU_LOADOP_CLEAR;
        target.store_op = SDL_GPU_STOREOP_STORE;
        const auto contentWidth = static_cast<float>(impl_->composite.world->width());
        const auto contentHeight = static_cast<float>(impl_->composite.world->height());
        struct alignas(16) CompositeUniforms { std::array<float, 4> gradeExposure; std::array<float, 4> bloomVignetteSaturation; std::array<float, 4> inverseSize; };
        const auto& settings = impl_->composite.settings;
        const CompositeUniforms uniforms{{settings.colorGrade.r, settings.colorGrade.g, settings.colorGrade.b, settings.exposure},
            {settings.bloomStrength, settings.bloomRadius, settings.vignetteStrength, settings.saturation},
            {1.0F / contentWidth, 1.0F / contentHeight,
             1.0F / static_cast<float>(impl_->composite.emissive->width()),
             1.0F / static_cast<float>(impl_->composite.emissive->height())}};
        SDL_PushGPUFragmentUniformData(commands, 0, &uniforms, sizeof(uniforms));
        SDL_GPURenderPass* finalPass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
        const auto fitted = Camera2D::letterbox({contentWidth, contentHeight},
                                                {static_cast<float>(swapchainWidth), static_cast<float>(swapchainHeight)});
        const SDL_GPUViewport viewport{fitted.x, fitted.y, fitted.width, fitted.height, 0.0F, 1.0F};
        SDL_SetGPUViewport(finalPass, &viewport);
        SDL_BindGPUGraphicsPipeline(finalPass, impl_->compositePipeline);
        const SDL_GPUBufferBinding vertexBinding{frame.vertices,
            static_cast<Uint32>(compositeFirst * sizeof(GPUVertex))};
        SDL_BindGPUVertexBuffers(finalPass, 0, &vertexBinding, 1);
        const std::array textureBindings{
            SDL_GPUTextureSamplerBinding{impl_->composite.world->impl_->texture, detail::NativeAccess::sampler(impl_->sampler)},
            SDL_GPUTextureSamplerBinding{impl_->composite.emissive->impl_->texture, detail::NativeAccess::sampler(impl_->sampler)},
            SDL_GPUTextureSamplerBinding{impl_->composite.overlay->impl_->texture, detail::NativeAccess::sampler(impl_->sampler)},
        };
        SDL_BindGPUFragmentSamplers(finalPass, 0, textureBindings.data(), static_cast<Uint32>(textureBindings.size()));
        SDL_DrawGPUPrimitives(finalPass, 6, 1, 0, 0);
        SDL_EndGPURenderPass(finalPass);
        ++impl_->stats.drawCalls;
        ++impl_->stats.batches;
        impl_->stats.triangles += 2;
    }
    if (!SDL_SubmitGPUCommandBuffer(commands)) return fail(ErrorCode::gpuError, SDL_GetError());
    impl_->stats.commandSubmissionMilliseconds = std::max(0.0F,
        std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - commandStart).count() -
        impl_->stats.presentWaitMilliseconds);
    impl_->stats.cpuFrameMilliseconds = impl_->stats.cpuUpdateMilliseconds + impl_->stats.spriteSubmissionMilliseconds +
        impl_->stats.particleUpdateMilliseconds + impl_->stats.batchConstructionMilliseconds +
        impl_->stats.commandSubmissionMilliseconds;
    return {};
}

const RendererStatistics& Renderer2D::statistics() const noexcept {
    static const RendererStatistics empty{};
    return impl_ ? impl_->stats : empty;
}

const Texture& Renderer2D::whiteTexture() const noexcept { return impl_->white; }
const Sampler& Renderer2D::defaultSampler() const noexcept { return impl_->sampler; }

} // namespace mf
