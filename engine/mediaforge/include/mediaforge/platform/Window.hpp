#pragma once

#include <mediaforge/foundation/Error.hpp>

#include <memory>
#include <string>

namespace mf {

namespace detail { class NativeAccess; }

struct Extent2D {
    int width{};
    int height{};
    friend constexpr bool operator==(Extent2D, Extent2D) noexcept = default;
};

enum class FullscreenMode { windowed, desktop };

struct WindowConfig {
    std::string title{"MediaForge"};
    int width{1280};
    int height{720};
    bool resizable{true};
    bool highPixelDensity{true};
    FullscreenMode fullscreen{FullscreenMode::windowed};
};

class GPUDevice;

class Window {
public:
    Window() noexcept;
    ~Window();
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] static Result<Window> create(const WindowConfig& config);
    [[nodiscard]] Status resize(Extent2D size);
    [[nodiscard]] Status setTitle(const std::string& title);
    [[nodiscard]] Status setFullscreen(FullscreenMode mode);
    [[nodiscard]] Extent2D size() const noexcept;
    [[nodiscard]] Extent2D drawableSize() const noexcept;
    [[nodiscard]] bool closeRequested() const noexcept;
    void requestClose() noexcept;

private:
    struct Impl;
    explicit Window(std::unique_ptr<Impl> impl) noexcept;
    [[nodiscard]] void* nativeHandle() const noexcept;

    std::unique_ptr<Impl> impl_;
    friend class GPUDevice;
    friend class detail::NativeAccess;
};

} // namespace mf
