#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "UI.hpp"
#include "Loot.hpp"
#include "CombatSystem.hpp"
#include "FloorManager.hpp"

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
    sf::Font font;
    bool fontLoaded = false;
    std::mt19937 rng;
    sf::Clock frameClock;

    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;

    Dungeon dungeon;
    Player player;
    UI ui;
    LootSystem loot;

    CombatSystem combat;
    FloorManager floors;

    bool runEnded = false;

    void processEvents();
    void update();
    void render();
    void restartGame();
    void endRun();
    void saveRunStats();
    void handleInputDebug(float dt);
};
