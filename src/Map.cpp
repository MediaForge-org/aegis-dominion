#include "Map.hpp"
#include "core/PlayableMap.hpp"
#include <array>
#include <iostream>

namespace {
sf::Color accentForBiome(const std::string& biome) {
    if (biome == "frost") return sf::Color(110,221,255);
    if (biome == "ember") return sf::Color(255,130,70);
    return sf::Color(92,196,139);
}
}

void GameMap::rebuildLengths(){
    segmentLengths_.clear();
    totalLength_=0.f;
    for(std::size_t i=0;i+1<data_.path.size();++i){
        float l=distance(data_.path[i],data_.path[i+1]);
        segmentLengths_.push_back(l);
        totalLength_+=l;
    }
    if(totalLength_<0.001f) totalLength_=1.f;
}

void GameMap::applyPlayableMap(const aegis::core::PlayableMap& map,int visualIndex,bool authoredBackground){
    index_=authoredBackground?std::max(0,std::min(2,visualIndex)):-1;
    data_={};
    data_.id=map.id;
    data_.name=map.name;
    data_.subtitle=map.subtitle;
    data_.description=map.description;
    data_.biome=map.biome;
    data_.accent=accentForBiome(map.biome);
    data_.sourceWidth=map.width;
    data_.sourceHeight=map.height;
    data_.authoredBackground=authoredBackground;
    const float scaleX=WORLD_W/map.width;
    const float scaleY=WORLD_H/map.height;
    auto project=[&](aegis::core::Vec2 point){return sf::Vector2f(point.x*scaleX,point.y*scaleY);};
    data_.path.reserve(map.route.size());
    for(const auto& point:map.route)data_.path.push_back(project(point));
    data_.spawn=project(map.spawn);
    data_.goal=project(map.goal);
    data_.zones.reserve(map.zones.size());
    for(const auto& zone:map.zones)data_.zones.push_back({zone.type,{zone.rect.x*scaleX,zone.rect.y*scaleY,zone.rect.w*scaleX,zone.rect.h*scaleY}});
    rebuildLengths();
}

void GameMap::applyDocument(const aegis::core::MapDocument& document,int visualIndex){
    std::string error;
    const auto playable=aegis::core::buildPlayableMap(document,&error);
    if(!playable){
        std::cerr << "Map konnte nicht in Gameplay-Daten konvertiert werden:\n" << error << "\n";
        loadLegacyFallback(visualIndex);
        return;
    }
    applyPlayableMap(*playable,visualIndex,true);
}

void GameMap::loadFromPlayableMap(const aegis::core::PlayableMap& map){
    applyPlayableMap(map,-1,false);
}

bool GameMap::loadFromFile(const std::string& file, int visualIndex){
    std::string error;
    auto loaded=aegis::core::MapDocument::load(file,&error);
    if(!loaded){
        std::cerr << "Map konnte nicht geladen werden: " << file << "\n" << error << "\n";
        return false;
    }
    applyDocument(*loaded,visualIndex);
    return true;
}

void GameMap::set(int index){
    static const std::array<const char*,3> files={"maps/verdant.aegismap","maps/frost.aegismap","maps/ember.aegismap"};
    index_=std::max(0,std::min(2,index));
    if(loadFromFile(files[static_cast<std::size_t>(index_)],index_)) return;
    loadLegacyFallback(index_);
}

void GameMap::loadLegacyFallback(int index){
    index_=std::max(0,std::min(2,index));
    data_={};
    if(index_==0){
        data_.id="verdant_frontier";data_.name="Grüne Grenze";data_.subtitle="Ausgewogen";data_.description="Weite Bauflächen und ein gut lesbarer Pfad. Ideal zum Lernen.";data_.biome="verdant";data_.accent=sf::Color(92,196,139);
        data_.path={{0,145},{180,145},{180,330},{500,330},{500,160},{780,160},{780,560},{1060,560},{1060,760},{1200,760}};
    } else if(index_==1){
        data_.id="frost_pass";data_.name="Frostpass";data_.subtitle="Taktisch";data_.description="Lange Geraden wechseln mit engen Kurven. Reichweite ist hier besonders wertvoll.";data_.biome="frost";data_.accent=sf::Color(110,221,255);
        data_.path={{0,690},{210,690},{210,480},{420,480},{420,720},{690,720},{690,400},{920,400},{920,190},{1200,190}};
    } else {
        data_.id="ember_field";data_.name="Aschefeld";data_.subtitle="Schwer";data_.description="Wenig sichere Fläche und ein aggressiver Zickzackkurs. Für gute Kombinationen.";data_.biome="ember";data_.accent=sf::Color(255,130,70);
        data_.path={{0,240},{230,240},{230,600},{430,600},{430,400},{690,400},{690,190},{900,190},{900,690},{1200,690}};
    }
    data_.spawn=data_.path.front();data_.goal=data_.path.back();data_.authoredBackground=true;
    rebuildLengths();
}

float GameMap::distanceToSegment(sf::Vector2f p,sf::Vector2f a,sf::Vector2f b) const{
    sf::Vector2f ab=b-a; float den=ab.x*ab.x+ab.y*ab.y; if(den<.001f) return distance(p,a);
    float t=((p-a).x*ab.x+(p-a).y*ab.y)/den; t=clampf(t,0.f,1.f); return distance(p,a+ab*t);
}

bool GameMap::canBuild(sf::Vector2f p,const std::vector<sf::Vector2f>& towers,float radius) const{
    if(p.x<radius+8 || p.y<radius+8 || p.x>WORLD_W-radius-8 || p.y>WORLD_H-radius-8) return false;
    for(std::size_t i=0;i+1<data_.path.size();++i) if(distanceToSegment(p,data_.path[i],data_.path[i+1]) < 58.f+radius*.42f) return false;
    for(auto t:towers) if(distance(p,t)<radius*1.75f) return false;
    if(!data_.path.empty() && (distance(p,data_.path.front())<105.f || distance(p,data_.path.back())<105.f)) return false;
    const bool hasBuildZone=std::any_of(data_.zones.begin(),data_.zones.end(),[](const GameMapZone& zone){return zone.type==aegis::core::ZoneType::Buildable;});
    if(!data_.authoredBackground && hasBuildZone){
        const bool insideBuildZone=std::any_of(data_.zones.begin(),data_.zones.end(),[&](const GameMapZone& zone){return zone.type==aegis::core::ZoneType::Buildable && zone.rect.contains(p);});
        if(!insideBuildZone)return false;
    }
    for(const auto& zone:data_.zones){
            if(zone.type!=aegis::core::ZoneType::Blocked && zone.type!=aegis::core::ZoneType::Water) continue;
            if(zone.rect.contains(p)) return false;
    }
    return true;
}

float GameMap::pathProgress(std::size_t segment,float segmentT) const{
    float d=0.f; for(std::size_t i=0;i<segment && i<segmentLengths_.size();++i)d+=segmentLengths_[i];
    if(segment<segmentLengths_.size()) d+=segmentLengths_[segment]*segmentT;
    return clampf(d/totalLength_,0.f,1.f);
}
