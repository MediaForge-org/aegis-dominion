#pragma once

#include <mediaforge/foundation/Error.hpp>
#include <mediaforge/math/Math.hpp>
#include <mediaforge/render/GPUDevice.hpp>
#include <mediaforge/render/Renderer2D.hpp>

#include <cstdint>
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace aegis::mediaforge {

enum class E2AssetId : std::uint8_t { interfaceFont };

struct TransparentStringHash {
    using is_transparent = void;
    std::size_t operator()(std::string_view value) const noexcept { return std::hash<std::string_view>{}(value); }
    std::size_t operator()(const std::string& value) const noexcept { return (*this)(std::string_view(value)); }
};

struct VisualAsset {
    const mf::Texture* texture{};
    const mf::Sampler* sampler{};
    mf::UvRegion uv{};
    mf::Vec2 sourceSize{};
    mf::Vec2 pivot{0.5F, 0.5F};
    float scale{1.0F};
    std::string material;
    std::string emissiveId;
    std::string normalId;
    std::string animationId;
};

class E2AssetCatalog {
public:
    [[nodiscard]] static mf::Result<E2AssetCatalog> load(mf::GPUDevice& device,
        const std::filesystem::path& assetsRoot, const std::filesystem::path& manifest);
    [[nodiscard]] const VisualAsset* find(std::string_view logicalId) const noexcept;
    [[nodiscard]] const VisualAsset* find(E2AssetId logicalId) const noexcept;
    [[nodiscard]] std::size_t assetCount() const noexcept { return assets_.size(); }
    [[nodiscard]] std::size_t textureCount() const noexcept { return textures_.size(); }
    [[nodiscard]] std::uint64_t approximateTextureBytes() const noexcept { return textureBytes_; }
    [[nodiscard]] float loadMilliseconds() const noexcept { return loadMilliseconds_; }

private:
    std::vector<mf::Texture> textures_;
    std::vector<mf::Sampler> samplers_;
    std::unordered_map<std::string, VisualAsset, TransparentStringHash, std::equal_to<>> assets_;
    std::uint64_t textureBytes_{};
    float loadMilliseconds_{};
};

} // namespace aegis::mediaforge
