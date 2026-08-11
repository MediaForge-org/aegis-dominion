#include "PlayableMap.hpp"

#include <algorithm>
#include <sstream>

namespace aegis::core {

std::optional<PlayableMap> buildPlayableMap(const MapDocument& document, std::string* error) {
    std::ostringstream errors;
    bool fatal = false;
    for (const auto& issue : document.validateDetailed()) {
        if (issue.severity != ValidationSeverity::Error) continue;
        fatal = true;
        errors << (errors.tellp() > 0 ? "\n" : "") << issue.message;
    }
    if (fatal) {
        if (error) *error = errors.str();
        return std::nullopt;
    }

    const Path* selectedPath = nullptr;
    for (const auto& path : document.paths) {
        const bool hasSpawn = std::any_of(document.spawns.begin(), document.spawns.end(), [&](const SpawnPoint& spawn) { return spawn.pathId == path.id; });
        const bool hasGoal = std::any_of(document.goals.begin(), document.goals.end(), [&](const GoalPoint& goal) { return goal.pathId == path.id; });
        if (hasSpawn && hasGoal) { selectedPath = &path; break; }
    }
    if (!selectedPath) {
        if (error) *error = "Kein Pfad besitzt sowohl Spawn als auch Ziel.";
        return std::nullopt;
    }
    const auto& path = *selectedPath;
    PlayableMap result;
    result.id = document.metadata.id;
    result.name = document.metadata.name;
    result.subtitle = document.metadata.subtitle;
    result.description = document.metadata.description;
    result.biome = document.metadata.biome;
    result.width = document.width;
    result.height = document.height;
    result.routeId = path.id;
    result.route = path.nodes;
    result.zones = document.zones;

    for (const auto& spawn : document.spawns) {
        if (spawn.pathId == path.id) {
            result.spawn = spawn.position;
            break;
        }
    }
    for (const auto& goal : document.goals) {
        if (goal.pathId == path.id) {
            result.goal = goal.position;
            break;
        }
    }
    return result;
}

} // namespace aegis::core
