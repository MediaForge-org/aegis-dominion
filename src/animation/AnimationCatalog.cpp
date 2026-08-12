#include "AnimationCatalog.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace aegis::animation {

bool AnimationCatalog::load(const std::filesystem::path& file,std::string* error){
    std::ifstream input(file,std::ios::binary);
    if(!input){if(error)*error="Animation definition file not found: "+file.string();return false;}
    std::string header;std::getline(input,header);
    if(header!="AEGIS_ANIMATIONS_V1"){if(error)*error="Unsupported animation header: "+header;return false;}
    std::unordered_map<std::string,ExternalAnimationDefinition> parsed;
    std::string line;int lineNumber=1;
    while(std::getline(input,line)){
        ++lineNumber;if(line.empty()||line[0]=='#')continue;
        std::istringstream row(line);std::string keyword,id,sheet,playback;int frameCount=0,x=0,y=0,w=0,h=0;float duration=0.f;
        row>>keyword>>std::quoted(id)>>std::quoted(sheet)>>playback>>frameCount>>x>>y>>w>>h>>duration;
        if(!row||keyword!="clip"||id.empty()||sheet.empty()||frameCount<=0||w<=0||h<=0||duration<=0.f||parsed.contains(id)){
            if(error)*error="Invalid animation definition in line "+std::to_string(lineNumber);
            return false;
        }
        ExternalAnimationDefinition definition;definition.animation.id=id;definition.spriteSheetId=sheet;
        definition.animation.playback=playback=="once"?Playback::OneShot:Playback::Loop;
        definition.animation.frames.reserve(static_cast<std::size_t>(frameCount));
        for(int frame=0;frame<frameCount;++frame)definition.animation.frames.push_back({{x+frame*w,y,w,h},duration});
        parsed.emplace(id,std::move(definition));
    }
    definitions_=std::move(parsed);return true;
}

const ExternalAnimationDefinition* AnimationCatalog::find(std::string_view id)const{
    const auto found=definitions_.find(std::string(id));return found==definitions_.end()?nullptr:&found->second;
}

} // namespace aegis::animation
