#pragma once

#include <cstdint>
#include <functional>
#include <limits>

namespace mf {

template <typename Tag, typename Value = std::uint32_t>
class Handle {
public:
    using value_type = Value;
    static constexpr Value invalidValue = std::numeric_limits<Value>::max();

    constexpr Handle() noexcept = default;
    explicit constexpr Handle(Value value) noexcept : value_(value) {}

    [[nodiscard]] constexpr Value value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool valid() const noexcept { return value_ != invalidValue; }
    explicit constexpr operator bool() const noexcept { return valid(); }

    friend constexpr bool operator==(Handle, Handle) noexcept = default;

private:
    Value value_{invalidValue};
};

template <typename Tag, typename Value>
struct HandleHash {
    std::size_t operator()(Handle<Tag, Value> handle) const noexcept {
        return std::hash<Value>{}(handle.value());
    }
};

struct BufferTag;
struct TextureTag;
struct SamplerTag;
struct ShaderTag;
struct PipelineTag;

using BufferHandle = Handle<BufferTag>;
using TextureHandle = Handle<TextureTag>;
using SamplerHandle = Handle<SamplerTag>;
using ShaderHandle = Handle<ShaderTag>;
using PipelineHandle = Handle<PipelineTag>;

} // namespace mf
