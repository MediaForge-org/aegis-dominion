#pragma once
#include <string>
#include <vector>
#include <optional>

namespace aegis::core {

inline constexpr int CurrentMapFormatVersion = 1;

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
};

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

enum class ZoneType { Buildable, Blocked, Water, DecorationOnly };

enum class ValidationSeverity { Error, Warning };
enum class MapObjectType { Map, Path, PathPoint, Spawn, Goal, Zone, Decoration };

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Error;
    MapObjectType objectType = MapObjectType::Map;
    std::string objectId;
    std::size_t index = 0;
    std::string message;
};

struct Zone {
    ZoneType type = ZoneType::Buildable;
    Rect rect;
};

struct Path {
    std::string id = "main";
    std::vector<Vec2> nodes;
};

struct SpawnPoint {
    std::string id = "spawn_0";
    Vec2 position;
    std::string pathId = "main";
};

struct GoalPoint {
    std::string id = "goal_0";
    Vec2 position;
    std::string pathId = "main";
};

struct Decoration {
    std::string assetId;
    Vec2 position;
    float rotationDeg = 0.f;
    float scale = 1.f;
    int layer = 0;
};

struct MapMetadata {
    int formatVersion = CurrentMapFormatVersion;
    std::string id;
    std::string name;
    std::string subtitle;
    std::string description;
    std::string biome = "verdant";
    std::string author = "AEGIS DOMINION";
    std::string difficulty = "Normal";
};

struct EnvironmentSettings {
    std::string weather = "Clear";
    float timeOfDay = 14.f;
    float ambientIntensity = 1.f;
    unsigned terrainSeed = 1;
};

struct MapDocument {
    MapMetadata metadata;
    float width = 1200.f;
    float height = 900.f;
    std::vector<Path> paths;
    std::vector<SpawnPoint> spawns;
    std::vector<GoalPoint> goals;
    std::vector<Zone> zones;
    std::vector<Decoration> decorations;
    EnvironmentSettings environment;
    std::string wavePreset = "campaign_default";

    bool save(const std::string& file, std::string* error = nullptr) const;
    static std::optional<MapDocument> load(const std::string& file, std::string* error = nullptr);
    std::vector<ValidationIssue> validateDetailed() const;
    std::vector<std::string> validate() const;
};

const char* toString(ZoneType type);
std::optional<ZoneType> zoneTypeFromString(const std::string& value);

} // namespace aegis::core
