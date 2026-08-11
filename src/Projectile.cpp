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
    if(dist<=step+std::max(8.f,t->radius()*.35f)){
        sf::Vector2f impact=t->position();
        if(splash>0){
            for(auto& e:enemies){ if(e->dead()||e->reachedEnd())continue; float dd=distance(e->position(),impact); if(dd<=splash){ float fall=.55f+.45f*(1.f-dd/splash); e->takeDamage(damage*fall); if(slowFactor<1.f)e->applySlow(slowFactor,slowDuration); } }
            fx.ring(impact,color,splash,.35f); fx.burst(impact,color,18,150.f);
        } else {
            t->takeDamage(damage); if(slowFactor<1.f)t->applySlow(slowFactor,slowDuration); fx.burst(impact,color,7,85.f);
        }
        alive=false; return;
    }
    pos += normalize(d)*step;
}

void Projectile::draw(sf::RenderTarget& rt) const{
    if(!alive) return;
    float r=kind==ProjectileKind::Missile?6.f:(kind==ProjectileKind::Shell?5.f:3.5f);
    sf::CircleShape glow(r*2.1f); glow.setOrigin(r*2.1f,r*2.1f); glow.setPosition(pos); glow.setFillColor(withAlpha(color,45)); rt.draw(glow);
    sf::CircleShape s(r); s.setOrigin(r,r); s.setPosition(pos); s.setFillColor(color); rt.draw(s);
}
