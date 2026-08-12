#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Effects.hpp"
#include <cmath>

static Enemy* findEnemy(std::vector<std::unique_ptr<Enemy>>& es,int id){ for(auto& e:es) if(e->id()==id && !e->dead() && !e->reachedEnd()) return e.get(); return nullptr; }

void Projectile::update(float dt,std::vector<std::unique_ptr<Enemy>>& enemies,Effects& fx){
    if(!alive) return;
    life-=dt;
    if(life<=0){ alive=false; return; }
    Enemy* t=findEnemy(enemies,targetId); if(!t){ alive=false; return; }
    sf::Vector2f d=t->position()-pos; float dist=length(d); float step=speed*dt;
    rotationDeg=angleDeg(d);
    if(kind==ProjectileKind::Missile)fx.trail(pos,-normalize(d)*35.f,sf::Color(125,132,136,150),7.f,.48f,-8.f);
    else if(kind==ProjectileKind::Frost)fx.trail(pos,-normalize(d)*18.f,sf::Color(155,241,255,145),3.5f,.28f);
    if(dist<=step+std::max(8.f,t->radius()*.35f)){
        sf::Vector2f impact=t->position();
        if(splash>0){
            for(auto& e:enemies){ if(e->dead()||e->reachedEnd())continue; float dd=distance(e->position(),impact); if(dd<=splash){ float fall=.55f+.45f*(1.f-dd/splash); e->takeDamage(damage*fall); if(slowFactor<1.f)e->applySlow(slowFactor,slowDuration); } }
            fx.ring(impact,color,splash,.35f); fx.burst(impact,color,18,150.f);fx.shake(kind==ProjectileKind::Missile?5.f:2.5f);
        } else {
            t->takeDamage(damage); if(slowFactor<1.f)t->applySlow(slowFactor,slowDuration); fx.burst(impact,color,7,85.f);
        }
        alive=false; return;
    }
    pos += normalize(d)*step;
}

aegis::render::ProjectileRenderSnapshot Projectile::renderSnapshot() const {
    aegis::render::ProjectileRenderSnapshot snapshot;
    snapshot.position={pos.x,pos.y}; snapshot.rotationDeg=rotationDeg;
    snapshot.visualId=kind==ProjectileKind::Missile?"projectile.missile":kind==ProjectileKind::Shell?"projectile.shell":kind==ProjectileKind::Frost?"projectile.cryo":"projectile.energy";
    snapshot.effectProfile=kind==ProjectileKind::Missile?"missile":kind==ProjectileKind::Shell?"shell":kind==ProjectileKind::Frost?"cryo":"energy";
    snapshot.color={color.r,color.g,color.b,color.a}; return snapshot;
}
