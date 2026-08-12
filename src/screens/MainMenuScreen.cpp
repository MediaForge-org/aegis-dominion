#include "MainMenuScreen.hpp"

#include "Common.hpp"
#include "app/ScreenManager.hpp"

#include <array>
#include <cmath>
#include <random>

namespace aegis::screens {
namespace {
const sf::FloatRect PlayButton(610.f, 445.f, 380.f, 58.f);
const sf::FloatRect ForgeButton(610.f, 515.f, 380.f, 58.f);
const sf::FloatRect TutorialButton(610.f, 585.f, 380.f, 58.f);
const sf::FloatRect ExitButton(610.f, 655.f, 380.f, 58.f);
const sf::FloatRect BackButton(48.f, 818.f, 180.f, 50.f);

const std::array<const char*, 6> TutorialTitles = {
    "1  DAS ZIEL", "2  TÜRME BAUEN", "3  WELLEN & GEGNER",
    "4  UPGRADES", "5  TAKTIK", "6  STEUERUNG"
};
const std::array<const char*, 6> TutorialBodies = {
    "Kein Feind darf den Aegis-Kern am Ende der Straße erreichen. Du startest mit 20 Kernpunkten und Credits. Besiegte Gegner bringen neue Credits; nach Welle 20 ist der Sektor gesichert.",
    "Wähle rechts einen Turm oder drücke 1–6. Bewege den Mauszeiger über die Karte und klicke auf eine freie Fläche. Grün bedeutet gültig, Rot bedeutet blockiert.",
    "Mit LEERTASTE startest du die nächste Angriffswelle. Jäger sind schnell, Brecher gepanzert, Schildträger geschützt und Regeneratoren heilen sich. Jede fünfte Welle bringt einen Titan.",
    "Wähle einen gebauten Turm, um Reichweite, Schaden, Zielpriorität und Upgrades zu sehen. Nach Stufe 2 entscheidest du dich dauerhaft für einen Spezialisierungsweg.",
    "Kombiniere Kontrolle, Flächenschaden und Einzelschaden. Kryo hält Gruppen in Feuerzonen, Mörser und Raketen treffen Gruppen, Railguns eliminieren harte Ziele.",
    "1–6 Turm wählen  •  Linksklick bauen/auswählen  •  Rechtsklick abbrechen  •  SPACE Welle  •  U Upgrade  •  T Zielmodus  •  S Verkauf  •  F Tempo  •  P/ESC Pause  •  F1 Hilfe"
};
}

MainMenuScreen::MainMenuScreen(app::ScreenContext context, bool openMapSelect) : Screen(context), ui_(context.window, context.assets) {
    context_.input.setContext(input::Context::Menu);
    std::mt19937 random{73};
    std::uniform_real_distribution<float> x(0.f, static_cast<float>(WINDOW_W));
    std::uniform_real_distribution<float> y(0.f, static_cast<float>(WINDOW_H));
    std::uniform_real_distribution<float> speed(7.f, 24.f);
    std::uniform_real_distribution<float> radius(1.f, 3.5f);
    for (int i = 0; i < 90; ++i) particles_.push_back({{x(random), y(random)}, {-speed(random) * .25f, speed(random)}, radius(random), sf::Color(85, 190, 255, 60)});
    map_.set(0);
    if (openMapSelect) page_ = Page::MapSelect;
    pageAppear_.restart();
}

void MainMenuScreen::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Closed) { context_.screens.quit(); return; }
    if (context_.input.triggered(input::Action::MenuBack, event)) {
        if (page_ == Page::Menu) context_.screens.quit();
        else page_ = Page::Menu;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        context_.input.consumePointerPress(sf::Mouse::Left);
        click(context_.window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}));
    }
}

void MainMenuScreen::click(sf::Vector2f position) {
    const auto previousPage = page_;
    if (page_ == Page::Menu) {
        if (PlayButton.contains(position)) page_ = Page::MapSelect;
        else if (ForgeButton.contains(position)) context_.screens.replace({app::ScreenType::MapForge});
        else if (TutorialButton.contains(position)) { tutorialPage_ = 0; page_ = Page::Tutorial; }
        else if (ExitButton.contains(position)) context_.screens.quit();
    } else if (page_ == Page::MapSelect) {
        for (int i = 0; i < 3; ++i) {
            if (sf::FloatRect(70.f + static_cast<float>(i) * 510.f, 220.f, 440.f, 500.f).contains(position)) {
                app::ScreenRequest request{app::ScreenType::Game};
                request.gameLaunch = core::GameLaunchConfig{core::StandardGameLaunch{i}};
                context_.screens.replace(std::move(request));
                return;
            }
        }
        if (BackButton.contains(position)) page_ = Page::Menu;
    } else {
        const sf::FloatRect previous(500.f, 800.f, 180.f, 52.f);
        const sf::FloatRect menu(700.f, 800.f, 200.f, 52.f);
        const sf::FloatRect next(920.f, 800.f, 180.f, 52.f);
        if (previous.contains(position) && tutorialPage_ > 0) --tutorialPage_;
        else if (next.contains(position) && tutorialPage_ < 5) ++tutorialPage_;
        else if (menu.contains(position)) page_ = Page::Menu;
    }
    if (page_ != previousPage) pageAppear_.restart();
    context_.assets.play("click", 50.f);
}

void MainMenuScreen::update(float deltaSeconds) {
    pageAppear_.update(deltaSeconds);
    for (auto& particle : particles_) {
        particle.position += particle.velocity * deltaSeconds;
        if (particle.position.y > static_cast<float>(WINDOW_H) + 10.f) {
            particle.position.y = -10.f;
            particle.position.x = std::fmod(particle.position.x + 213.f, static_cast<float>(WINDOW_W));
        }
    }
}

void MainMenuScreen::render() {
    if (page_ == Page::Menu) drawMenu();
    else if (page_ == Page::MapSelect) drawMapSelect();
    else drawTutorial();
    ui_.panel({0.f, 0.f, 1600.f * pageAppear_.value(), 3.f}, withAlpha(page_ == Page::Tutorial ? ui::Green : ui::Cyan, 210));
}

void MainMenuScreen::drawMenu() {
    sf::Sprite background(context_.assets.ui("menu_background"));
    context_.window.draw(background);
    for (const auto& particle : particles_) {
        sf::CircleShape shape(particle.radius);
        shape.setOrigin(particle.radius, particle.radius);
        shape.setPosition(particle.position);
        shape.setFillColor(particle.color);
        context_.window.draw(shape);
    }
    sf::Sprite logo(context_.assets.ui("logo"));
    const auto bounds = logo.getLocalBounds();
    logo.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    logo.setPosition(800.f, 225.f);
    logo.setScale(.86f, .86f);
    context_.window.draw(logo);
    ui_.wrapped("Verteidige den Aegis-Kern oder erschaffe in MAP FORGE eigene Einsatzgebiete.", 20, {520.f, 340.f, 560.f, 70.f}, sf::Color(176, 199, 216), 6.f);
    ui_.card({580.f, 420.f, 440.f, 320.f}, ui::Cyan, true);
    ui_.button(PlayButton, "SPIEL STARTEN", ui::Cyan, false, 22);
    ui_.button(ForgeButton, "MAP FORGE", ui::Orange, false, 22);
    ui_.button(TutorialButton, "ANLEITUNG", ui::Green, false, 21);
    ui_.button(ExitButton, "BEENDEN", ui::Red, false, 20);
    ui_.text("C++23  •  SFML  •  MAP FORGE", 16, {800.f, 780.f}, ui::Muted, false, true);
}

void MainMenuScreen::drawMapSelect() {
    ui_.text("EINSATZGEBIET WÄHLEN", 38, {800.f, 72.f}, ui::Text, true, true);
    ui_.text("Jede Karte verändert gute Baupositionen und die optimale Turmwahl.", 18, {800.f, 118.f}, ui::Muted, false, true);
    const auto mouse = context_.window.mapPixelToCoords(sf::Mouse::getPosition(context_.window));
    for (int i = 0; i < 3; ++i) {
        map_.set(i);
        const sf::FloatRect rect(70.f + static_cast<float>(i) * 510.f, 220.f, 440.f, 500.f);
        const bool hovered = rect.contains(mouse);
        ui_.card(rect, withAlpha(map_.data().accent, hovered ? 255 : 150), true);
        sf::Sprite preview(context_.assets.mapTexture(i));
        preview.setPosition(rect.left + 20.f, rect.top + 20.f);
        preview.setScale(400.f / 1200.f, 280.f / 900.f);
        context_.window.draw(preview);
        ui_.text(map_.data().name, 28, {rect.left + 22.f, rect.top + 322.f}, ui::Text, true);
        ui_.text(map_.data().subtitle, 16, {rect.left + 22.f, rect.top + 360.f}, map_.data().accent, true);
        ui_.wrapped(map_.data().description, 16, {rect.left + 22.f, rect.top + 394.f, 392.f, 72.f}, ui::Muted, 4.f);
    }
    map_.set(0);
    ui_.iconButton(BackButton, "ZURÜCK", ui::UiIcon::ArrowLeft, ui::IconPlacement::Left, ui::Cyan);
}

void MainMenuScreen::drawTutorial() {
    sf::Sprite background(context_.assets.ui("menu_background"));
    background.setColor(sf::Color(150, 160, 170));
    context_.window.draw(background);
    ui_.card({330.f, 90.f, 940.f, 670.f}, ui::Cyan, true);
    ui_.separator({430.f, 205.f}, {1170.f, 205.f}, withAlpha(ui::Cyan, 100));
    ui_.text(TutorialTitles[static_cast<std::size_t>(tutorialPage_)], 34, {800.f, 160.f}, ui::Cyan, true, true);
    ui_.wrapped(TutorialBodies[static_cast<std::size_t>(tutorialPage_)], 22, {430.f, 240.f, 740.f, 330.f}, ui::Text, 10.f);
    ui_.text(std::to_string(tutorialPage_ + 1) + " / 6", 16, {800.f, 720.f}, ui::Muted, false, true);
    ui_.progressBar({620.f, 744.f, 360.f, 7.f}, static_cast<float>(tutorialPage_ + 1) / 6.f, ui::Cyan);
    ui_.iconButton({500.f, 800.f, 180.f, 52.f}, "ZURÜCK", ui::UiIcon::ArrowLeft,
                   ui::IconPlacement::Left, ui::Cyan, tutorialPage_ == 0, 17);
    ui_.button({700.f, 800.f, 200.f, 52.f}, "HAUPTMENÜ", ui::Muted, false, 17);
    ui_.iconButton({920.f, 800.f, 180.f, 52.f}, "WEITER", ui::UiIcon::ArrowRight,
                   ui::IconPlacement::Right, ui::Green, tutorialPage_ == 5, 17);
}

} // namespace aegis::screens
