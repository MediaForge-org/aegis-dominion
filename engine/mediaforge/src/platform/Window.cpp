#include <mediaforge/platform/Window.hpp>

#include <mediaforge/foundation/Log.hpp>

#include <SDL3/SDL.h>

#include <format>
#include <utility>

namespace mf {

struct Window::Impl {
    SDL_Window* window{};
    bool closeRequested{};
    bool ownsSdlVideo{};

    ~Impl() {
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
        if (ownsSdlVideo) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
        }
    }
};

Window::Window() noexcept = default;
Window::~Window() = default;
Window::Window(Window&&) noexcept = default;
Window& Window::operator=(Window&&) noexcept = default;
Window::Window(std::unique_ptr<Impl> impl) noexcept : impl_(std::move(impl)) {}

Result<Window> Window::create(const WindowConfig& config) {
    if (config.width <= 0 || config.height <= 0) {
        return fail(ErrorCode::invalidArgument, "Window dimensions must be positive");
    }
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return fail(ErrorCode::platformError, std::format("SDL initialization failed: {}", SDL_GetError()));
    }

    SDL_WindowFlags flags = 0;
    if (config.resizable) { flags |= SDL_WINDOW_RESIZABLE; }
    if (config.highPixelDensity) { flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY; }
    if (config.fullscreen == FullscreenMode::desktop) { flags |= SDL_WINDOW_FULLSCREEN; }

    SDL_Window* native = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (native == nullptr) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
        return fail(ErrorCode::platformError, std::format("Window creation failed: {}", SDL_GetError()));
    }

    Window result(std::make_unique<Impl>());
    result.impl_->window = native;
    result.impl_->ownsSdlVideo = true;
    const auto drawable = result.drawableSize();
    log(LogLevel::info, std::format("MediaForge drawable size: {}x{}", drawable.width, drawable.height));
    return result;
}

Status Window::resize(Extent2D requested) {
    if (!impl_ || requested.width <= 0 || requested.height <= 0) {
        return fail(ErrorCode::invalidArgument, "Invalid window or dimensions");
    }
    if (!SDL_SetWindowSize(impl_->window, requested.width, requested.height)) {
        return fail(ErrorCode::platformError, SDL_GetError());
    }
    return {};
}

Status Window::setTitle(const std::string& title) {
    if (!impl_ || !SDL_SetWindowTitle(impl_->window, title.c_str())) {
        return fail(ErrorCode::platformError, SDL_GetError());
    }
    return {};
}

Status Window::setFullscreen(FullscreenMode mode) {
    if (!impl_ || !SDL_SetWindowFullscreen(impl_->window, mode == FullscreenMode::desktop)) {
        return fail(ErrorCode::platformError, SDL_GetError());
    }
    return {};
}

Extent2D Window::size() const noexcept {
    Extent2D result{};
    if (impl_) { SDL_GetWindowSize(impl_->window, &result.width, &result.height); }
    return result;
}

Extent2D Window::drawableSize() const noexcept {
    Extent2D result{};
    if (impl_) { SDL_GetWindowSizeInPixels(impl_->window, &result.width, &result.height); }
    return result;
}

bool Window::closeRequested() const noexcept { return !impl_ || impl_->closeRequested; }
void Window::requestClose() noexcept { if (impl_) { impl_->closeRequested = true; } }
void* Window::nativeHandle() const noexcept { return impl_ ? impl_->window : nullptr; }

} // namespace mf
