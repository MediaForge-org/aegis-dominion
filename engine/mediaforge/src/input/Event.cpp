#include <mediaforge/input/Event.hpp>

#include <SDL3/SDL.h>

namespace mf {

std::vector<Event> EventPump::poll() {
    std::vector<Event> events;
    SDL_Event native{};
    while (SDL_PollEvent(&native)) {
        switch (native.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            events.emplace_back(QuitEvent{});
            break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            events.emplace_back(WindowResizeEvent{native.window.data1, native.window.data2});
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            events.emplace_back(KeyboardEvent{static_cast<Key>(native.key.scancode),
                                               native.type == SDL_EVENT_KEY_DOWN, native.key.repeat});
            break;
        case SDL_EVENT_MOUSE_MOTION:
            events.emplace_back(MouseMoveEvent{{native.motion.x, native.motion.y},
                                                {native.motion.xrel, native.motion.yrel}});
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            events.emplace_back(MouseButtonEvent{static_cast<MouseButton>(native.button.button),
                                                  native.type == SDL_EVENT_MOUSE_BUTTON_DOWN,
                                                  {native.button.x, native.button.y}});
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            events.emplace_back(MouseWheelEvent{{native.wheel.x, native.wheel.y}});
            break;
        default:
            break;
        }
    }
    return events;
}

} // namespace mf
