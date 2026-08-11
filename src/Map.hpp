#pragma once
#include "Common.hpp"
#include "core/MapDocument.hpp"
#include <vector>
#include <string>

struct MapData {
    std::string id;
    std::string name;
    std::string subtitle;
    std::string description;
    std::string biome;
    sf::Color accent;
    std::vector<sf::Vector2f> path;
};

class GameMap {
public:
    void set(int index);
    bool loadFromFile(const std::string& file, int visualIndex = 0);
    int index() const { return index_; }
    const MapData& data() const { return data_; }
    const std::vector<sf::Vector2f>& path() const { return data_.path; }
    float totalLength() const { return totalLength_; }
    bool canBuild(sf::Vector2f p, const std::vector<sf::Vector2f>& towerPositions, float radius=42.f) const;
    float pathProgress(std::size_t segment, float segmentT) const;
    const aegis::core::MapDocument* document() const { return document_ ? &*document_ : nullptr; }
private:
    void rebuildLengths();
    void applyDocument(const aegis::core::MapDocument& document, int visualIndex);
    void loadLegacyFallback(int index);
    float distanceToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
    int index_=0;
    MapData data_;
    std::optional<aegis::core::MapDocument> document_;
    std::vector<float> segmentLengths_;
    float totalLength_=1.f;
};
