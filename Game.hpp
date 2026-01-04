#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include <vector>
#include "Dungeon.hpp"
#include "UI.hpp"
#include <random>

class Game {
public:
    Game();
    void run();

    enum class GameState {
        Playing,
        Dead
    };

    GameState state = GameState::Playing;

private:
    sf::RenderWindow window;
    sf::View camera;
    std::optional<sf::Event> event;
    std::vector<Enemy> enemies;
    sf::Font font;
    bool fontLoaded = false;
    std::mt19937 rng;
	sf::Clock frameClock;

    Player player;
    Dungeon dungeon;
    UI ui;

    sf::Clock attackCooldown;
    bool canAttack() const;

    std::optional<sf::CircleShape> attackEffect;
    sf::Clock attackEffectTimer;

    // Game constants
    static constexpr float AttackRadius = 40.f;
    static constexpr int   EnemiesPerRoom = 5;
    static constexpr float EnemyContactDPS = 30.f;
    static constexpr int AttackCooldownMs = 500;
	static constexpr int VisionRadiusTiles = 4;
    //static constexpr sf::Time AttackCooldown = sf::milliseconds(500);
    
    void processEvents();
    void update();
    void render();
    void spawnEnemies();
    void restartGame();
    void handleCombat();
    void handleInputDebug(float dt);
};