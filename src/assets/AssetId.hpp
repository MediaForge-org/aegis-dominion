#pragma once

#include <compare>
#include <string>
#include <string_view>

namespace aegis::assets {

template <typename Tag>
class AssetId {
public:
    AssetId() = default;
    explicit AssetId(std::string value) : value_(std::move(value)) {}
    explicit AssetId(std::string_view value) : value_(value) {}
    explicit AssetId(const char* value) : value_(value) {}

    const std::string& value() const { return value_; }
    bool valid() const { return !value_.empty() && value_.find('/') == std::string::npos && value_.find('\\') == std::string::npos; }
    auto operator<=>(const AssetId&) const = default;

private:
    std::string value_;
};

struct TextureTag {};
struct FontTag {};
struct SoundTag {};
using TextureId = AssetId<TextureTag>;
using FontId = AssetId<FontTag>;
using SoundId = AssetId<SoundTag>;

enum class AssetType { Texture, Font, Sound };

struct AssetIdHash {
    template <typename Tag>
    std::size_t operator()(const AssetId<Tag>& id) const noexcept {
        return std::hash<std::string>{}(id.value());
    }
};

} // namespace aegis::assets
