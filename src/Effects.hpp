#pragma once
#include "Common.hpp"
#include <vector>
#include <string>

namespace aegis::ui { class TextService; }

struct Particle { sf::Vector2f pos, vel; sf::Color color; float life=1.f,maxLife=1.f,size=4.f; };
struct RingFx { sf::Vector2f pos; sf::Color color; float radius=8.f, maxRadius=80.f, life=.5f,maxLife=.5f; };
struct TracerFx { sf::Vector2f a,b; sf::Color color; float life=.12f,maxLife=.12f; float width=3.f; };
struct FloatTextFx { sf::Vector2f pos; std::string text; sf::Color color; float life=.9f,maxLife=.9f; };

class Effects {
public:
    void update(float dt);
    void draw(sf::RenderTarget& rt, const aegis::ui::TextService* text) const;
    void burst(sf::Vector2f pos, sf::Color c, int count=12, float speed=90.f);
    void ring(sf::Vector2f pos, sf::Color c, float maxRadius=70.f, float life=.45f);
    void tracer(sf::Vector2f a,sf::Vector2f b,sf::Color c,float life=.12f,float width=3.f);
    void text(sf::Vector2f pos,const std::string& s,sf::Color c);
    void clear();
private:
    std::vector<Particle> particles_;
    std::vector<RingFx> rings_;
    std::vector<TracerFx> tracers_;
    std::vector<FloatTextFx> texts_;
};
