#pragma once

#include <mediaforge/foundation/Error.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace mf {

struct ImageData {
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::byte> rgbaPixels;
};

[[nodiscard]] Result<ImageData> loadImage(const std::filesystem::path& path);

} // namespace mf
