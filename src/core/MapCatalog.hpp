#pragma once

#include "MapDocument.hpp"
#include "PlayableMap.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace aegis::core {

enum class MapSource { BuiltIn, Custom };

struct MapCatalogEntry {
    std::filesystem::path file;
    MapSource source = MapSource::Custom;
    std::optional<MapDocument> document;
    std::optional<PlayableMap> playable;
    std::string error;

    [[nodiscard]] bool valid() const noexcept { return playable.has_value(); }
    [[nodiscard]] std::string stableId() const;
    [[nodiscard]] std::string displayName() const;
};

[[nodiscard]] constexpr std::array<std::string_view, 3> builtInMapFilenames() noexcept {
    return {"verdant.aegismap", "frost.aegismap", "ember.aegismap"};
}

[[nodiscard]] std::vector<MapCatalogEntry> discoverMapCatalog(const std::filesystem::path& mapsDirectory);

} // namespace aegis::core
