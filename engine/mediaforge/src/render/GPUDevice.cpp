#include <mediaforge/render/GPUDevice.hpp>

#include "NativeAccess.hpp"

#include <mediaforge/foundation/Log.hpp>
#include <mediaforge/platform/Window.hpp>

#include <SDL3/SDL.h>

#include <cstring>
#include <format>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace mf {

namespace {
SDL_GPUPresentMode nativePresentMode(PresentMode mode) {
    switch (mode) {
        case PresentMode::vsync: return SDL_GPU_PRESENTMODE_VSYNC;
        case PresentMode::immediate: return SDL_GPU_PRESENTMODE_IMMEDIATE;
        case PresentMode::mailbox: return SDL_GPU_PRESENTMODE_MAILBOX;
    }
    return SDL_GPU_PRESENTMODE_VSYNC;
}
}

struct Buffer::Impl { SDL_GPUDevice* device{}; SDL_GPUBuffer* value{}; ~Impl() { if (value) SDL_ReleaseGPUBuffer(device, value); } };
struct Texture::Impl { SDL_GPUDevice* device{}; SDL_GPUTexture* value{}; ~Impl() { if (value) SDL_ReleaseGPUTexture(device, value); } };
struct Sampler::Impl { SDL_GPUDevice* device{}; SDL_GPUSampler* value{}; ~Impl() { if (value) SDL_ReleaseGPUSampler(device, value); } };
struct Shader::Impl { SDL_GPUDevice* device{}; SDL_GPUShader* value{}; ~Impl() { if (value) SDL_ReleaseGPUShader(device, value); } };
struct GraphicsPipeline::Impl { SDL_GPUDevice* device{}; SDL_GPUGraphicsPipeline* value{}; ~Impl() { if (value) SDL_ReleaseGPUGraphicsPipeline(device, value); } };

#define MF_RESOURCE_SPECIAL_MEMBERS(Type) \
    Type::Type() noexcept = default; \
    Type::~Type() = default; \
    Type::Type(Type&&) noexcept = default; \
    Type& Type::operator=(Type&&) noexcept = default; \
    Type::Type(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {} \
    Type::operator bool() const noexcept { return impl_ && impl_->value; }

MF_RESOURCE_SPECIAL_MEMBERS(Buffer)
MF_RESOURCE_SPECIAL_MEMBERS(Texture)
MF_RESOURCE_SPECIAL_MEMBERS(Sampler)
MF_RESOURCE_SPECIAL_MEMBERS(Shader)
MF_RESOURCE_SPECIAL_MEMBERS(GraphicsPipeline)
#undef MF_RESOURCE_SPECIAL_MEMBERS

struct GPUDevice::Impl {
    SDL_GPUDevice* device{};
    SDL_Window* claimedWindow{};

    ~Impl() {
        if (device != nullptr) {
            SDL_WaitForGPUIdle(device);
            if (claimedWindow != nullptr) { SDL_ReleaseWindowFromGPUDevice(device, claimedWindow); }
            SDL_DestroyGPUDevice(device);
        }
    }
};

GPUDevice::GPUDevice() noexcept = default;
GPUDevice::~GPUDevice() = default;
GPUDevice::GPUDevice(GPUDevice&&) noexcept = default;
GPUDevice& GPUDevice::operator=(GPUDevice&&) noexcept = default;
GPUDevice::GPUDevice(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {}

Result<GPUDevice> GPUDevice::create(Window& window, const EngineConfig& config) {
    const char* requested = preferredBackendName(config.gpuBackend);
    SDL_GPUDevice* native = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                                config.gpuDebug, requested);
    if (native == nullptr && config.gpuBackend == GpuBackendPreference::preferVulkan) {
        log(LogLevel::warning, std::format("Vulkan GPU creation failed; trying SDL automatic selection: {}", SDL_GetError()));
        native = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, config.gpuDebug, nullptr);
    }
    if (native == nullptr) {
        return fail(ErrorCode::gpuError, std::format("GPU device creation failed: {}", SDL_GetError()));
    }

    auto impl = std::make_unique<Impl>();
    impl->device = native;
    SDL_Window* nativeWindow = static_cast<SDL_Window*>(window.nativeHandle());
    if (!SDL_ClaimWindowForGPUDevice(native, nativeWindow)) {
        return fail(ErrorCode::gpuError, std::format("GPU window claim failed: {}", SDL_GetError()));
    }
    impl->claimedWindow = nativeWindow;
    GPUDevice result(std::move(impl));
    log(LogLevel::info, std::format("MediaForge GPU backend: {}", result.backendName()));
    return result;
}

std::string GPUDevice::backendName() const {
    if (!impl_) { return {}; }
    const char* name = SDL_GetGPUDeviceDriver(impl_->device);
    return name != nullptr ? name : "unknown";
}

bool GPUDevice::supportsPresentMode(const Window& window, PresentMode mode) const noexcept {
    return impl_ && SDL_WindowSupportsGPUPresentMode(impl_->device,
        static_cast<SDL_Window*>(window.nativeHandle()), nativePresentMode(mode));
}

Result<void> GPUDevice::setPresentMode(Window& window, PresentMode mode) {
    if (!impl_) return fail(ErrorCode::invalidArgument, "Invalid GPU device");
    const auto native = nativePresentMode(mode);
    if (!supportsPresentMode(window, mode)) return fail(ErrorCode::gpuError, "Requested GPU present mode is unsupported");
    if (!SDL_SetGPUSwapchainParameters(impl_->device, static_cast<SDL_Window*>(window.nativeHandle()),
                                       SDL_GPU_SWAPCHAINCOMPOSITION_SDR, native)) {
        return fail(ErrorCode::gpuError, std::format("Could not set GPU present mode: {}", SDL_GetError()));
    }
    return {};
}

namespace {

Result<SDL_GPUTransferBuffer*> createUploadTransfer(SDL_GPUDevice* device, std::span<const std::byte> bytes) {
    const SDL_GPUTransferBufferCreateInfo info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                               static_cast<Uint32>(bytes.size()), 0};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &info);
    if (transfer == nullptr) { return fail(ErrorCode::gpuError, SDL_GetError()); }
    void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
    if (mapped == nullptr) {
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        return fail(ErrorCode::gpuError, SDL_GetError());
    }
    std::memcpy(mapped, bytes.data(), bytes.size());
    SDL_UnmapGPUTransferBuffer(device, transfer);
    return transfer;
}

SDL_GPUVertexElementFormat vertexFormat(VertexFormat value) {
    return value == VertexFormat::float2 ? SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 : SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
}

} // namespace

Result<Buffer> GPUDevice::createBuffer(BufferUsage usage, std::span<const std::byte> data, std::string debugName) {
    if (!impl_ || data.empty()) { return fail(ErrorCode::invalidArgument, "Buffer data must not be empty"); }
    const SDL_GPUBufferCreateInfo info{usage == BufferUsage::vertex ? SDL_GPU_BUFFERUSAGE_VERTEX : SDL_GPU_BUFFERUSAGE_INDEX,
                                       static_cast<Uint32>(data.size()), 0};
    SDL_GPUBuffer* native = SDL_CreateGPUBuffer(impl_->device, &info);
    if (!native) { return fail(ErrorCode::gpuError, SDL_GetError()); }
    if (!debugName.empty()) { SDL_SetGPUBufferName(impl_->device, native, debugName.c_str()); }

    auto transferResult = createUploadTransfer(impl_->device, data);
    if (!transferResult) { SDL_ReleaseGPUBuffer(impl_->device, native); return std::unexpected(transferResult.error()); }
    SDL_GPUTransferBuffer* transfer = *transferResult;
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!commands) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); SDL_ReleaseGPUBuffer(impl_->device, native); return fail(ErrorCode::gpuError, SDL_GetError()); }
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    const SDL_GPUTransferBufferLocation source{transfer, 0};
    const SDL_GPUBufferRegion destination{native, 0, static_cast<Uint32>(data.size())};
    SDL_UploadToGPUBuffer(copy, &source, &destination, false);
    SDL_EndGPUCopyPass(copy);
    const bool submitted = SDL_SubmitGPUCommandBuffer(commands);
    SDL_ReleaseGPUTransferBuffer(impl_->device, transfer);
    if (!submitted) { SDL_ReleaseGPUBuffer(impl_->device, native); return fail(ErrorCode::gpuError, SDL_GetError()); }
    auto resource = std::make_unique<Buffer::Impl>(); resource->device = impl_->device; resource->value = native;
    return Buffer(std::move(resource));
}

Result<Texture> GPUDevice::createTexture(const TextureDescription& description, std::span<const std::byte> pixels) {
    const std::size_t expected = static_cast<std::size_t>(description.width) * description.height * 4U;
    if (!impl_ || description.width == 0 || description.height == 0 || pixels.size() != expected) {
        return fail(ErrorCode::invalidArgument, "RGBA texture byte count does not match dimensions");
    }
    const SDL_GPUTextureCreateInfo info{SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        SDL_GPU_TEXTUREUSAGE_SAMPLER, description.width, description.height, 1, 1, SDL_GPU_SAMPLECOUNT_1, 0};
    SDL_GPUTexture* native = SDL_CreateGPUTexture(impl_->device, &info);
    if (!native) { return fail(ErrorCode::gpuError, SDL_GetError()); }
    if (!description.debugName.empty()) { SDL_SetGPUTextureName(impl_->device, native, description.debugName.c_str()); }
    auto transferResult = createUploadTransfer(impl_->device, pixels);
    if (!transferResult) { SDL_ReleaseGPUTexture(impl_->device, native); return std::unexpected(transferResult.error()); }
    SDL_GPUTransferBuffer* transfer = *transferResult;
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!commands) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); SDL_ReleaseGPUTexture(impl_->device, native); return fail(ErrorCode::gpuError, SDL_GetError()); }
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    const SDL_GPUTextureTransferInfo source{transfer, 0, description.width, description.height};
    const SDL_GPUTextureRegion destination{native, 0, 0, 0, 0, 0, description.width, description.height, 1};
    SDL_UploadToGPUTexture(copy, &source, &destination, false);
    SDL_EndGPUCopyPass(copy);
    const bool submitted = SDL_SubmitGPUCommandBuffer(commands);
    SDL_ReleaseGPUTransferBuffer(impl_->device, transfer);
    if (!submitted) { SDL_ReleaseGPUTexture(impl_->device, native); return fail(ErrorCode::gpuError, SDL_GetError()); }
    auto resource = std::make_unique<Texture::Impl>(); resource->device = impl_->device; resource->value = native;
    return Texture(std::move(resource));
}

Result<Sampler> GPUDevice::createSampler(const SamplerDescription& description) {
    if (!impl_) { return fail(ErrorCode::invalidArgument, "Invalid GPU device"); }
    SDL_GPUSamplerCreateInfo info{};
    info.min_filter = description.filter == FilterMode::linear ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
    info.mag_filter = info.min_filter;
    info.mipmap_mode = description.filter == FilterMode::linear ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u = description.wrapU == WrapMode::repeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = description.wrapV == WrapMode::repeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    SDL_GPUSampler* native = SDL_CreateGPUSampler(impl_->device, &info);
    if (!native) { return fail(ErrorCode::gpuError, SDL_GetError()); }
    auto resource = std::make_unique<Sampler::Impl>(); resource->device = impl_->device; resource->value = native;
    return Sampler(std::move(resource));
}

Result<Shader> GPUDevice::createShader(const std::filesystem::path& path, const ShaderDescription& description) {
    std::ifstream input(path, std::ios::binary);
    if (!input) { log(LogLevel::error, std::format("Shader load failed: {}", path.string())); return fail(ErrorCode::ioError, "Cannot open shader: " + path.string()); }
    const std::vector<unsigned char> bytes(std::istreambuf_iterator<char>(input), {});
    const SDL_GPUShaderCreateInfo info{bytes.size(), bytes.data(), "main", SDL_GPU_SHADERFORMAT_SPIRV,
        description.stage == ShaderStage::vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT,
        description.samplerCount, 0, 0, description.uniformBufferCount, 0};
    SDL_GPUShader* native = SDL_CreateGPUShader(impl_->device, &info);
    if (!native) { return fail(ErrorCode::gpuError, std::format("Shader creation failed ({}): {}", path.string(), SDL_GetError())); }
    auto resource = std::make_unique<Shader::Impl>(); resource->device = impl_->device; resource->value = native;
    return Shader(std::move(resource));
}

Result<GraphicsPipeline> GPUDevice::createGraphicsPipeline(Window& window, const Shader& vertexShader,
    const Shader& fragmentShader, std::uint32_t stride, std::span<const VertexAttribute> attributes, bool alpha) {
    if (!impl_ || !vertexShader || !fragmentShader) { return fail(ErrorCode::invalidArgument, "Invalid pipeline inputs"); }
    std::vector<SDL_GPUVertexAttribute> nativeAttributes;
    nativeAttributes.reserve(attributes.size());
    for (const auto& attribute : attributes) {
        nativeAttributes.push_back({attribute.location, 0, vertexFormat(attribute.format), attribute.offset});
    }
    const SDL_GPUVertexBufferDescription bufferDescription{0, stride, SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = SDL_GetGPUSwapchainTextureFormat(impl_->device, static_cast<SDL_Window*>(window.nativeHandle()));
    if (alpha) {
        colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTarget.blend_state.enable_blend = true;
    }
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader.impl_->value; info.fragment_shader = fragmentShader.impl_->value;
    info.vertex_input_state = {&bufferDescription, 1, nativeAttributes.data(), static_cast<Uint32>(nativeAttributes.size())};
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    SDL_GPUGraphicsPipeline* native = SDL_CreateGPUGraphicsPipeline(impl_->device, &info);
    if (!native) { return fail(ErrorCode::gpuError, std::format("Graphics pipeline creation failed: {}", SDL_GetError())); }
    auto resource = std::make_unique<GraphicsPipeline::Impl>(); resource->device = impl_->device; resource->value = native;
    return GraphicsPipeline(std::move(resource));
}

namespace detail {
SDL_Window* NativeAccess::window(const Window& value) noexcept { return static_cast<SDL_Window*>(value.nativeHandle()); }
SDL_GPUDevice* NativeAccess::device(const GPUDevice& value) noexcept { return value.impl_ ? value.impl_->device : nullptr; }
SDL_GPUBuffer* NativeAccess::buffer(const Buffer& value) noexcept { return value.impl_ ? value.impl_->value : nullptr; }
SDL_GPUTexture* NativeAccess::texture(const Texture& value) noexcept { return value.impl_ ? value.impl_->value : nullptr; }
SDL_GPUSampler* NativeAccess::sampler(const Sampler& value) noexcept { return value.impl_ ? value.impl_->value : nullptr; }
SDL_GPUShader* NativeAccess::shader(const Shader& value) noexcept { return value.impl_ ? value.impl_->value : nullptr; }
SDL_GPUGraphicsPipeline* NativeAccess::pipeline(const GraphicsPipeline& value) noexcept { return value.impl_ ? value.impl_->value : nullptr; }
} // namespace detail

} // namespace mf
