#pragma once
#include "Common.hpp"
#include "core/PlayableMap.hpp"
#include <vector>
#include <string>

struct GameMapZone {
    aegis::core::ZoneType type = aegis::core::ZoneType::Buildable;
    sf::FloatRect rect;
};

struct MapData {
    std::string id;
    std::string name;
    std::string subtitle;
    std::string description;
    std::string biome;
    sf::Color accent;
    std::vector<sf::Vector2f> path;
    sf::Vector2f spawn;
    sf::Vector2f goal;
    std::vector<GameMapZone> zones;
    float sourceWidth = WORLD_W;
    float sourceHeight = WORLD_H;
    bool authoredBackground = true;
};

class GameMap {
public:
    void set(int index);
    bool loadFromFile(const std::string& file, int visualIndex = 0);
    void loadFromPlayableMap(const aegis::core::PlayableMap& map);
    int index() const { return index_; }
    const MapData& data() const { return data_; }
    const std::vector<sf::Vector2f>& path() const { return data_.path; }
    float totalLength() const { return totalLength_; }
    bool canBuild(sf::Vector2f p, const std::vector<sf::Vector2f>& towerPositions, float radius=42.f) const;
    float pathProgress(std::size_t segment, float segmentT) const;
private:
    void rebuildLengths();
    void applyDocument(const aegis::core::MapDocument& document, int visualIndex);
    void applyPlayableMap(const aegis::core::PlayableMap& map, int visualIndex, bool authoredBackground);
    void loadLegacyFallback(int index);
    float distanceToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
    int index_=0;
    MapData data_;
    std::vector<float> segmentLengths_;
    float totalLength_=1.f;
};
