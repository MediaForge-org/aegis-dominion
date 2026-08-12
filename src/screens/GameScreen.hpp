#pragma once

#include "app/Screen.hpp"
#include "Effects.hpp"
#include "Enemy.hpp"
#include "Map.hpp"
#include "Projectile.hpp"
#include "Tower.hpp"
#include "WaveManager.hpp"
#include "ui/UiRenderer.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace aegis::screens {

class GameScreen final : public app::Screen {
public:
    GameScreen(app::ScreenContext context, core::GameLaunchConfig launch);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaSeconds) override;
    void render() override;
    void onResume() override { context_.input.setContext(input::Context::Gameplay); }

private:
    void resetGameplay();
    void processClick(sf::Vector2f position, sf::Mouse::Button button);
    void startNextWave();
    void selectBuild(TowerKind kind);
    void sellSelected();
    void upgradeSelected(UpgradeBranch branch = UpgradeBranch::None);
    std::vector<sf::Vector2f> towerPositions() const;
    int towerAt(sf::Vector2f position) const;

    void drawWorld();
    void drawHud();
    void drawCommandPanel();
    void drawBuildCards();
    void drawSelectedTowerPanel();
    void drawPauseOverlay();
    void drawHelpOverlay();
    void drawEndOverlay();
    void drawTowerIcon(TowerKind kind, sf::Vector2f center, float scale = .38f, float rotation = 0.f, sf::Color tint = sf::Color::White);
    void drawEnemyIcon(EnemyKind kind, sf::Vector2f center, float scale = .42f);
    void drawStatBadge(const std::string& icon, const std::string& value, sf::Vector2f position, sf::Color accent);
    void leaveGame();
    bool isPlaytest() const;

    ui::UiRenderer ui_;
    core::GameLaunchConfig launch_;
    GameMap map_;
    WaveManager waves_;
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::vector<std::unique_ptr<Tower>> towers_;
    std::vector<Projectile> projectiles_;
    Effects effects_;
    std::optional<TowerKind> buildKind_;
    int selectedTower_ = -1;
    int credits_ = 520;
    int coreHealth_ = 20;
    int score_ = 0;
    int kills_ = 0;
    int nextEnemyId_ = 1;
    int speed_ = 1;
    bool paused_ = false;
    bool helpOverlay_ = false;
    bool gameOver_ = false;
    bool victory_ = false;
    float elapsed_ = 0.f;
};

} // namespace aegis::screens
