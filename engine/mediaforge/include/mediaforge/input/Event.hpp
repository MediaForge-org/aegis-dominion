#pragma once

#include <mediaforge/math/Math.hpp>

#include <cstdint>
#include <variant>
#include <vector>

namespace mf {

enum class Key : std::uint16_t {
    unknown = 0,
    a = 4, d = 7, s = 22, w = 26,
    escape = 41, space = 44,
    left = 80, right = 79, down = 81, up = 82,
};

enum class MouseButton : std::uint8_t { left = 1, middle = 2, right = 3, extra1 = 4, extra2 = 5 };

struct QuitEvent {};
struct WindowResizeEvent { int width{}; int height{}; };
struct KeyboardEvent { Key key{Key::unknown}; bool pressed{}; bool repeat{}; };
struct MouseMoveEvent { Vec2 position{}; Vec2 delta{}; };
struct MouseButtonEvent { MouseButton button{MouseButton::left}; bool pressed{}; Vec2 position{}; };
struct MouseWheelEvent { Vec2 delta{}; };

using Event = std::variant<QuitEvent, WindowResizeEvent, KeyboardEvent, MouseMoveEvent,
                           MouseButtonEvent, MouseWheelEvent>;

class EventPump {
public:
    [[nodiscard]] std::vector<Event> poll();
};

} // namespace mf
