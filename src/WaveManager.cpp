#include "WaveManager.hpp"
#include <algorithm>

void WaveManager::reset(){ wave_=0;active_=false;pendingSpawn_=false;spawnTimer_=0;spawnIndex_=0;current_.clear();preview_=buildWave(1); }
std::vector<EnemyKind> WaveManager::buildWave(int w) const{
    std::vector<EnemyKind> out; int base=7+w*2;
    for(int i=0;i<base;++i){
        EnemyKind k=EnemyKind::Raider;
        int m=(i+w*3)%12;
        if(w>=2 && m==1) k=EnemyKind::Runner;
        if(w>=4 && (m==4||m==9)) k=EnemyKind::Tank;
        if(w>=6 && m==6) k=EnemyKind::Shield;
        if(w>=8 && m==8) k=EnemyKind::Regen;
        if(w>=10 && m==10) k=EnemyKind::Splitter;
        out.push_back(k);
    }
    if(w%5==0) out.push_back(EnemyKind::Boss);
    return out;
}
bool WaveManager::startNext(){ if(active_||wave_>=maxWaves())return false; wave_++;current_=buildWave(wave_);preview_=(wave_<maxWaves()?buildWave(wave_+1):std::vector<EnemyKind>{});active_=true;spawnIndex_=0;spawnTimer_=.15f;spawnInterval_=std::max(.28f,.74f-wave_*.018f);pendingSpawn_=false;return true; }
void WaveManager::update(float dt){ if(!active_||pendingSpawn_||spawnIndex_>=current_.size())return; spawnTimer_-=dt; if(spawnTimer_<=0){ pendingSpawn_=true; spawnTimer_=spawnInterval_; } }
EnemyKind WaveManager::consumeSpawn(){ pendingSpawn_=false; if(spawnIndex_>=current_.size())return EnemyKind::Raider; return current_[spawnIndex_++]; }
float WaveManager::enemyScale() const { return 1.f + std::max(0,wave_-1)*.115f; }
std::string WaveManager::threatText() const{ if(wave_<5)return"Niedrig"; if(wave_<10)return"Erhöht"; if(wave_<15)return"Hoch"; return"KRITISCH"; }
