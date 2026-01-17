#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "UI.hpp"
#include <random>

struct Pickup {
    enum class Type {
        Heal
    };

    sf::Vector2f position;
    Type type;
    float value; // heal amount

    sf::CircleShape shape;

    Pickup(sf::Vector2f pos, Type t, float v)
        : position(pos), type(t), value(v), shape(8.f) {

        shape.setOrigin({ 8.f, 8.f });
        shape.setPosition(position);
        shape.setFillColor(sf::Color(80, 255, 80));
    }
};

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
    std::vector<Pickup> pickups;


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
	static constexpr int VisionRadiusTiles = 5;
    sf::Time AttackEffectDuration = sf::milliseconds(100);

    //static constexpr sf::Time AttackCooldown = sf::milliseconds(500);
    
    void processEvents();
    void update();
    void render();
    void spawnEnemies();
    void restartGame();
    void handleCombat();
    void handleInputDebug(float dt);
	void spawnBoss();

};