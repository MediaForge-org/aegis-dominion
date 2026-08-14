#include "MapCatalog.hpp"

#include <algorithm>
#include <system_error>
#include <unordered_map>

namespace aegis::core {
namespace {

MapCatalogEntry loadEntry(const std::filesystem::path& file, MapSource source) {
    MapCatalogEntry entry;
    entry.file = file;
    entry.source = source;
    std::string error;
    entry.document = MapDocument::load(file.string(), &error);
    if (!entry.document) {
        entry.error = std::move(error);
        return entry;
    }
    entry.playable = buildPlayableMap(*entry.document, &error);
    if (!entry.playable) entry.error = std::move(error);
    return entry;
}

bool isBuiltInFilename(const std::filesystem::path& file) {
    const auto filename = file.filename().string();
    const auto names = builtInMapFilenames();
    return std::ranges::find(names, filename) != names.end();
}

} // namespace

std::string MapCatalogEntry::stableId() const {
    if (document && !document->metadata.id.empty()) return document->metadata.id;
    return file.stem().string();
}

std::string MapCatalogEntry::displayName() const {
    if (document && !document->metadata.name.empty()) return document->metadata.name;
    return file.stem().string();
}

std::vector<MapCatalogEntry> discoverMapCatalog(const std::filesystem::path& mapsDirectory) {
    std::vector<MapCatalogEntry> entries;
    const auto builtIns = builtInMapFilenames();
    entries.reserve(builtIns.size());
    for (const auto filename : builtIns) entries.push_back(loadEntry(mapsDirectory / filename, MapSource::BuiltIn));

    std::error_code error;
    std::vector<std::filesystem::path> customFiles;
    if (std::filesystem::is_directory(mapsDirectory, error)) {
        for (std::filesystem::directory_iterator iterator(mapsDirectory, error), end; iterator != end && !error;
             iterator.increment(error)) {
            const auto& item = *iterator;
            if (!item.is_regular_file(error) || error || item.path().extension() != ".aegismap" ||
                isBuiltInFilename(item.path())) continue;
            customFiles.push_back(item.path());
        }
    }
    std::ranges::sort(customFiles, {}, [](const auto& file) { return file.filename().string(); });
    for (const auto& file : customFiles) entries.push_back(loadEntry(file, MapSource::Custom));

    std::unordered_map<std::string, std::size_t> idCounts;
    for (const auto& entry : entries) {
        if (entry.document && !entry.document->metadata.id.empty()) ++idCounts[entry.document->metadata.id];
    }
    for (auto& entry : entries) {
        if (!entry.document || entry.document->metadata.id.empty() || idCounts[entry.document->metadata.id] < 2)
            continue;
        entry.error = "Map-ID '" + entry.document->metadata.id + "' ist bereits vergeben.";
        entry.playable.reset();
    }
    return entries;
}

} // namespace aegis::core
