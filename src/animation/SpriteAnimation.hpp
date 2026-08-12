#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <vector>

namespace aegis::animation {

enum class Playback { Loop, OneShot };
struct AnimationFrame { sf::IntRect textureRect; float duration = .1f; };
struct AnimationDefinition { std::string id; std::vector<AnimationFrame> frames; Playback playback = Playback::Loop; };

class SpriteAnimator {
public:
    void play(const AnimationDefinition& definition, bool restart = false);
    void update(float deltaSeconds);
    const AnimationFrame* frame() const;
    std::size_t frameIndex() const { return frameIndex_; }
    bool finished() const { return finished_; }

private:
    const AnimationDefinition* definition_ = nullptr;
    std::size_t frameIndex_ = 0;
    float elapsed_ = 0.f;
    bool finished_ = false;
};

} // namespace aegis::animation
