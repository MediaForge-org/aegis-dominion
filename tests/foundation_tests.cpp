#include "animation/SpriteAnimation.hpp"
#include "animation/Tween.hpp"
#include "assets/AssetCatalog.hpp"
#include "assets/AssetManager.hpp"
#include "assets/ResourceCache.hpp"
#include "input/InputSystem.hpp"
#include "logging/Logger.hpp"
#include "render/RenderContext.hpp"
#include "settings/Settings.hpp"
#include "ui/Interaction.hpp"
#include "ui/Layout.hpp"
#include "ui/Theme.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

sf::Event keyEvent(sf::Keyboard::Key key, bool control = false, bool shift = false) {
    sf::Event event{}; event.type = sf::Event::KeyPressed; event.key.code = key; event.key.control = control; event.key.shift = shift; return event;
}

void assetIdsAndCatalog() {
    using namespace aegis::assets;
    require(TextureId{"ui.panel.primary"}.valid(), "logical texture ID rejected");
    require(!TextureId{"assets/ui/panel.png"}.valid(), "free file path accepted as asset ID");
    auto catalog = AssetCatalog::builtIn();
    require(catalog.find("ui.logo", AssetType::Texture) != nullptr, "UI catalog entry missing");
    require(catalog.find("ui.logo", AssetType::Sound) == nullptr, "typed catalog lookup crossed types");
    require(!catalog.add({"ui.logo", AssetType::Texture, "duplicate.png"}), "duplicate catalog ID accepted");
}

void resourceCachingAndFallback() {
    using namespace aegis::assets;
    int fallback = -1;
    ResourceCache<TextureId, int> cache(fallback);
    int loads = 0;
    const TextureId valid{"ui.test"};
    auto loader = [&] { ++loads; return std::make_unique<int>(42); };
    auto& first = cache.get(valid, loader);
    auto& duplicate = cache.get(valid, loader);
    require(&first == &duplicate && first == 42 && loads == 1, "duplicate request did not use cached ownership");
    const TextureId missing{"ui.missing"};
    auto& missingResult = cache.get(missing, [] { return std::unique_ptr<int>{}; });
    require(&missingResult == &fallback, "missing resource did not return explicit fallback");
    const auto image = makeMissingTextureImage();
    require(image.getSize() == sf::Vector2u(32, 32), "missing texture image has wrong size");
    require(image.getPixel(0, 0) == sf::Color(255, 0, 190) && image.getPixel(8, 0) == sf::Color(20, 20, 24),
            "missing texture is not a conspicuous checkerboard");
}

void inputContextsAndBindings() {
    using namespace aegis::input;
    InputSystem input;
    input.setContext(Context::Gameplay);
    require(input.triggered(Action::StartWave, keyEvent(sf::Keyboard::Space)), "gameplay action mapping failed");
    require(input.triggered(Action::Pause, keyEvent(sf::Keyboard::P)), "alternate pause binding missing");
    input.setContext(Context::Menu);
    require(!input.triggered(Action::StartWave, keyEvent(sf::Keyboard::Space)), "action leaked across contexts");
    input.setContext(Context::MapForge);
    require(input.triggered(Action::EditorUndo, keyEvent(sf::Keyboard::Z, true)), "modifier binding failed");
    require(!input.triggered(Action::EditorUndo, keyEvent(sf::Keyboard::Z)), "binding conflict ignored modifier");
    input.bind(Context::Gameplay, Action::Pause, {sf::Keyboard::P});
    input.setContext(Context::Gameplay);
    require(input.triggered(Action::Pause, keyEvent(sf::Keyboard::P)), "rebinding preparation failed");
}

void pointerConsumption() {
    aegis::input::InputSystem input;
    input.beginFrame();
    sf::Event event{}; event.type = sf::Event::MouseButtonPressed; event.mouseButton.button = sf::Mouse::Left;
    input.handleEvent(event);
    require(input.pointerPressed(sf::Mouse::Left), "pointer press not recorded");
    require(input.consumePointerPress(sf::Mouse::Left), "pointer press not consumed");
    require(!input.pointerPressed(sf::Mouse::Left), "consumed UI pointer leaked to world");
}

void buttonInteractionStates() {
    aegis::ui::ButtonInteraction button({0.f, 0.f, 100.f, 40.f});
    button.pointerMove({10.f, 10.f}); require(button.state() == aegis::ui::ElementState::Hover, "hover state missing");
    button.pointerPress({10.f, 10.f}); require(button.state() == aegis::ui::ElementState::Pressed, "pressed state missing");
    require(button.pointerRelease({10.f, 10.f}), "valid click rejected");
    button.setSelected(true); require(button.state() == aegis::ui::ElementState::Selected, "selected state missing");
    button.setEnabled(false); button.pointerPress({10.f, 10.f}); require(!button.pointerRelease({10.f, 10.f}), "disabled button accepted input");
    require(button.state() == aegis::ui::ElementState::Disabled, "disabled state missing");
}

void layoutAndScale() {
    const std::vector<aegis::ui::LayoutItem> items{{100.f, 0.f}, {0.f, 1.f}, {0.f, 2.f}};
    const aegis::ui::LinearLayout layout(aegis::ui::Axis::Horizontal, 10.f, 20.f);
    const auto boxes = layout.calculate({0.f, 0.f, 600.f, 100.f}, items);
    require(boxes.size() == 3 && std::abs(boxes[0].width - 100.f) < .01f, "fixed layout size failed");
    require(std::abs(boxes[2].width - boxes[1].width * 2.f) < .02f, "flex layout ratio failed");
    aegis::ui::UiScale scale; scale.update(1280, 720); require(std::abs(scale.factor() - .8f) < .001f, "16:9 UI scale failed");
    scale.update(1024, 768); require(scale.viewport().height < 768.f && scale.viewport().top > 0.f, "letterbox calculation failed");
}

void themeAndSettings() {
    require(aegis::ui::Theme::command().complete(), "required theme tokens incomplete");
    aegis::settings::Settings settings; settings.uiScale = 5.f; settings.masterVolume = -1.f; settings.fpsLimit = 2; settings.sanitize();
    require(settings.uiScale == 2.f && settings.masterVolume == 0.f && settings.fpsLimit == 30, "settings bounds failed");
}

void loggingLevels() {
    std::ostringstream output;
    aegis::logging::Logger logger(output); logger.setLevel(aegis::logging::Level::Warning);
    logger.info("hidden {}", 1); logger.warning("visible {}", 2);
    require(output.str().find("hidden") == std::string::npos && output.str().find("[WARNING] visible 2") != std::string::npos,
            "logging level filtering failed");
}

void animationModels() {
    aegis::animation::Tween tween(0.f, 10.f, 1.f, aegis::animation::Easing::Linear); tween.restart(); tween.update(.5f);
    require(std::abs(tween.value() - 5.f) < .01f, "tween interpolation failed");
    const aegis::animation::AnimationDefinition definition{"enemy.scout.idle", {{{0, 0, 16, 16}, .1f}, {{16, 0, 16, 16}, .1f}}, aegis::animation::Playback::Loop};
    aegis::animation::SpriteAnimator animator; animator.play(definition); animator.update(.11f);
    require(animator.frameIndex() == 1 && animator.frame() != nullptr, "sprite frame progression failed");
    animator.update(.11f); require(animator.frameIndex() == 0, "sprite loop failed");
}

void renderLayerOrder() {
    aegis::render::RenderQueue queue; std::vector<int> order;
    queue.submit(aegis::render::Layer::ModalUi, [&] { order.push_back(3); });
    queue.submit(aegis::render::Layer::Terrain, [&] { order.push_back(1); });
    queue.submit(aegis::render::Layer::Enemies, [&] { order.push_back(2); });
    queue.execute(); require(order == std::vector<int>({1, 2, 3}) && queue.size() == 0, "render layers are not deterministic");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"asset IDs/catalog", assetIdsAndCatalog}, {"resource caching/fallback", resourceCachingAndFallback},
        {"input contexts/bindings", inputContextsAndBindings}, {"pointer consumption", pointerConsumption},
        {"button states", buttonInteractionStates}, {"layout/scaling", layoutAndScale},
        {"theme/settings", themeAndSettings}, {"logging levels", loggingLevels},
        {"animation", animationModels}, {"render layers", renderLayerOrder}};
    try { for (const auto& test : tests) test.second(); std::cout << tests.size() << " foundation test cases passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << "TEST FAILURE: " << error.what() << '\n'; return 1; }
}
