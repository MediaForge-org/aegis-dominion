#pragma once
#include "Common.hpp"
#include "core/PlayableMap.hpp"
#include "render/RenderSnapshot.hpp"
#include <vector>
#include <string>

struct GameMapZone {
    aegis::core::ZoneType type = aegis::core::ZoneType::Buildable;
    sf::FloatRect rect;
};

struct GameMapDecoration {
    std::string assetId;
    sf::Vector2f position;
    float rotationDeg = 0.f;
    float scale = 1.f;
    int layer = 0;
};

struct MapData {
    std::string id;
    std::string name;
    std::string subtitle;
    std::string description;
    std::string biome;
    sf::Color accent;
    std::vector<sf::Vector2f> logicalPath;
    std::vector<sf::Vector2f> path;
    sf::Vector2f spawn;
    sf::Vector2f goal;
    std::vector<GameMapZone> zones;
    std::vector<GameMapDecoration> decorations;
    unsigned terrainSeed = 1;
    float ambientIntensity = 1.f;
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
    aegis::render::MapRenderSnapshot renderSnapshot(bool buildMode = false, bool debugZones = false) const;
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
