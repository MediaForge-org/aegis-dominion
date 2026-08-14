#include <mediaforge/foundation/Config.hpp>
#include <mediaforge/foundation/Error.hpp>
#include <mediaforge/foundation/Handle.hpp>
#include <mediaforge/foundation/ScopeExit.hpp>
#include <mediaforge/foundation/Time.hpp>
#include <mediaforge/foundation/Version.hpp>
#include <mediaforge/input/InputState.hpp>
#include <mediaforge/math/Math.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <string_view>

namespace {
int checks{};
int failures{};

void check(bool condition, std::string_view name) {
    ++checks;
    if (!condition) { ++failures; std::cerr << "FAILED: " << name << '\n'; }
}

bool near(float left, float right) { return std::abs(left - right) < 0.0001F; }
}

int main() {
    check(mf::versionString == "0.2.0-dev", "independent engine version");
    mf::TextureHandle invalid;
    mf::TextureHandle first(7);
    check(!invalid.valid(), "default handle invalid");
    check(first.valid() && first.value() == 7, "typed handle value");
    check(first == mf::TextureHandle(7), "typed handle equality");

    mf::Result<int> success = 42;
    mf::Result<int> failure = mf::fail(mf::ErrorCode::ioError, "missing");
    check(success && *success == 42, "expected success path");
    check(!failure && failure.error().code == mf::ErrorCode::ioError, "expected error path");

    bool exited = false;
    { mf::ScopeExit guard([&exited] { exited = true; }); }
    check(exited, "scope-safe cleanup");
    check(near(mf::seconds(std::chrono::milliseconds(250)), 0.25F), "duration conversion");
    check(mf::preferredBackendName(mf::GpuBackendPreference::automatic) == nullptr, "automatic backend");
    check(std::string_view(mf::preferredBackendName(mf::GpuBackendPreference::preferVulkan)) == "vulkan", "Vulkan preference");

    check(mf::Vec2{1, 2} + mf::Vec2{3, 4} == mf::Vec2{4, 6}, "Vec2 addition");
    check(mf::Vec3{4, 5, 6} - mf::Vec3{1, 2, 3} == mf::Vec3{3, 3, 3}, "Vec3 subtraction");
    check(near(mf::length(mf::Vec2{3, 4}), 5.0F), "Vec2 length");
    check(near(mf::dot(mf::Vec3{1, 2, 3}, mf::Vec3{4, 5, 6}), 32.0F), "Vec3 dot");
    check(near(mf::radians(180.0F), std::numbers::pi_v<float>), "degrees to radians");
    check(near(mf::degrees(std::numbers::pi_v<float>), 180.0F), "radians to degrees");
    check(mf::clamp(12, 0, 10) == 10, "clamp");
    check(near(mf::lerp(2.0F, 6.0F, 0.25F), 3.0F), "lerp");
    check(mf::Mat4::identity() * mf::Mat4::identity() == mf::Mat4::identity(), "Mat4 multiplication");
    const auto transform = mf::Transform3D{{2, 3, 4}, {}, {1, 1, 1}}.matrix();
    check(near(transform.at(0, 3), 2) && near(transform.at(2, 3), 4), "Transform3D matrix");

    mf::InputState input;
    input.beginFrame();
    input.consume(mf::KeyboardEvent{mf::Key::space, true, false});
    check(input.keyDown(mf::Key::space) && input.keyPressed(mf::Key::space), "key pressed state");
    input.beginFrame();
    check(input.keyDown(mf::Key::space) && !input.keyPressed(mf::Key::space), "held key across frame");
    input.consume(mf::KeyboardEvent{mf::Key::space, false, false});
    check(!input.keyDown(mf::Key::space) && input.keyReleased(mf::Key::space), "key released state");
    input.consume(mf::MouseMoveEvent{{10, 20}, {3, -2}});
    input.consume(mf::MouseWheelEvent{{0, 1}});
    check(input.mousePosition() == mf::Vec2{10, 20} && input.mouseDelta() == mf::Vec2{3, -2}, "mouse motion state");
    check(input.wheelDelta() == mf::Vec2{0, 1}, "wheel state");

    std::cout << "MediaForge foundation: " << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
