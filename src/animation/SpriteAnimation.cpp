#include "SpriteAnimation.hpp"

#include <algorithm>

namespace aegis::animation {

void SpriteAnimator::play(const AnimationDefinition& definition, bool restart) {
    if (definition_ == &definition && !restart) return;
    definition_ = &definition; frameIndex_ = 0; elapsed_ = 0.f; finished_ = definition.frames.empty();
}

void SpriteAnimator::update(float deltaSeconds) {
    if (!definition_ || definition_->frames.empty() || finished_) return;
    elapsed_ += std::max(0.f, deltaSeconds);
    while (elapsed_ >= definition_->frames[frameIndex_].duration && !finished_) {
        elapsed_ -= std::max(.001f, definition_->frames[frameIndex_].duration);
        if (frameIndex_ + 1 < definition_->frames.size()) ++frameIndex_;
        else if (definition_->playback == Playback::Loop) frameIndex_ = 0;
        else finished_ = true;
    }
}

const AnimationFrame* SpriteAnimator::frame() const {
    if (!definition_ || definition_->frames.empty()) return nullptr;
    return &definition_->frames[frameIndex_];
}

} // namespace aegis::animation
