#include "GameScreen.hpp"

#include "Common.hpp"
#include "app/ScreenManager.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include "logging/Logger.hpp"
#include "render/RenderContext.hpp"
#include "render/WorldRenderer.hpp"
#include <sstream>

namespace aegis::screens {
namespace {
const std::vector<TowerKind> TowerOrder = {TowerKind::Pulse, TowerKind::Cannon, TowerKind::Frost, TowerKind::Sniper, TowerKind::Tesla, TowerKind::Missile};
const sf::FloatRect StartWaveRect(1230.f, 72.f, 340.f, 54.f);
const sf::FloatRect SpeedRect(1230.f, 138.f, 160.f, 44.f);
const sf::FloatRect PauseRect(1410.f, 138.f, 160.f, 44.f);
const sf::FloatRect TargetRect(1230.f, 448.f, 164.f, 42.f);
const sf::FloatRect SellRect(1406.f, 448.f, 164.f, 42.f);
const sf::FloatRect UpgradeRect(1230.f, 500.f, 340.f, 46.f);
const sf::FloatRect BranchARect(1230.f, 494.f, 164.f, 62.f);
const sf::FloatRect BranchBRect(1406.f, 494.f, 164.f, 62.f);

sf::FloatRect towerCardRect(int index) {
    const int row = index / 2;
    const int column = index % 2;
    return {1230.f + static_cast<float>(column) * 174.f, 565.f + static_cast<float>(row) * 104.f, 164.f, 94.f};
}
}

GameScreen::GameScreen(app::ScreenContext context, core::GameLaunchConfig launch)
    : Screen(context), ui_(context.window, context.assets), launch_(std::move(launch)) {
    context_.input.setContext(input::Context::Gameplay);
    if (const auto* standard = std::get_if<core::StandardGameLaunch>(&launch_)) {
        map_.loadFromStandardMap(standard->map);
        logging::log().info("Starting game with map: {} / {}", map_.data().id, map_.data().name);
    } else {
        const auto& playtest = std::get<core::MapForgePlaytestLaunch>(launch_);
        map_.loadFromPlayableMap(playtest.map);
        logging::log().info("Starting MAP FORGE playtest with map: {} / {}", playtest.map.id, playtest.map.name);
    }
    resetGameplay();
}

bool GameScreen::isPlaytest() const { return std::holds_alternative<core::MapForgePlaytestLaunch>(launch_); }

void GameScreen::resetGameplay() {
    enemies_.clear();
    towers_.clear();
    projectiles_.clear();
    effects_.clear();
    waves_.reset();
    credits_ = 520;
    coreHealth_ = 20;
    score_ = kills_ = 0;
    nextEnemyId_ = 1;
    speed_ = 1;
    paused_ = helpOverlay_ = gameOver_ = victory_ = false;
    selectedTower_ = -1;
    buildKind_.reset();
}

void GameScreen::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Closed) { context_.screens.quit(); return; }
    if (event.type == sf::Event::KeyPressed) {
        if (context_.input.triggered(input::Action::Pause, event)) {
            if (helpOverlay_) helpOverlay_ = false;
            else paused_ = !paused_;
        }
        if (context_.input.triggered(input::Action::OpenHelp, event)) helpOverlay_ = !helpOverlay_;
        if (!gameOver_ && !victory_) {
            if (context_.input.triggered(input::Action::StartWave, event)) startNextWave();
            if (context_.input.triggered(input::Action::GameSpeedUp, event)) speed_ = speed_ == 1 ? 2 : 1;
            if (context_.input.triggered(input::Action::UpgradeTower, event)) upgradeSelected();
            if (context_.input.triggered(input::Action::ChangeTargeting, event) && selectedTower_ >= 0) towers_[static_cast<std::size_t>(selectedTower_)]->cycleTargetMode();
            if (context_.input.triggered(input::Action::SellTower, event)) sellSelected();
            const input::Action towerActions[] = {input::Action::Tower1, input::Action::Tower2, input::Action::Tower3,
                                                  input::Action::Tower4, input::Action::Tower5, input::Action::Tower6};
            for (std::size_t i = 0; i < TowerOrder.size(); ++i)
                if (context_.input.triggered(towerActions[i], event)) selectBuild(TowerOrder[i]);
        }
    }
    if (event.type == sf::Event::MouseButtonPressed) {
        const auto position = context_.window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        if (position.x >= WORLD_W || paused_ || helpOverlay_ || gameOver_ || victory_)
            context_.input.consumePointerPress(event.mouseButton.button);
        processClick(position, event.mouseButton.button);
    }
}

void GameScreen::update(float deltaSeconds) {
    elapsed_ += deltaSeconds;
    screenShake_=std::max(0.f,screenShake_-deltaSeconds*11.f);
    if (paused_ || helpOverlay_ || gameOver_ || victory_) {
        effects_.update(deltaSeconds * .3f);
        return;
    }
    const float step = deltaSeconds * static_cast<float>(speed_);
    waves_.update(step);
    if (waves_.hasSpawn()) {
        auto enemy = makeEnemy(waves_.consumeSpawn(), nextEnemyId_++, waves_.enemyScale());
        enemy->setPathState(0, 0.f, map_);
        enemies_.push_back(std::move(enemy));
    }
    for (auto& enemy : enemies_) enemy->update(step, map_);
    for (auto& tower : towers_) tower->update(step, enemies_, projectiles_, effects_, context_.assets);
    for (auto& projectile : projectiles_) projectile.update(step, enemies_, effects_);
    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), [](const Projectile& projectile) { return !projectile.alive; }), projectiles_.end());

    std::vector<std::unique_ptr<Enemy>> splitChildren;
    for (auto& enemy : enemies_) {
        if (enemy->reachedEnd() && !enemy->dead()) {
            const int leak = enemy->kind() == EnemyKind::Boss ? 5 : (enemy->kind() == EnemyKind::Tank ? 2 : 1);
            coreHealth_ -= leak;
            effects_.text(enemy->position() + sf::Vector2f(-20.f, -30.f), "-" + std::to_string(leak) + " KERN", ui::Red);
            enemy->kill();
            context_.assets.play("explosion", 35.f);
        } else if (enemy->dead()) {
            credits_ += enemy->reward();
            score_ += enemy->reward() * 11;
            ++kills_;
            effects_.text(enemy->position() + sf::Vector2f(-12.f, -28.f), "+" + std::to_string(enemy->reward()), ui::Gold);
            sf::Color deathColor=map_.data().accent;
            if(enemy->kind()==EnemyKind::Shield)deathColor=sf::Color(90,190,255);else if(enemy->kind()==EnemyKind::Regen)deathColor=sf::Color(90,235,140);
            else if(enemy->kind()==EnemyKind::Splitter)deathColor=sf::Color(215,120,255);else if(enemy->kind()==EnemyKind::Tank||enemy->kind()==EnemyKind::Boss)deathColor=sf::Color(255,125,72);
            effects_.burst(enemy->position(),deathColor,enemy->kind()==EnemyKind::Boss?42:enemy->kind()==EnemyKind::Tank?24:16,enemy->kind()==EnemyKind::Boss?230.f:130.f);
            if(enemy->kind()==EnemyKind::Tank||enemy->kind()==EnemyKind::Boss)for(int puff=0;puff<5;++puff)effects_.trail(enemy->position()+sf::Vector2f(static_cast<float>(puff*6-12),0.f),{0.f,-35.f-static_cast<float>(puff*5)},sf::Color(90,92,96,150),8.f,.65f,-12.f);
            if (enemy->kind() == EnemyKind::Splitter) {
                for (int i = 0; i < 2; ++i) {
                    auto child = makeEnemy(EnemyKind::Runner, nextEnemyId_++, std::max(.7f, waves_.enemyScale() * .72f));
                    child->setPathState(enemy->segment(), clampf(enemy->segmentT() + static_cast<float>(i) * .018f, 0.f, .98f), map_);
                    splitChildren.push_back(std::move(child));
                }
            }
        }
    }
    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const auto& enemy) { return enemy->dead(); }), enemies_.end());
    for (auto& child : splitChildren) enemies_.push_back(std::move(child));

    if (coreHealth_ <= 0) {
        coreHealth_ = 0;
        gameOver_ = true;
        context_.assets.play("gameover", 65.f);
    }
    if (waves_.active() && waves_.spawningDone() && enemies_.empty()) {
        const int bonus = waves_.completionBonus();
        credits_ += bonus;
        score_ += bonus * 5;
        waves_.finishWave();
        effects_.text({520.f, 105.f}, "WELLE GESICHERT  +" + std::to_string(bonus) + " C", ui::Green);
        context_.assets.play("upgrade", 50.f);
        if (waves_.wave() >= waves_.maxWaves()) {
            victory_ = true;
            score_ += coreHealth_ * 250;
        }
    }
    effects_.update(step);
    screenShake_=std::max(screenShake_,effects_.consumeShake());
}

void GameScreen::render() {
    render::RenderContext renderContext(context_.window);
    const auto stableView=context_.window.getView();
    if(screenShake_>.05f){auto shaken=stableView;shaken.move(std::sin(elapsed_*91.f)*screenShake_,std::cos(elapsed_*77.f)*screenShake_*.65f);context_.window.setView(shaken);}
    render::WorldRenderer world(renderContext, context_.assets);
    world.draw(buildRenderSnapshot());
    drawBuildPreview();
    renderContext.setLayer(render::Layer::Effects);
    effects_.draw(context_.window, context_.assets.text().loaded() ? &context_.assets.text() : nullptr);
    context_.window.setView(stableView);
    renderContext.setLayer(render::Layer::ScreenUi);
    drawHud();
    drawCommandPanel();
    renderContext.setLayer(render::Layer::ModalUi);
    if (helpOverlay_) drawHelpOverlay();
    else if (paused_) drawPauseOverlay();
    if (gameOver_ || victory_) drawEndOverlay();
}

void GameScreen::leaveGame() {
    if (isPlaytest()) context_.screens.pop();
    else context_.screens.replace({app::ScreenType::MainMenu});
}

void GameScreen::processClick(sf::Vector2f position, sf::Mouse::Button button) {
    if (button == sf::Mouse::Right) { buildKind_.reset(); selectedTower_ = -1; return; }
    if (button != sf::Mouse::Left) return;
    if (gameOver_ || victory_) {
        if (sf::FloatRect(555.f, 560.f, 220.f, 54.f).contains(position)) resetGameplay();
        else if (sf::FloatRect(790.f, 560.f, 220.f, 54.f).contains(position)) {
            if (isPlaytest()) leaveGame();
            else { app::ScreenRequest request{app::ScreenType::MainMenu}; request.openMapSelect = true; context_.screens.replace(std::move(request)); }
        } else if (!isPlaytest() && sf::FloatRect(673.f, 630.f, 220.f, 48.f).contains(position)) leaveGame();
        return;
    }
    if (helpOverlay_) { if (sf::FloatRect(690.f, 720.f, 220.f, 50.f).contains(position)) helpOverlay_ = false; return; }
    if (paused_) {
        if (sf::FloatRect(690.f, 430.f, 220.f, 54.f).contains(position)) paused_ = false;
        else if (sf::FloatRect(690.f, 500.f, 220.f, 50.f).contains(position)) leaveGame();
        return;
    }
    if (position.x < WORLD_W) {
        if (buildKind_) {
            const int cost = towerCost(*buildKind_);
            if (credits_ >= cost && map_.canBuild(position, towerPositions())) {
                credits_ -= cost;
                towers_.push_back(makeTower(*buildKind_, position));
                effects_.ring(position, ui::Green, 65.f, .35f);
                context_.assets.play("build", 52.f);
            }
        } else selectedTower_ = towerAt(position);
        return;
    }
    if (StartWaveRect.contains(position)) { startNextWave(); return; }
    if (SpeedRect.contains(position)) { speed_ = speed_ == 1 ? 2 : 1; return; }
    if (PauseRect.contains(position)) { paused_ = true; return; }
    for (int i = 0; i < 6; ++i) if (towerCardRect(i).contains(position)) { selectBuild(TowerOrder[static_cast<std::size_t>(i)]); return; }
    if (selectedTower_ >= 0) {
        auto& tower = *towers_[static_cast<std::size_t>(selectedTower_)];
        if (TargetRect.contains(position)) tower.cycleTargetMode();
        else if (SellRect.contains(position)) sellSelected();
        else if (tower.needsBranch() && BranchARect.contains(position)) upgradeSelected(UpgradeBranch::A);
        else if (tower.needsBranch() && BranchBRect.contains(position)) upgradeSelected(UpgradeBranch::B);
        else if (UpgradeRect.contains(position)) upgradeSelected();
    }
}

void GameScreen::startNextWave() {
    if (gameOver_ || victory_ || paused_) return;
    if (waves_.startNext()) {
        context_.assets.play("wave", 60.f);
        effects_.text({505.f, 100.f}, "WELLE " + std::to_string(waves_.wave()) + " BEGINNT", ui::Cyan);
    }
}

void GameScreen::selectBuild(TowerKind kind) { buildKind_ = kind; selectedTower_ = -1; context_.assets.play("click", 38.f); }

std::vector<sf::Vector2f> GameScreen::towerPositions() const {
    std::vector<sf::Vector2f> positions;
    for (const auto& tower : towers_) positions.push_back(tower->position());
    return positions;
}

int GameScreen::towerAt(sf::Vector2f position) const {
    for (int i = static_cast<int>(towers_.size()) - 1; i >= 0; --i)
        if (distance(position, towers_[static_cast<std::size_t>(i)]->position()) < 43.f) return i;
    return -1;
}

void GameScreen::sellSelected() {
    if (selectedTower_ < 0 || selectedTower_ >= static_cast<int>(towers_.size())) return;
    credits_ += towers_[static_cast<std::size_t>(selectedTower_)]->sellValue();
    towers_.erase(towers_.begin() + selectedTower_);
    selectedTower_ = -1;
}

void GameScreen::upgradeSelected(UpgradeBranch branch) {
    if (selectedTower_ < 0 || selectedTower_ >= static_cast<int>(towers_.size())) return;
    auto& tower = *towers_[static_cast<std::size_t>(selectedTower_)];
    if (tower.maxLevel() || (tower.needsBranch() && branch == UpgradeBranch::None) || credits_ < tower.upgradeCost()) return;
    const int cost = tower.upgradeCost();
    if (tower.upgrade(branch)) {
        credits_ -= cost;
        effects_.ring(tower.position(), branch == UpgradeBranch::B ? ui::Orange : ui::Cyan, 70.f, .55f);
        context_.assets.play("upgrade", 58.f);
    }
}

render::WorldRenderSnapshot GameScreen::buildRenderSnapshot() const {
    render::WorldRenderSnapshot snapshot;
    snapshot.map=map_.renderSnapshot(buildKind_.has_value(), false); snapshot.elapsedSeconds=elapsed_;
    snapshot.enemies.reserve(enemies_.size());
    for (const auto& enemy : enemies_) snapshot.enemies.push_back(enemy->renderSnapshot(false));
    snapshot.towers.reserve(towers_.size());
    for (std::size_t i=0;i<towers_.size();++i) snapshot.towers.push_back(towers_[i]->renderSnapshot(static_cast<int>(i)==selectedTower_));
    snapshot.projectiles.reserve(projectiles_.size());
    for (const auto& projectile : projectiles_) if(projectile.alive) snapshot.projectiles.push_back(projectile.renderSnapshot());
    return snapshot;
}

void GameScreen::drawBuildPreview() {
    if (buildKind_) {
        const auto mouse = context_.window.mapPixelToCoords(sf::Mouse::getPosition(context_.window));
        if (mouse.x < WORLD_W) {
            const bool valid=map_.canBuild(mouse,towerPositions());
            sf::CircleShape footprint(46.f,48); footprint.setOrigin(46.f,46.f); footprint.setPosition(mouse);
            footprint.setFillColor(valid?sf::Color(70,230,155,34):sf::Color(255,80,95,38)); footprint.setOutlineColor(valid?ui::Green:ui::Red); footprint.setOutlineThickness(2.f); context_.window.draw(footprint);
            drawTowerIcon(*buildKind_, mouse, .72f, 0.f, valid ? sf::Color(190, 255, 220, 210) : sf::Color(255, 145, 150, 210));
        }
    }
}

void GameScreen::drawStatBadge(const std::string& icon, const std::string& value, sf::Vector2f position, sf::Color accent) {
    ui_.panel({position.x, position.y, 146.f, 48.f}, sf::Color(7, 14, 22, 205), withAlpha(accent, 80), 1.f);
    sf::Sprite image(context_.assets.ui(icon));
    image.setPosition(position.x + 10.f, position.y + 8.f);
    image.setScale(.5f, .5f);
    context_.window.draw(image);
    ui_.text(value, 21, {position.x + 48.f, position.y + 13.f}, ui::Text, true);
}

void GameScreen::drawHud() {
    ui_.card({18.f, 18.f, 640.f, 64.f}, map_.data().accent, true);
    drawStatBadge("credits", std::to_string(credits_), {30.f, 26.f}, ui::Gold);
    drawStatBadge("core", std::to_string(coreHealth_), {184.f, 26.f}, ui::Red);
    drawStatBadge("wave", std::to_string(waves_.wave()) + "/20", {338.f, 26.f}, ui::Cyan);
    drawStatBadge("score", std::to_string(score_), {492.f, 26.f}, ui::Purple);
    ui_.card({820.f, 20.f, 350.f, 48.f}, map_.data().accent, true);
    const auto mapLabel = (isPlaytest() ? "PLAYTEST · " : "") + map_.data().name + " · " +
                          std::to_string(static_cast<int>(map_.data().sourceWidth)) + "×" + std::to_string(static_cast<int>(map_.data().sourceHeight));
    ui_.text(mapLabel, 15, {835.f, 35.f}, ui::Text, true);
}

void GameScreen::drawCommandPanel() {
    ui_.card({PANEL_X, 0.f, PANEL_W, WORLD_H}, map_.data().accent, false);
    ui_.text(isPlaytest() ? "MAP FORGE PLAYTEST" : "AEGIS COMMAND", 22, {1230.f, 24.f}, ui::Text, true);
    ui_.button(StartWaveRect, waves_.active() ? "WELLE LÄUFT" : "WELLE STARTEN  [SPACE]", waves_.active() ? ui::Muted : ui::Green, waves_.active());
    ui_.button(SpeedRect, speed_ == 1 ? "ZEIT  1×" : "ZEIT  2×", ui::Gold, speed_ == 2, 16);
    ui_.button(PauseRect, "PAUSE", ui::Cyan, false, 16);
    ui_.card({1230.f, 198.f, 340.f, 132.f}, ui::Cyan, true);
    ui_.text("BEDROHUNGSANALYSE", 14, {1248.f, 214.f}, ui::Muted, true);
    ui_.text("Welle " + std::to_string(std::min(20, waves_.wave() + 1)), 22, {1248.f, 242.f}, ui::Text, true);
    int shown = 0;
    for (const auto kind : waves_.preview()) {
        if (shown >= 7) break;
        drawEnemyIcon(kind, {1260.f + static_cast<float>(shown) * 42.f, 304.f}, .32f);
        ++shown;
    }
    if (selectedTower_ >= 0 && selectedTower_ < static_cast<int>(towers_.size())) drawSelectedTowerPanel();
    else {
        ui_.card({1230.f, 346.f, 340.f, 188.f}, ui::Purple, true);
        ui_.text("BAUPLAN", 18, {1248.f, 365.f}, ui::Text, true);
        ui_.wrapped("Wähle unten einen Turm. Klicke auf einen gebauten Turm für Upgrades, Verkauf und Zielpriorität.", 15, {1248.f, 400.f, 300.f, 100.f}, ui::Muted, 5.f);
    }
    drawBuildCards();
    ui_.text(isPlaytest() ? "ESC Pause · zurück über Pausenmenü" : "F1 Anleitung · ESC Pause", 12, {1230.f, 877.f}, sf::Color(100, 127, 146));
}

void GameScreen::drawSelectedTowerPanel() {
    auto& tower = *towers_[static_cast<std::size_t>(selectedTower_)];
    const auto stats = tower.stats();
    ui_.card({1230.f, 346.f, 340.f, 206.f}, ui::Cyan, true);
    drawTowerIcon(tower.kind(), {1282.f, 392.f}, .55f, tower.rotation());
    ui_.text(towerName(tower.kind()) + "  L" + std::to_string(tower.level()), 21, {1325.f, 365.f}, ui::Text, true);
    ui_.text("DMG " + std::to_string(static_cast<int>(stats.damage)) + "  RNG " + std::to_string(static_cast<int>(stats.range)), 14, {1325.f, 405.f}, ui::Gold, true);
    ui_.button(TargetRect, "ZIEL: " + targetModeName(tower.targetMode()), ui::Cyan, false, 13);
    ui_.button(SellRect, "VERKAUF +" + std::to_string(tower.sellValue()), ui::Gold, false, 13);
    if (tower.needsBranch()) {
        ui_.button(BranchARect, tower.branchAName(), ui::Cyan, false, 12);
        ui_.button(BranchBRect, tower.branchBName(), ui::Orange, false, 12);
    } else ui_.button(UpgradeRect, tower.maxLevel() ? "MAXIMALE STUFE" : "UPGRADE [U]", tower.maxLevel() ? ui::Muted : ui::Green, false, 15);
}

void GameScreen::drawBuildCards() {
    const auto mouse = context_.window.mapPixelToCoords(sf::Mouse::getPosition(context_.window));
    for (int i = 0; i < 6; ++i) {
        const auto kind = TowerOrder[static_cast<std::size_t>(i)];
        const auto rect = towerCardRect(i);
        const bool active = buildKind_ && *buildKind_ == kind;
        ui_.panel(rect, rect.contains(mouse) ? sf::Color(22, 37, 50) : sf::Color(16, 28, 40), active ? map_.data().accent : withAlpha(map_.data().accent, 70), active ? 2.f : 1.f);
        drawTowerIcon(kind, {rect.left + 39.f, rect.top + 43.f}, .38f);
        ui_.text(towerName(kind), 15, {rect.left + 76.f, rect.top + 17.f}, ui::Text, true);
        ui_.text(std::to_string(towerCost(kind)) + " C", 14, {rect.left + 76.f, rect.top + 42.f}, credits_ >= towerCost(kind) ? ui::Gold : ui::Red, true);
    }
}

void GameScreen::drawTowerIcon(TowerKind kind, sf::Vector2f center, float scale, float rotation, sf::Color tint) {
    sf::Sprite base(context_.assets.towerBase(kind));
    auto bounds = base.getLocalBounds();
    base.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    base.setPosition(center); base.setScale(scale, scale); base.setColor(tint); context_.window.draw(base);
    sf::Sprite turret(context_.assets.towerTurret(kind));
    bounds = turret.getLocalBounds();
    turret.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    turret.setPosition(center); turret.setScale(scale, scale); turret.setRotation(rotation); turret.setColor(tint); context_.window.draw(turret);
}

void GameScreen::drawEnemyIcon(EnemyKind kind, sf::Vector2f center, float scale) {
    sf::Sprite sprite(context_.assets.enemy(kind));
    const auto bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    sprite.setPosition(center); sprite.setScale(scale, scale); context_.window.draw(sprite);
}

void GameScreen::drawPauseOverlay() {
    ui_.panel({0.f, 0.f, 1600.f, 900.f}, sf::Color(0, 0, 0, 150));
    ui_.panel({585.f, 300.f, 430.f, 290.f}, sf::Color(9, 18, 28, 250), sf::Color(75, 145, 180, 150), 2.f);
    ui_.text("PAUSE", 40, {800.f, 355.f}, ui::Text, true, true);
    ui_.button({690.f, 430.f, 220.f, 54.f}, "WEITER", ui::Green);
    ui_.button({690.f, 500.f, 220.f, 50.f}, isPlaytest() ? "ZURÜCK ZU MAP FORGE" : "HAUPTMENÜ", ui::Red, false, 15);
}

void GameScreen::drawHelpOverlay() {
    ui_.panel({0.f, 0.f, 1600.f, 900.f}, sf::Color(0, 0, 0, 165));
    ui_.panel({390.f, 125.f, 820.f, 650.f}, sf::Color(8, 17, 27, 252), sf::Color(80, 173, 215, 170), 2.f);
    ui_.text("KURZANLEITUNG", 32, {800.f, 172.f}, ui::Cyan, true, true);
    ui_.wrapped("1–6 Turm wählen · Linksklick bauen · SPACE Welle starten · U Upgrade · T Zielmodus · S Verkaufen · F Zeitfaktor · P/ESC Pause", 20, {455.f, 250.f, 690.f, 240.f}, ui::Text, 10.f);
    ui_.button({690.f, 720.f, 220.f, 50.f}, "VERSTANDEN", ui::Green);
}

void GameScreen::drawEndOverlay() {
    ui_.panel({0.f, 0.f, 1600.f, 900.f}, sf::Color(0, 0, 0, 176));
    ui_.panel({470.f, 250.f, 660.f, 440.f}, sf::Color(8, 17, 27, 252), victory_ ? ui::Green : ui::Red, 2.5f);
    ui_.text(victory_ ? "SEKTOR GESICHERT" : "KERN VERLOREN", 38, {800.f, 315.f}, victory_ ? ui::Green : ui::Red, true, true);
    ui_.text("Welle " + std::to_string(waves_.wave()) + "  ·  Abschüsse " + std::to_string(kills_) + "  ·  Score " + std::to_string(score_), 20, {800.f, 450.f}, ui::Text, true, true);
    ui_.button({555.f, 560.f, 220.f, 54.f}, "NOCH EINMAL", ui::Green);
    ui_.button({790.f, 560.f, 220.f, 54.f}, isPlaytest() ? "ZURÜCK ZU MAP FORGE" : "KARTENWAHL", ui::Cyan, false, 15);
    if (!isPlaytest()) ui_.button({673.f, 630.f, 220.f, 48.f}, "HAUPTMENÜ", ui::Muted, false, 15);
}

} // namespace aegis::screens
