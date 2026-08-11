#include "MapDocument.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace aegis::core {
namespace {
std::string quote(const std::string& s) {
    std::ostringstream out;
    out << std::quoted(s);
    return out.str();
}

bool finite(float v) { return std::isfinite(v); }
}

const char* toString(ZoneType type) {
    switch (type) {
        case ZoneType::Buildable: return "buildable";
        case ZoneType::Blocked: return "blocked";
        case ZoneType::Water: return "water";
        case ZoneType::DecorationOnly: return "decoration_only";
    }
    return "blocked";
}

std::optional<ZoneType> zoneTypeFromString(const std::string& value) {
    if (value == "buildable") return ZoneType::Buildable;
    if (value == "blocked") return ZoneType::Blocked;
    if (value == "water") return ZoneType::Water;
    if (value == "decoration_only") return ZoneType::DecorationOnly;
    return std::nullopt;
}

bool MapDocument::save(const std::string& file, std::string* error) const {
    if (metadata.formatVersion != CurrentMapFormatVersion) {
        if (error) *error = "Nicht unterstützte Map-Formatversion: " + std::to_string(metadata.formatVersion);
        return false;
    }
    std::ofstream out(file, std::ios::binary);
    if (!out) {
        if (error) *error = "Datei konnte nicht zum Schreiben geöffnet werden: " + file;
        return false;
    }

    out << "AEGIS_MAP_V1\n";
    out << "meta " << quote(metadata.id) << ' ' << quote(metadata.name) << ' ' << quote(metadata.subtitle) << ' '
        << quote(metadata.description) << ' ' << quote(metadata.biome) << ' ' << quote(metadata.author) << ' '
        << quote(metadata.difficulty) << "\n";
    out << "size " << width << ' ' << height << "\n";
    out << "environment " << quote(environment.weather) << ' ' << environment.timeOfDay << ' '
        << environment.ambientIntensity << ' ' << environment.terrainSeed << "\n";
    out << "waves " << quote(wavePreset) << "\n";

    for (const auto& path : paths) {
        out << "path " << quote(path.id) << ' ' << path.nodes.size();
        for (const auto& p : path.nodes) out << ' ' << p.x << ' ' << p.y;
        out << "\n";
    }
    for (const auto& spawn : spawns)
        out << "spawn " << quote(spawn.id) << ' ' << quote(spawn.pathId) << ' ' << spawn.position.x << ' ' << spawn.position.y << "\n";
    for (const auto& goal : goals)
        out << "goal " << quote(goal.id) << ' ' << quote(goal.pathId) << ' ' << goal.position.x << ' ' << goal.position.y << "\n";
    for (const auto& zone : zones)
        out << "zone " << toString(zone.type) << ' ' << zone.rect.x << ' ' << zone.rect.y << ' ' << zone.rect.w << ' ' << zone.rect.h << "\n";
    for (const auto& deco : decorations)
        out << "deco " << quote(deco.assetId) << ' ' << deco.position.x << ' ' << deco.position.y << ' '
            << deco.rotationDeg << ' ' << deco.scale << ' ' << deco.layer << "\n";

    if (!out.good()) {
        if (error) *error = "Fehler beim Schreiben der Map-Datei: " + file;
        return false;
    }
    return true;
}

std::optional<MapDocument> MapDocument::load(const std::string& file, std::string* error) {
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        if (error) *error = "Map-Datei nicht gefunden: " + file;
        return std::nullopt;
    }

    std::string header;
    std::getline(in, header);
    if (header != "AEGIS_MAP_V1") {
        if (error) *error = "Unbekanntes Map-Format: " + header;
        return std::nullopt;
    }

    MapDocument doc;
    doc.metadata.formatVersion = CurrentMapFormatVersion;
    std::string line;
    int lineNo = 1;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty() || line[0] == '#') continue;
        std::istringstream row(line);
        std::string key;
        row >> key;
        if (key == "meta") {
            row >> std::quoted(doc.metadata.id) >> std::quoted(doc.metadata.name) >> std::quoted(doc.metadata.subtitle)
                >> std::quoted(doc.metadata.description) >> std::quoted(doc.metadata.biome) >> std::quoted(doc.metadata.author)
                >> std::quoted(doc.metadata.difficulty);
        } else if (key == "size") {
            row >> doc.width >> doc.height;
        } else if (key == "environment") {
            row >> std::quoted(doc.environment.weather) >> doc.environment.timeOfDay >> doc.environment.ambientIntensity >> doc.environment.terrainSeed;
        } else if (key == "waves") {
            row >> std::quoted(doc.wavePreset);
        } else if (key == "path") {
            Path path;
            std::size_t count = 0;
            row >> std::quoted(path.id) >> count;
            path.nodes.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                Vec2 p;
                row >> p.x >> p.y;
                path.nodes.push_back(p);
            }
            doc.paths.push_back(std::move(path));
        } else if (key == "spawn") {
            SpawnPoint s;
            row >> std::quoted(s.id) >> std::quoted(s.pathId) >> s.position.x >> s.position.y;
            doc.spawns.push_back(std::move(s));
        } else if (key == "goal") {
            GoalPoint g;
            row >> std::quoted(g.id) >> std::quoted(g.pathId) >> g.position.x >> g.position.y;
            doc.goals.push_back(std::move(g));
        } else if (key == "zone") {
            std::string type;
            Zone z;
            row >> type >> z.rect.x >> z.rect.y >> z.rect.w >> z.rect.h;
            auto parsed = zoneTypeFromString(type);
            if (!parsed) {
                if (error) *error = "Ungültiger Zonentyp in Zeile " + std::to_string(lineNo);
                return std::nullopt;
            }
            z.type = *parsed;
            doc.zones.push_back(z);
        } else if (key == "deco") {
            Decoration d;
            row >> std::quoted(d.assetId) >> d.position.x >> d.position.y >> d.rotationDeg >> d.scale >> d.layer;
            doc.decorations.push_back(std::move(d));
        } else {
            if (error) *error = "Unbekannter Eintrag in Zeile " + std::to_string(lineNo) + ": " + key;
            return std::nullopt;
        }

        if (!row) {
            if (error) *error = "Fehlerhafte Map-Daten in Zeile " + std::to_string(lineNo);
            return std::nullopt;
        }
    }

    return doc;
}

std::vector<ValidationIssue> MapDocument::validateDetailed() const {
    std::vector<ValidationIssue> issues;
    auto add = [&](ValidationSeverity severity, MapObjectType type, std::string id, std::size_t index, std::string message) {
        issues.push_back({severity, type, std::move(id), index, std::move(message)});
    };
    if (metadata.id.empty()) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Map-ID fehlt.");
    if (metadata.name.empty()) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Map-Name fehlt.");
    if (metadata.formatVersion != CurrentMapFormatVersion) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Map-Formatversion wird nicht unterstützt.");
    if (!finite(width) || !finite(height) || width < 320.f || height < 240.f)
        add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Kartengröße ist ungültig.");
    if (paths.empty()) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Mindestens ein Gegnerpfad ist erforderlich.");

    std::vector<std::string> pathIds;
    for (std::size_t pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
        const auto& path = paths[pathIndex];
        if (path.id.empty()) add(ValidationSeverity::Error, MapObjectType::Path, {}, pathIndex, "Ein Pfad besitzt keine ID.");
        if (std::find(pathIds.begin(), pathIds.end(), path.id) != pathIds.end())
            add(ValidationSeverity::Error, MapObjectType::Path, path.id, pathIndex, "Pfad-ID '" + path.id + "' ist doppelt vergeben.");
        pathIds.push_back(path.id);
        if (path.nodes.size() < 2)
            add(ValidationSeverity::Error, MapObjectType::Path, path.id, pathIndex, "Pfad '" + path.id + "' benötigt mindestens zwei Punkte.");
        for (std::size_t nodeIndex = 0; nodeIndex < path.nodes.size(); ++nodeIndex) {
            const auto& p = path.nodes[nodeIndex];
            if (!finite(p.x) || !finite(p.y) || p.x < 0.f || p.y < 0.f || p.x > width || p.y > height)
                add(ValidationSeverity::Error, MapObjectType::PathPoint, path.id, nodeIndex,
                    "Pfad '" + path.id + "' besitzt einen Punkt außerhalb der Karte.");
        }
    }

    auto hasPath = [&](const std::string& id) {
        return std::any_of(paths.begin(), paths.end(), [&](const Path& p) { return p.id == id; });
    };
    if (spawns.empty()) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Mindestens ein Spawnpunkt ist erforderlich.");
    if (goals.empty()) add(ValidationSeverity::Error, MapObjectType::Map, {}, 0, "Mindestens ein Zielpunkt ist erforderlich.");
    auto insideMap = [&](Vec2 point) {
        return finite(point.x) && finite(point.y) && point.x >= 0.f && point.y >= 0.f && point.x <= width && point.y <= height;
    };
    std::vector<std::string> spawnIds;
    for (std::size_t i = 0; i < spawns.size(); ++i) {
        const auto& spawn = spawns[i];
        if (spawn.id.empty()) add(ValidationSeverity::Error, MapObjectType::Spawn, {}, i, "Ein Spawnpunkt besitzt keine ID.");
        if (std::find(spawnIds.begin(), spawnIds.end(), spawn.id) != spawnIds.end())
            add(ValidationSeverity::Error, MapObjectType::Spawn, spawn.id, i, "Spawn-ID '" + spawn.id + "' ist doppelt vergeben.");
        spawnIds.push_back(spawn.id);
        if (!hasPath(spawn.pathId)) add(ValidationSeverity::Error, MapObjectType::Spawn, spawn.id, i, "Spawn '" + spawn.id + "' verweist auf einen unbekannten Pfad.");
        if (!insideMap(spawn.position)) add(ValidationSeverity::Error, MapObjectType::Spawn, spawn.id, i, "Spawn '" + spawn.id + "' liegt außerhalb der Karte.");
    }
    std::vector<std::string> goalIds;
    for (std::size_t i = 0; i < goals.size(); ++i) {
        const auto& goal = goals[i];
        if (goal.id.empty()) add(ValidationSeverity::Error, MapObjectType::Goal, {}, i, "Ein Zielpunkt besitzt keine ID.");
        if (std::find(goalIds.begin(), goalIds.end(), goal.id) != goalIds.end())
            add(ValidationSeverity::Error, MapObjectType::Goal, goal.id, i, "Ziel-ID '" + goal.id + "' ist doppelt vergeben.");
        goalIds.push_back(goal.id);
        if (!hasPath(goal.pathId)) add(ValidationSeverity::Error, MapObjectType::Goal, goal.id, i, "Ziel '" + goal.id + "' verweist auf einen unbekannten Pfad.");
        if (!insideMap(goal.position)) add(ValidationSeverity::Error, MapObjectType::Goal, goal.id, i, "Ziel '" + goal.id + "' liegt außerhalb der Karte.");
    }

    bool hasBuildable = false;
    for (std::size_t i = 0; i < zones.size(); ++i) {
        const auto& zone = zones[i];
        hasBuildable = hasBuildable || zone.type == ZoneType::Buildable;
        if (!finite(zone.rect.x) || !finite(zone.rect.y) || !finite(zone.rect.w) || !finite(zone.rect.h) ||
            zone.rect.w <= 0.f || zone.rect.h <= 0.f) {
            add(ValidationSeverity::Error, MapObjectType::Zone, {}, i, "Eine Zone besitzt keine gültige positive Größe.");
        } else if (zone.rect.x < 0.f || zone.rect.y < 0.f || zone.rect.x + zone.rect.w > width || zone.rect.y + zone.rect.h > height) {
            add(ValidationSeverity::Error, MapObjectType::Zone, {}, i, "Eine Zone liegt außerhalb der Karte.");
        }
    }
    if (!hasBuildable) add(ValidationSeverity::Warning, MapObjectType::Map, {}, 0, "Keine explizite Bauzone vorhanden; freie Flächen bleiben bebaubar.");
    for (std::size_t i = 0; i < decorations.size(); ++i) {
        const auto& decoration = decorations[i];
        if (decoration.assetId.empty()) add(ValidationSeverity::Error, MapObjectType::Decoration, {}, i, "Eine Dekoration besitzt keine Asset-ID.");
        if (!insideMap(decoration.position)) add(ValidationSeverity::Error, MapObjectType::Decoration, decoration.assetId, i, "Dekoration '" + decoration.assetId + "' liegt außerhalb der Karte.");
        if (!finite(decoration.rotationDeg) || !finite(decoration.scale) || decoration.scale <= 0.f)
            add(ValidationSeverity::Error, MapObjectType::Decoration, decoration.assetId, i, "Dekoration '" + decoration.assetId + "' besitzt eine ungültige Transformation.");
    }
    return issues;
}

std::vector<std::string> MapDocument::validate() const {
    std::vector<std::string> messages;
    for (const auto& issue : validateDetailed()) messages.push_back(issue.message);
    return messages;
}

} // namespace aegis::core
