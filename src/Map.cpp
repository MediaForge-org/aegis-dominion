#include "Map.hpp"
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

void GameMap::applyDocument(const aegis::core::MapDocument& doc, int visualIndex){
    index_=std::max(0,std::min(2,visualIndex));
    document_=doc;
    data_.id=doc.metadata.id;
    data_.name=doc.metadata.name;
    data_.subtitle=doc.metadata.subtitle;
    data_.description=doc.metadata.description;
    data_.biome=doc.metadata.biome;
    data_.accent=accentForBiome(doc.metadata.biome);
    data_.path.clear();
    if(!doc.paths.empty()){
        data_.path.reserve(doc.paths.front().nodes.size());
        for(const auto& p:doc.paths.front().nodes) data_.path.emplace_back(p.x,p.y);
    }
    rebuildLengths();
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
    document_.reset();
    index_=std::max(0,std::min(2,index));
    if(index_==0){
        data_={"verdant_frontier","Grüne Grenze","Ausgewogen","Weite Bauflächen und ein gut lesbarer Pfad. Ideal zum Lernen.","verdant",sf::Color(92,196,139),
            {{0,145},{180,145},{180,330},{500,330},{500,160},{780,160},{780,560},{1060,560},{1060,760},{1200,760}}};
    } else if(index_==1){
        data_={"frost_pass","Frostpass","Taktisch","Lange Geraden wechseln mit engen Kurven. Reichweite ist hier besonders wertvoll.","frost",sf::Color(110,221,255),
            {{0,690},{210,690},{210,480},{420,480},{420,720},{690,720},{690,400},{920,400},{920,190},{1200,190}}};
    } else {
        data_={"ember_field","Aschefeld","Schwer","Wenig sichere Fläche und ein aggressiver Zickzackkurs. Für gute Kombinationen.","ember",sf::Color(255,130,70),
            {{0,240},{230,240},{230,600},{430,600},{430,400},{690,400},{690,190},{900,190},{900,690},{1200,690}}};
    }
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
    return true;
}

float GameMap::pathProgress(std::size_t segment,float segmentT) const{
    float d=0.f; for(std::size_t i=0;i<segment && i<segmentLengths_.size();++i)d+=segmentLengths_[i];
    if(segment<segmentLengths_.size()) d+=segmentLengths_[segment]*segmentT;
    return clampf(d/totalLength_,0.f,1.f);
}
