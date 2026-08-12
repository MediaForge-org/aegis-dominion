#include "animation/AnimationCatalog.hpp"
#include "assets/AssetCatalog.hpp"
#include "core/PathCurve.hpp"
#include "render/RenderSnapshot.hpp"
#include "render/WorldRenderData.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool value,const std::string& message){if(!value)throw std::runtime_error(message);}
bool near(float a,float b){return std::abs(a-b)<.01f;}

aegis::render::MapRenderSnapshot mapSnapshot(unsigned seed=42){
    aegis::render::MapRenderSnapshot map;map.id="phase4_test";map.biome="verdant";map.terrainSeed=seed;
    map.path={{0.f,300.f},{300.f,300.f},{520.f,520.f},{900.f,520.f},{1200.f,700.f}};map.spawn={0.f,300.f};map.goal={1200.f,700.f};
    map.zones.push_back({aegis::core::ZoneType::Water,{650.f,80.f,160.f,120.f}});return map;
}

void pathCurveEndpointsAndDensity(){
    const std::vector<aegis::core::Vec2> nodes={{0,0},{100,0},{100,100},{240,140}};
    const auto curve=aegis::core::samplePathCurve(nodes,10.f);
    require(curve.size()>nodes.size()*3,"curve was not sampled densely");
    require(near(curve.front().x,0)&&near(curve.front().y,0)&&near(curve.back().x,240)&&near(curve.back().y,140),"curve endpoints changed");
    bool curved=false;for(const auto point:curve)if(point.x>60.f&&point.x<100.f&&point.y<0.f)curved=true;
    require(curved,"Catmull-Rom corner has no interpolation curvature");
}

void materialMapping(){
    using namespace aegis::render;
    require(terrainMaterialForBiome("verdant")==TerrainMaterial::Grass,"verdant material mismatch");
    require(terrainMaterialForBiome("frost")==TerrainMaterial::Snow,"frost material mismatch");
    require(terrainMaterialForBiome("volcanic")==TerrainMaterial::Ash,"volcanic material mismatch");
    require(std::string(terrainTextureId(TerrainMaterial::Industrial))=="terrain.industrial.surface","industrial texture mapping missing");
}

void gameplayZoneVisibility(){
    using namespace aegis::render;
    require(zoneVisibleInGameplay(aegis::core::ZoneType::Water,false,false),"water hidden in gameplay");
    require(!zoneVisibleInGameplay(aegis::core::ZoneType::Buildable,false,false),"build zone leaked into ordinary gameplay");
    require(zoneVisibleInGameplay(aegis::core::ZoneType::Buildable,true,false),"build zone hidden during construction");
    require(!zoneVisibleInGameplay(aegis::core::ZoneType::Blocked,false,false)&&zoneVisibleInGameplay(aegis::core::ZoneType::Blocked,false,true),"blocked debug visibility rule broken");
}

void deterministicDecorations(){
    const auto first=aegis::render::generateProceduralDecorations(mapSnapshot(91),36);
    const auto repeat=aegis::render::generateProceduralDecorations(mapSnapshot(91),36);
    const auto changed=aegis::render::generateProceduralDecorations(mapSnapshot(92),36);
    require(first.size()==36&&repeat.size()==first.size(),"deterministic generator did not fill requested layer");
    for(std::size_t i=0;i<first.size();++i)require(first[i].visualId==repeat[i].visualId&&near(first[i].position.x,repeat[i].position.x)&&near(first[i].position.y,repeat[i].position.y),"same seed changed decoration layout");
    require(!near(first.front().position.x,changed.front().position.x)||!near(first.front().position.y,changed.front().position.y),"different seed produced identical layout");
}

void authoredDecorationsWin(){
    auto map=mapSnapshot();map.decorations.push_back({"environment.crate",{200,200},17.f,1.1f,0,aegis::render::Layer::Environment});
    const auto result=aegis::render::generateProceduralDecorations(map,50);
    require(result.size()==1&&result.front().visualId=="environment.crate","authored decoration was replaced by procedural content");
}

void externalAssetManifest(){
    std::string error;const auto catalog=aegis::assets::AssetCatalog::loadManifest(std::filesystem::path(AEGIS_SOURCE_DIR)/"assets/manifest.aegis",&error);
    require(catalog.has_value(),error);require(catalog->definitions().size()>=55,"external manifest is incomplete");
    for(const auto& definition:catalog->definitions())if(definition.id!="font.ui.default")require(std::filesystem::exists(std::filesystem::path(AEGIS_SOURCE_DIR)/"assets"/definition.relativePath),"manifest path missing for "+definition.id);
    const auto* terrain=catalog->find("terrain.grass.surface",aegis::assets::AssetType::Texture);
    require(terrain&&terrain->repeated,"terrain repeat metadata was not loaded");
    require(catalog->find("environment.not_registered",aegis::assets::AssetType::Texture)==nullptr,"missing manifest entry resolved unexpectedly");
}

void invalidManifestRejected(){
    const auto file=std::filesystem::temp_directory_path()/"aegis_invalid_assets.aegis";
    {std::ofstream out(file);out<<"AEGIS_ASSETS_V1\ntexture \"duplicate\" \"a.png\" 1 0\ntexture \"duplicate\" \"b.png\" 1 0\n";}
    std::string error;require(!aegis::assets::AssetCatalog::loadManifest(file,&error)&&error.find("duplicate")!=std::string::npos,"duplicate manifest entry accepted");
    std::filesystem::remove(file);
}

void externalAnimations(){
    aegis::animation::AnimationCatalog animations;std::string error;
    require(animations.load(std::filesystem::path(AEGIS_SOURCE_DIR)/"assets/animations.aegis",&error),error);
    const auto* move=animations.find("enemy.runner.move");const auto* spawn=animations.find("enemy.raider.spawn");
    require(animations.size()>=10&&move&&move->animation.playback==aegis::animation::Playback::Loop,"move animation definition missing");
    require(spawn&&spawn->animation.playback==aegis::animation::Playback::OneShot,"spawn playback definition missing");
}

void snapshotIsolation(){
    const auto file=std::filesystem::path(AEGIS_SOURCE_DIR)/"src/render/RenderSnapshot.hpp";std::ifstream input(file);const std::string source((std::istreambuf_iterator<char>(input)),{});
    require(source.find("SFML/")==std::string::npos&&source.find("Enemy.hpp")==std::string::npos&&source.find("Tower.hpp")==std::string::npos,"snapshot contract depends on renderer/gameplay classes");
    aegis::render::EnemyRenderSnapshot enemy;require(enemy.layer==aegis::render::Layer::Enemies,"enemy snapshot default layer incorrect");
}

} // namespace

int main(){
    const std::vector<std::pair<std::string,std::function<void()>>> tests={{"path interpolation",pathCurveEndpointsAndDensity},{"material mapping",materialMapping},{"zone visibility",gameplayZoneVisibility},{"deterministic decoration",deterministicDecorations},{"authored decoration",authoredDecorationsWin},{"asset manifest",externalAssetManifest},{"manifest errors",invalidManifestRejected},{"animation definitions",externalAnimations},{"snapshot isolation",snapshotIsolation}};
    try{for(const auto& test:tests)test.second();std::cout<<tests.size()<<" Phase-4 rendering checks passed\n";return 0;}catch(const std::exception& error){std::cerr<<"TEST FAILURE: "<<error.what()<<'\n';return 1;}
}
