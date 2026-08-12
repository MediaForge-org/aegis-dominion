#pragma once

#include "AssetId.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace aegis::assets {

struct AssetDefinition {
    std::string id;
    AssetType type = AssetType::Texture;
    std::filesystem::path relativePath;
    bool smooth = true;
    bool repeated = false;
};

class AssetCatalog {
public:
    bool add(AssetDefinition definition);
    const AssetDefinition* find(std::string_view id, AssetType type) const;
    const std::vector<AssetDefinition>& definitions() const { return definitions_; }
    static AssetCatalog builtIn();

private:
    std::vector<AssetDefinition> definitions_;
    std::unordered_map<std::string, std::size_t> indices_;
};

} // namespace aegis::assets
