#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Enemy.hpp"
#include <vector>
#include "Dungeon.hpp"
#include "UI.hpp"

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

    Player player;
    Dungeon dungeon;
    UI ui;

    sf::Clock attackCooldown;
    bool canAttack() const;

    std::optional<sf::CircleShape> attackEffect;
    sf::Clock attackEffectTimer;

    
    void processEvents();
    void update();
    void render();
    void spawnEnemies();
    void restartGame();
    void handleCombat();
};