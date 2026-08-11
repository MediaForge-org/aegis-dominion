#include "Enemy.hpp"

Enemy::Enemy(int id,EnemyKind kind,float hp,float speed,int reward,float armor)
:id_(id),kind_(kind),hp_(hp),maxHp_(hp),baseSpeed_(speed),armor_(armor),reward_(reward){}

void Enemy::update(float dt,const GameMap& map){
    if(dead()||reachedEnd_) return;
    if(regenPerSec_>0) heal(regenPerSec_*dt);
    if(slowTimer_>0){ slowTimer_-=dt; if(slowTimer_<=0) slowFactor_=1.f; }
    const auto& p=map.path();
    if(p.size()<2){ reachedEnd_=true; return; }
    if(segment_>=p.size()-1){ reachedEnd_=true; return; }
    float remaining=baseSpeed_*slowFactor_*dt;
    while(remaining>0.f && !reachedEnd_){
        sf::Vector2f a=p[segment_], b=p[segment_+1]; float segLen=distance(a,b);
        if(segLen<.001f){ segment_++; segmentT_=0; continue; }
        float left=(1.f-segmentT_)*segLen;
        if(remaining>=left){ remaining-=left; segment_++; segmentT_=0.f; if(segment_>=p.size()-1){ reachedEnd_=true; pos_=p.back(); break; } }
        else { segmentT_+=remaining/segLen; remaining=0.f; }
    }
    if(!reachedEnd_){ sf::Vector2f a=p[segment_],b=p[segment_+1]; pos_=a+(b-a)*segmentT_; angle_=angleDeg(b-a); }
    progress_=map.pathProgress(std::min(segment_,p.size()-2),segmentT_);
}

void Enemy::setPathState(std::size_t seg,float t,const GameMap& map){
    const auto& p=map.path(); if(p.size()<2)return; segment_=std::min(seg,p.size()-2); segmentT_=clampf(t,0.f,1.f);
    sf::Vector2f a=p[segment_],b=p[segment_+1]; pos_=a+(b-a)*segmentT_; angle_=angleDeg(b-a); progress_=map.pathProgress(segment_,segmentT_); reachedEnd_=false;
}

float Enemy::takeDamage(float dmg){
    if(dmg<=0) return 0;
    float actual=dmg*(1.f-armor_);
    if(shield_>0){ float s=std::min(shield_,actual); shield_-=s; actual-=s; }
    float before=hp_; hp_=std::max(0.f,hp_-actual); return before-hp_;
}
void Enemy::applySlow(float factor,float duration){
    factor=1.f-(1.f-factor)*(1.f-slowResistance_);
    if(factor<slowFactor_ || slowTimer_<=0) slowFactor_=clampf(factor,.2f,1.f);
    slowTimer_=std::max(slowTimer_,duration*(1.f-slowResistance_*.4f));
}

void Enemy::draw(sf::RenderTarget& rt,const Assets& assets) const{
    sf::Sprite s(assets.enemy(kind_));
    auto b=s.getLocalBounds(); s.setOrigin(b.width/2,b.height/2); s.setPosition(pos_); s.setRotation(angle_); float sc=(kind_==EnemyKind::Boss?1.2f:.82f); s.setScale(sc,sc); rt.draw(s);
    float w=(kind_==EnemyKind::Boss?78.f:56.f), h=7.f; sf::RectangleShape bg({w,h}); bg.setOrigin(w/2,h/2); bg.setPosition(pos_.x,pos_.y-radius_-13); bg.setFillColor(sf::Color(5,8,11,210)); rt.draw(bg);
    sf::RectangleShape hp({w*clampf(hp_/maxHp_,0,1),h-2}); hp.setOrigin(w/2, (h-2)/2); hp.setPosition(pos_.x,pos_.y-radius_-13); hp.setFillColor(sf::Color(92,225,129)); rt.draw(hp);
    if(maxShield_>0 && shield_>0){ sf::RectangleShape sh({w*clampf(shield_/maxShield_,0,1),3}); sh.setOrigin(w/2,1.5f); sh.setPosition(pos_.x,pos_.y-radius_-7); sh.setFillColor(sf::Color(92,189,255)); rt.draw(sh); }
}

RaiderEnemy::RaiderEnemy(int id,float s):Enemy(id,EnemyKind::Raider,80*s,76.f,12,0.02f){}
RunnerEnemy::RunnerEnemy(int id,float s):Enemy(id,EnemyKind::Runner,48*s,125.f,10,0.f){ radius_=24; }
TankEnemy::TankEnemy(int id,float s):Enemy(id,EnemyKind::Tank,310*s,48.f,28,0.26f){ radius_=32; }
ShieldEnemy::ShieldEnemy(int id,float s):Enemy(id,EnemyKind::Shield,125*s,68.f,20,0.08f){ setShield(105*s); }
RegenEnemy::RegenEnemy(int id,float s):Enemy(id,EnemyKind::Regen,150*s,62.f,22,0.06f){ setRegen(8.f*s); }
SplitterEnemy::SplitterEnemy(int id,float s):Enemy(id,EnemyKind::Splitter,110*s,84.f,18,0.04f){}
BossEnemy::BossEnemy(int id,float s):Enemy(id,EnemyKind::Boss,2200*s,39.f,180,0.18f){ radius_=40; setShield(520*s); setRegen(7.f*s); setSlowResistance(.72f); }

std::unique_ptr<Enemy> makeEnemy(EnemyKind k,int id,float scale){
    switch(k){
        case EnemyKind::Raider:return std::make_unique<RaiderEnemy>(id,scale);
        case EnemyKind::Runner:return std::make_unique<RunnerEnemy>(id,scale);
        case EnemyKind::Tank:return std::make_unique<TankEnemy>(id,scale);
        case EnemyKind::Shield:return std::make_unique<ShieldEnemy>(id,scale);
        case EnemyKind::Regen:return std::make_unique<RegenEnemy>(id,scale);
        case EnemyKind::Splitter:return std::make_unique<SplitterEnemy>(id,scale);
        case EnemyKind::Boss:return std::make_unique<BossEnemy>(id,scale);
    }
    return std::make_unique<RaiderEnemy>(id,scale);
}
