#include <mediaforge/render/Camera2D.hpp>
#include <mediaforge/render/ParticleSystem2D.hpp>
#include <mediaforge/render/RenderQueue2D.hpp>
#include <mediaforge/render/Renderer2D.hpp>
#include <mediaforge/ui/SegmentedLevelIndicator.hpp>
#include <mediaforge/ui/Ui.hpp>

#include <cmath>
#include <iostream>
#include <string_view>
#include <vector>

namespace {
int checks{};
int failures{};

void check(bool condition, std::string_view name) {
    ++checks;
    if (!condition) { ++failures; std::cerr << "FAILED: " << name << '\n'; }
}

bool near(float left, float right) { return std::abs(left - right) < 0.001F; }
}

int main() {
    mf::Camera2D camera;
    camera.setCenter({800, 450});
    camera.setOrthographicSize({1600, 900});
    camera.setViewport({80, 40, 1280, 720});
    const auto screen = camera.worldToScreen({200, 700});
    const auto roundTrip = camera.screenToWorld(screen);
    check(near(roundTrip.x, 200) && near(roundTrip.y, 700), "camera world/screen round trip");
    camera.setZoom(2.0F);
    check(camera.visibleSize() == mf::Vec2{800, 450}, "camera zoom controls visible size");
    const auto wide = mf::Camera2D::letterbox({1600, 900}, {1280, 1024});
    check(near(wide.width, 1280) && near(wide.height, 720) && near(wide.y, 152), "letterbox viewport");
    check(mf::spriteBoundsVisible({50, 50}, {20, 20}, {0.5F, 0.5F}, 0, {0, 0, 100, 100}) &&
          !mf::spriteBoundsVisible({150, 50}, {20, 20}, {0.5F, 0.5F}, 0, {0, 0, 100, 100}) &&
          mf::spriteBoundsVisible({105, 50}, {20, 80}, {0.5F, 0.5F}, 0.8F, {0, 0, 100, 100}),
          "rotated sprite viewport culling is conservative");

    const std::vector<mf::SubmissionState2D> submissions{
        {30, 0, 1, 5, mf::BlendMode::alpha, 5}, {10, 0, 1, 7, mf::BlendMode::alpha, 0},
        {10, 0, 1, 7, mf::BlendMode::alpha, 1}, {10, 0, 1, 8, mf::BlendMode::additive, 2},
        {20, 2, 1, 5, mf::BlendMode::alpha, 4}, {20, 1, 1, 5, mf::BlendMode::alpha, 3},
    };
    const auto plan = mf::buildBatchPlan(submissions);
    check(plan.orderedIndices.front() == 1 && plan.orderedIndices.back() == 0, "layer ordering is stable");
    check(plan.batches.size() == 5 && plan.batches.front().count == 2, "compatible submissions batch");
    check(plan.orderedIndices[3] == 5 && plan.orderedIndices[4] == 4, "explicit order precedes state grouping");
    mf::BatchPlan2D reusablePlan;
    reusablePlan.orderedIndices.reserve(32);
    reusablePlan.batches.reserve(32);
    mf::buildBatchPlan(submissions, reusablePlan);
    const auto orderedCapacity = reusablePlan.orderedIndices.capacity();
    mf::buildBatchPlan(submissions, reusablePlan);
    check(reusablePlan.batches.size() == plan.batches.size() && reusablePlan.orderedIndices.capacity() == orderedCapacity,
          "batch plan reuses reserved frame storage");

    mf::ParticleSystem2D particles(2);
    mf::Particle2D particle;
    particle.velocity = {10, 0}; particle.acceleration = {0, 5}; particle.lifetime = 1; particle.drag = 0;
    particle.startSize = 10; particle.endSize = 20;
    check(particles.emit(particle) && particles.emit(particle) && !particles.emit(particle), "bounded particle pool");
    particles.update(0.5F);
    check(particles.particles().size() == 2 && near(particles.particles()[0].position.x, 5), "particle integration");
    check(near(mf::particleSize(particles.particles()[0]), 15), "particle interpolation");
    particles.update(0.5F);
    check(particles.particles().empty(), "expired particles removed");

    check(mf::validRenderTargetDescription({1280, 720, "valid"}), "valid render target");
    check(!mf::validRenderTargetDescription({0, 720, "invalid"}) &&
          !mf::validRenderTargetDescription({20'000, 720, "invalid"}), "invalid render target rejected");
    check(mf::approximateRenderTargetBytes({1600, 900, "hdr", mf::RenderTargetDescription::Format::rgba16Float}) ==
              1600ULL * 900ULL * 8ULL &&
          mf::approximateRenderTargetBytes({1600, 900, "ui", mf::RenderTargetDescription::Format::rgba8Unorm}) ==
              1600ULL * 900ULL * 4ULL, "render-target memory accounting distinguishes formats");

    mf::ui::Canvas canvas({1600, 900});
    canvas.setTargetSize({1280, 1024});
    const auto center = canvas.mapToCanvas({640, 512});
    check(center.inside && near(center.position.x, 800) && near(center.position.y, 450),
          "UI canvas maps resized letterboxed coordinates");
    check(!canvas.mapToCanvas({640, 80}).inside && canvas.viewport() == mf::ui::Rect{{0, 152}, {1280, 720}},
          "UI canvas rejects letterbox bars");
    check(mf::ui::Rect{{10, 20}, {100, 40}}.contains({10, 20}) &&
          mf::ui::Rect{{10, 20}, {100, 40}}.contains({110, 60}) &&
          !mf::ui::Rect{{10, 20}, {100, 40}}.contains({111, 60}), "UI rectangle hit testing includes edges");

    constexpr mf::ui::SegmentedLevelIndicator emptyIndicator{};
    constexpr mf::ui::SegmentedLevelIndicator firstOfFive{5, 0};
    constexpr mf::ui::SegmentedLevelIndicator fourthOfFive{5, 3};
    constexpr mf::ui::SegmentedLevelIndicator boundedIndicator{5, 99};
    static_assert(emptyIndicator.activeSegmentCount() == 0);
    static_assert(firstOfFive.activeSegmentCount() == 1);
    static_assert(fourthOfFive.activeSegmentCount() == 4);
    static_assert(boundedIndicator.activeSegmentCount() == 5);
    check(firstOfFive.segmentActive(0) && !firstOfFive.segmentActive(1) &&
          fourthOfFive.segmentActive(3) && !fourthOfFive.segmentActive(4),
          "segmented indicator derives active segments from current index");
    const auto fiveSegmentLayout = mf::ui::segmentedLevelIndicatorLayout(
        {{100, 200}, {232, 7}}, fourthOfFive);
    const auto twoSegmentLayout = mf::ui::segmentedLevelIndicatorLayout(
        {{100, 200}, {232, 7}}, {2, 1});
    check(fiveSegmentLayout.segmentCount == 5 && near(fiveSegmentLayout.segmentWidth, 40) &&
          near(fiveSegmentLayout.segmentBounds(4).position.x, 292),
          "segmented indicator lays out the actual option count");
    check(twoSegmentLayout.segmentCount == 2 && near(twoSegmentLayout.segmentWidth, 50) &&
          near(twoSegmentLayout.backgroundBounds.size.x, 116),
          "short segmented indicators stay compact and centered");
    check(fiveSegmentLayout.segmentBounds(5) == mf::ui::Rect{},
          "segmented indicator rejects geometry outside its segment count");
    const auto activeIdle = mf::ui::segmentedLevelIndicatorSegmentColor(
        fourthOfFive, 2, mf::ui::WidgetState::normal, 0.0F);
    const auto activePulse = mf::ui::segmentedLevelIndicatorSegmentColor(
        fourthOfFive, 2, mf::ui::WidgetState::normal, 1.0F);
    const auto inactivePulse = mf::ui::segmentedLevelIndicatorSegmentColor(
        fourthOfFive, 4, mf::ui::WidgetState::normal, 1.0F);
    check(activePulse.g > activeIdle.g && activePulse.b > activeIdle.b &&
          inactivePulse == mf::ui::SegmentedLevelIndicatorStyle{}.inactive,
          "segmented indicator pulse is bounded to active changed state");

    constexpr mf::ui::WidgetId buttonId = 91;
    const mf::ui::Rect buttonBounds{{100, 100}, {240, 64}};
    mf::ui::Context ui;
    ui.beginFrame({180, 120}, true, false, false, false);
    auto button = ui.button(buttonId, buttonBounds);
    check(button.state == mf::ui::WidgetState::hovered && button.consumed && ui.inputConsumed(),
          "hovered button consumes pointer input");
    ui.endFrame();
    ui.beginFrame({180, 120}, true, true, true, false);
    button = ui.button(buttonId, buttonBounds);
    check(button.state == mf::ui::WidgetState::pressed && !button.activated,
          "button press captures without early activation");
    ui.endFrame();
    ui.beginFrame({180, 120}, true, false, false, true);
    button = ui.button(buttonId, buttonBounds);
    check(button.activated && button.consumed, "captured button activates on release inside");
    ui.endFrame();
    ui.beginFrame({180, 120}, true, false, false, false);
    button = ui.button(buttonId, buttonBounds, false);
    check(button.state == mf::ui::WidgetState::disabled && !button.activated && button.consumed,
          "disabled button retains interaction barrier");
    ui.endFrame();
    ui.setKeyboardFocus(buttonId);
    ui.beginFrame({0, 0}, false, false, false, false, true);
    button = ui.button(buttonId, buttonBounds);
    check(button.state == mf::ui::WidgetState::focused && button.activated && button.consumed,
          "focused button is keyboard activation ready");
    ui.endFrame();

    std::cout << "MediaForge render: " << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
