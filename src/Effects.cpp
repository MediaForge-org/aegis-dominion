#include "Effects.hpp"
#include "ui/TextService.hpp"
#include <random>
#include <cmath>

static std::mt19937& rng(){ static std::mt19937 r{1337}; return r; }
void Effects::burst(sf::Vector2f pos,sf::Color c,int count,float speed){
    std::uniform_real_distribution<float> a(0.f,2.f*PI_F), s(.35f,1.f), l(.25f,.65f), z(2.f,6.f);
    for(int i=0;i<count;++i){ float ang=a(rng()), sp=speed*s(rng()); particles_.push_back({pos,{std::cos(ang)*sp,std::sin(ang)*sp},c,l(rng()),.65f,z(rng())}); }
}
void Effects::ring(sf::Vector2f p,sf::Color c,float r,float life){ rings_.push_back({p,c,8.f,r,life,life}); }
void Effects::tracer(sf::Vector2f a,sf::Vector2f b,sf::Color c,float life,float w){ tracers_.push_back({a,b,c,life,life,w}); }
void Effects::text(sf::Vector2f p,const std::string& s,sf::Color c){ texts_.push_back({p,s,c,.9f,.9f}); }
void Effects::clear(){ particles_.clear(); rings_.clear(); tracers_.clear(); texts_.clear(); }
void Effects::update(float dt){
    for(auto& p:particles_){ p.life-=dt; p.pos+=p.vel*dt; p.vel*=std::pow(.08f,dt); }
    particles_.erase(std::remove_if(particles_.begin(),particles_.end(),[](auto& p){return p.life<=0;}),particles_.end());
    for(auto& r:rings_){ r.life-=dt; float t=1.f-r.life/r.maxLife; r.radius=8.f+(r.maxRadius-8.f)*t; }
    rings_.erase(std::remove_if(rings_.begin(),rings_.end(),[](auto& r){return r.life<=0;}),rings_.end());
    for(auto& t:tracers_) t.life-=dt;
    tracers_.erase(std::remove_if(tracers_.begin(),tracers_.end(),[](auto& t){return t.life<=0;}),tracers_.end());
    for(auto& t:texts_){ t.life-=dt; t.pos.y-=22.f*dt; }
    texts_.erase(std::remove_if(texts_.begin(),texts_.end(),[](auto& t){return t.life<=0;}),texts_.end());
}
void Effects::draw(sf::RenderTarget& rt,const aegis::ui::TextService* text) const{
    for(const auto& t:tracers_){
        sf::Vector2f d=t.b-t.a; float len=length(d); sf::RectangleShape r({len,t.width}); r.setOrigin(0,t.width/2); r.setPosition(t.a); r.setRotation(std::atan2(d.y,d.x)*180/PI_F); auto c=t.color; c.a=static_cast<sf::Uint8>(255*clampf(t.life/t.maxLife,0,1)); r.setFillColor(c); rt.draw(r);
    }
    for(const auto& r:rings_){ sf::CircleShape s(r.radius); s.setOrigin(r.radius,r.radius); s.setPosition(r.pos); s.setFillColor(sf::Color::Transparent); auto c=r.color; c.a=static_cast<sf::Uint8>(190*clampf(r.life/r.maxLife,0,1)); s.setOutlineColor(c); s.setOutlineThickness(3.f); rt.draw(s); }
    for(const auto& p:particles_){ sf::CircleShape s(p.size); s.setOrigin(p.size,p.size); s.setPosition(p.pos); auto c=p.color; c.a=static_cast<sf::Uint8>(255*clampf(p.life/p.maxLife,0,1)); s.setFillColor(c); rt.draw(s); }
    if(text && text->loaded()){ for(const auto& ft:texts_){ sf::Text t=text->makeText(ft.text,17); t.setStyle(sf::Text::Bold); t.setPosition(ft.pos); auto c=ft.color; c.a=static_cast<sf::Uint8>(255*clampf(ft.life/ft.maxLife,0,1)); t.setFillColor(c); t.setOutlineColor(sf::Color(0,0,0,c.a)); t.setOutlineThickness(2.f); rt.draw(t); } }
}
