#include <mediaforge/render/Image.hpp>

#include <SDL3/SDL.h>

#include <cstring>
#include <format>
#include <memory>

namespace mf {

Result<ImageData> loadImage(const std::filesystem::path& path) {
    using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
    SurfacePtr loaded(SDL_LoadPNG(path.string().c_str()), SDL_DestroySurface);
    if (!loaded) return fail(ErrorCode::ioError, std::format("Image load failed ({}): {}", path.string(), SDL_GetError()));
    SurfacePtr rgba(SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
    if (!rgba) return fail(ErrorCode::ioError, std::format("RGBA conversion failed ({}): {}", path.string(), SDL_GetError()));

    ImageData image;
    image.width = static_cast<std::uint32_t>(rgba->w);
    image.height = static_cast<std::uint32_t>(rgba->h);
    image.rgbaPixels.resize(static_cast<std::size_t>(rgba->w) * static_cast<std::size_t>(rgba->h) * 4U);
    const auto* source = static_cast<const std::byte*>(rgba->pixels);
    const std::size_t rowBytes = static_cast<std::size_t>(rgba->w) * 4U;
    for (int row = 0; row < rgba->h; ++row) {
        std::memcpy(image.rgbaPixels.data() + static_cast<std::size_t>(row) * rowBytes,
                    source + static_cast<std::size_t>(row) * static_cast<std::size_t>(rgba->pitch), rowBytes);
    }
    return image;
}

} // namespace mf
