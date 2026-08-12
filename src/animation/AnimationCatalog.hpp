#pragma once

#include "SpriteAnimation.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace aegis::animation {

struct ExternalAnimationDefinition {
    AnimationDefinition animation;
    std::string spriteSheetId;
};

class AnimationCatalog {
public:
    bool load(const std::filesystem::path& file, std::string* error = nullptr);
    const ExternalAnimationDefinition* find(std::string_view id) const;
    std::size_t size() const { return definitions_.size(); }

private:
    std::unordered_map<std::string, ExternalAnimationDefinition> definitions_;
};

} // namespace aegis::animation
