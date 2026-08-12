#include "AssetCatalog.hpp"

#include <array>

namespace aegis::assets {

bool AssetCatalog::add(AssetDefinition definition) {
    if (definition.id.empty() || definition.id.find('/') != std::string::npos || indices_.contains(definition.id)) return false;
    indices_.emplace(definition.id, definitions_.size());
    definitions_.push_back(std::move(definition));
    return true;
}

const AssetDefinition* AssetCatalog::find(std::string_view id, AssetType type) const {
    const auto found = indices_.find(std::string(id));
    if (found == indices_.end()) return nullptr;
    const auto& definition = definitions_[found->second];
    return definition.type == type ? &definition : nullptr;
}

AssetCatalog AssetCatalog::builtIn() {
    AssetCatalog catalog;
    const std::array towerNames{"pulse", "cannon", "frost", "sniper", "tesla", "missile"};
    for (const auto* name : towerNames) {
        catalog.add({"tower." + std::string(name) + ".base", AssetType::Texture, "towers/" + std::string(name) + "_base.png"});
        catalog.add({"tower." + std::string(name) + ".turret", AssetType::Texture, "towers/" + std::string(name) + "_turret.png"});
    }
    const std::array enemyNames{"raider", "runner", "tank", "shield", "regen", "splitter", "boss"};
    for (const auto* name : enemyNames)
        catalog.add({"enemy." + std::string(name) + ".idle", AssetType::Texture, "enemies/" + std::string(name) + ".png"});
    const std::array mapNames{"verdant", "frost", "ember"};
    for (const auto* name : mapNames)
        catalog.add({"environment." + std::string(name) + ".background", AssetType::Texture, "maps/" + std::string(name) + ".png"});
    const std::array uiNames{"menu_background", "logo", "credits", "core", "wave", "score"};
    for (const auto* name : uiNames)
        catalog.add({"ui." + std::string(name), AssetType::Texture, "ui/" + std::string(name) + ".png"});
    const std::array soundNames{"click", "build", "upgrade", "shoot", "laser", "explosion", "wave", "gameover"};
    for (const auto* name : soundNames)
        catalog.add({"sfx." + std::string(name), AssetType::Sound, "sfx/" + std::string(name) + ".wav", false});
    catalog.add({"font.ui.default", AssetType::Font, "fonts/NotoSans-Regular.ttf", false});
    return catalog;
}

} // namespace aegis::assets
