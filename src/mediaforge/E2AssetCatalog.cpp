#include "E2AssetCatalog.hpp"

#include <mediaforge/render/Image.hpp>

#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace aegis::mediaforge {
namespace {

struct Definition {
    std::string id;
    std::string path;
    std::uint32_t x{}, y{}, width{}, height{}, sourceWidth{}, sourceHeight{};
    std::string filter;
    std::string wrap;
    float pivotX{}, pivotY{}, scale{};
    std::string material;
    std::string emissive;
    std::string normal;
    std::string animation;
};

mf::Result<std::vector<Definition>> readDefinitions(const std::filesystem::path& manifest) {
    std::ifstream input(manifest);
    if (!input) return mf::fail(mf::ErrorCode::ioError, "Cannot open E2 asset manifest: " + manifest.string());
    std::string line;
    if (!std::getline(input, line) || line != "MEDIAFORGE_ASSETS_V1") {
        return mf::fail(mf::ErrorCode::ioError, "Unsupported E2 asset manifest header");
    }
    std::vector<Definition> definitions;
    std::size_t lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty() || line.front() == '#') continue;
        std::istringstream row(line);
        std::string keyword;
        Definition value;
        row >> keyword >> std::quoted(value.id) >> std::quoted(value.path) >> value.x >> value.y >> value.width >> value.height
            >> value.sourceWidth >> value.sourceHeight >> value.filter >> value.wrap >> value.pivotX >> value.pivotY >> value.scale
            >> std::quoted(value.material) >> std::quoted(value.emissive) >> std::quoted(value.normal) >> std::quoted(value.animation);
        if (!row || keyword != "asset" || value.id.empty() || value.path.empty() || value.width == 0 || value.height == 0 ||
            value.sourceWidth == 0 || value.sourceHeight == 0 || value.x + value.width > value.sourceWidth ||
            value.y + value.height > value.sourceHeight) {
            return mf::fail(mf::ErrorCode::ioError, "Malformed E2 asset row " + std::to_string(lineNumber));
        }
        definitions.push_back(std::move(value));
    }
    return definitions;
}

} // namespace

mf::Result<E2AssetCatalog> E2AssetCatalog::load(mf::GPUDevice& device, const std::filesystem::path& assetsRoot,
                                                 const std::filesystem::path& manifest) {
    const auto loadStart = std::chrono::steady_clock::now();
    auto definitions = readDefinitions(manifest);
    if (!definitions) return std::unexpected(definitions.error());
    E2AssetCatalog result;
    result.textures_.reserve(definitions->size());
    result.samplers_.reserve(4);
    result.assets_.reserve(definitions->size());
    std::unordered_map<std::string, std::size_t> texturesByPath;
    std::unordered_map<std::string, std::size_t> samplersByProfile;

    for (const auto& definition : *definitions) {
        if (result.assets_.contains(definition.id)) {
            return mf::fail(mf::ErrorCode::ioError, "Duplicate E2 asset ID: " + definition.id);
        }
        std::size_t textureIndex{};
        if (const auto found = texturesByPath.find(definition.path); found != texturesByPath.end()) {
            textureIndex = found->second;
        } else {
            auto image = mf::loadImage(assetsRoot / definition.path);
            if (!image) return std::unexpected(image.error());
            if (image->width != definition.sourceWidth || image->height != definition.sourceHeight) {
                return mf::fail(mf::ErrorCode::ioError, "E2 source dimensions do not match manifest: " + definition.path);
            }
            auto texture = device.createTexture({image->width, image->height, definition.path}, image->rgbaPixels);
            if (!texture) return std::unexpected(texture.error());
            textureIndex = result.textures_.size();
            texturesByPath.emplace(definition.path, textureIndex);
            result.textures_.push_back(std::move(*texture));
            result.textureBytes_ += static_cast<std::uint64_t>(image->width) * image->height * 4U;
        }

        const std::string samplerProfile = definition.filter + ':' + definition.wrap;
        std::size_t samplerIndex{};
        if (const auto found = samplersByProfile.find(samplerProfile); found != samplersByProfile.end()) {
            samplerIndex = found->second;
        } else {
            const auto filter = definition.filter == "nearest" ? mf::FilterMode::nearest : mf::FilterMode::linear;
            const auto wrap = definition.wrap == "repeat" ? mf::WrapMode::repeat : mf::WrapMode::clamp;
            auto sampler = device.createSampler({filter, wrap, wrap, samplerProfile});
            if (!sampler) return std::unexpected(sampler.error());
            samplerIndex = result.samplers_.size();
            samplersByProfile.emplace(samplerProfile, samplerIndex);
            result.samplers_.push_back(std::move(*sampler));
        }

        const float inverseWidth = 1.0F / static_cast<float>(definition.sourceWidth);
        const float inverseHeight = 1.0F / static_cast<float>(definition.sourceHeight);
        VisualAsset asset;
        asset.texture = &result.textures_[textureIndex];
        asset.sampler = &result.samplers_[samplerIndex];
        asset.uv = {definition.x * inverseWidth, definition.y * inverseHeight,
                    (definition.x + definition.width) * inverseWidth, (definition.y + definition.height) * inverseHeight};
        asset.sourceSize = {static_cast<float>(definition.width), static_cast<float>(definition.height)};
        asset.pivot = {definition.pivotX, definition.pivotY};
        asset.scale = definition.scale;
        asset.material = definition.material;
        asset.emissiveId = definition.emissive == "-" ? "" : definition.emissive;
        asset.normalId = definition.normal == "-" ? "" : definition.normal;
        asset.animationId = definition.animation == "-" ? "" : definition.animation;
        result.assets_.emplace(definition.id, std::move(asset));
    }
    result.loadMilliseconds_ = std::chrono::duration<float, std::milli>(
        std::chrono::steady_clock::now() - loadStart).count();
    return result;
}

const VisualAsset* E2AssetCatalog::find(std::string_view logicalId) const noexcept {
    const auto found = assets_.find(logicalId);
    return found == assets_.end() ? nullptr : &found->second;
}

const VisualAsset* E2AssetCatalog::find(E2AssetId logicalId) const noexcept {
    switch (logicalId) {
        case E2AssetId::interfaceFont: return find("ui.font.interface");
    }
    return nullptr;
}

} // namespace aegis::mediaforge
