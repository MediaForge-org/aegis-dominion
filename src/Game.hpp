#pragma once
#include "Common.hpp"
#include "Assets.hpp"
#include "Map.hpp"
#include "Enemy.hpp"
#include "Tower.hpp"
#include "Projectile.hpp"
#include "Effects.hpp"
#include "WaveManager.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

class Game {
public:
    Game();
    int run();
private:
    void processEvents();
    void update(float dt);
    void render();

    void startGame(int mapIndex);
    void resetGameplay();
    void updateGameplay(float dt);
    void processGameplayClick(sf::Vector2f p, sf::Mouse::Button button);
    void processMenuClick(sf::Vector2f p);
    void processMapSelectClick(sf::Vector2f p);
    void processTutorialClick(sf::Vector2f p);

    void drawMenu();
    void drawMapSelect();
    void drawTutorial();
    void drawGameplay();
    void drawWorld();
    void drawHud();
    void drawCommandPanel();
    void drawBuildCards();
    void drawSelectedTowerPanel();
    void drawPauseOverlay();
    void drawHelpOverlay();
    void drawEndOverlay();

    void drawPanel(const sf::FloatRect& r, sf::Color fill, sf::Color outline=sf::Color::Transparent, float thickness=1.f);
    void drawText(const std::string& s, unsigned size, sf::Vector2f p, sf::Color color=sf::Color::White, bool bold=false, bool centered=false);
    void drawWrapped(const std::string& s, unsigned size, sf::FloatRect box, sf::Color color, float lineGap=5.f, bool bold=false);
    bool drawButton(const sf::FloatRect& r, const std::string& label, sf::Color accent, bool active=false, unsigned size=20);
    void drawStatBadge(const std::string& icon, const std::string& value, sf::Vector2f pos, sf::Color accent);
    void drawTowerIcon(TowerKind kind, sf::Vector2f center, float scale=.38f, float rotation=0.f, sf::Color tint=sf::Color::White);
    void drawEnemyIcon(EnemyKind kind, sf::Vector2f center, float scale=.42f);

    void startNextWave();
    void selectBuild(TowerKind k);
    void sellSelected();
    void upgradeSelected(UpgradeBranch branch=UpgradeBranch::None);
    std::vector<sf::Vector2f> towerPositions() const;
    int towerAt(sf::Vector2f p) const;

    sf::RenderWindow window_;
    Assets assets_;
    GameState state_=GameState::Menu;
    GameMap map_;
    WaveManager waves_;
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::vector<std::unique_ptr<Tower>> towers_;
    std::vector<Projectile> projectiles_;
    Effects fx_;

    std::optional<TowerKind> buildKind_;
    int selectedTower_=-1;
    int credits_=520;
    int core_=20;
    int score_=0;
    int kills_=0;
    int nextEnemyId_=1;
    int speed_=1;
    bool paused_=false;
    bool helpOverlay_=false;
    bool gameOver_=false;
    bool victory_=false;
    float elapsed_=0.f;
    int tutorialPage_=0;

    struct MenuParticle { sf::Vector2f p,v; float r; sf::Color c; };
    std::vector<MenuParticle> menuParticles_;
};
