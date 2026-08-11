#pragma once
#include "Common.hpp"
#include <vector>
#include <string>

class WaveManager {
public:
    void reset();
    bool startNext();
    void update(float dt);
    bool hasSpawn() const { return pendingSpawn_; }
    EnemyKind consumeSpawn();
    bool active() const { return active_; }
    bool spawningDone() const { return active_ && spawnIndex_>=current_.size(); }
    void finishWave(){ active_=false; }
    int wave() const { return wave_; }
    int maxWaves() const { return 20; }
    int completionBonus() const { return 45 + wave_*8; }
    float enemyScale() const;
    const std::vector<EnemyKind>& preview() const { return preview_; }
    std::string threatText() const;
private:
    std::vector<EnemyKind> buildWave(int w) const;
    int wave_=0;
    bool active_=false, pendingSpawn_=false;
    float spawnTimer_=0.f, spawnInterval_=.75f;
    std::size_t spawnIndex_=0;
    std::vector<EnemyKind> current_, preview_;
};
