#include "AssetCatalog.hpp"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>

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
    const std::array materials{"grass","dirt","rock","snow","ice","sand","ash","industrial"};
    for (const auto* name : materials) catalog.add({"terrain."+std::string(name)+".surface",AssetType::Texture,"terrain/"+std::string(name)+".png",true,true});
    const std::array environment{"tree","bush","rock_small","rock_large","crate","ruin","lamp","antenna","scrap","barricade"};
    for (const auto* name : environment) catalog.add({"environment."+std::string(name),AssetType::Texture,"environment/"+std::string(name)+".png"});
    catalog.add({"world.spawn_gate",AssetType::Texture,"world/spawn_gate.png"});
    catalog.add({"world.aegis_core",AssetType::Texture,"world/aegis_core.png"});
    catalog.add({"world.aegis_core_energy",AssetType::Texture,"world/aegis_core_energy.png"});
    catalog.add({"ui.panel.authored",AssetType::Texture,"ui/panel_authored.png"});
    const std::array panelRoles{"main","card","card_selected","button_primary","button_danger","tooltip","modal"};
    for(const auto* role:panelRoles)catalog.add({"ui.surface."+std::string(role),AssetType::Texture,"ui/panel_authored.png"});
    return catalog;
}

std::optional<AssetCatalog> AssetCatalog::loadManifest(const std::filesystem::path& file,std::string* error){
    std::ifstream input(file,std::ios::binary);
    if(!input){if(error)*error="Asset manifest not found: "+file.string();return std::nullopt;}
    std::string header; std::getline(input,header);
    if(header!="AEGIS_ASSETS_V1"){if(error)*error="Unsupported asset manifest header: "+header;return std::nullopt;}
    AssetCatalog result; std::string line; int lineNumber=1;
    while(std::getline(input,line)){
        ++lineNumber; if(line.empty()||line[0]=='#')continue;
        std::istringstream row(line); std::string type,id,path; int smooth=1,repeated=0;
        row>>type>>std::quoted(id)>>std::quoted(path)>>smooth>>repeated;
        AssetType assetType;
        if(type=="texture")assetType=AssetType::Texture; else if(type=="font")assetType=AssetType::Font; else if(type=="sound")assetType=AssetType::Sound;
        else{if(error)*error="Unknown asset type in line "+std::to_string(lineNumber);return std::nullopt;}
        if(!row||!result.add({id,assetType,path,smooth!=0,repeated!=0})){
            if(error)*error="Invalid or duplicate asset in line "+std::to_string(lineNumber);
            return std::nullopt;
        }
    }
    return result;
}

} // namespace aegis::assets
