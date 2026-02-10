#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "UI.hpp"
#include <random>
#include "Loot.hpp"

struct DamageNumber {
    sf::Text text;
    sf::Vector2f velocity;
    sf::Clock lifetime;

    DamageNumber(
        const sf::Font& font,
        const sf::Vector2f& worldPos,
        int value,
        const sf::Color& color
    )
        : text(font, std::to_string(value), 18),
        velocity(0.f, -30.f)
    {
        text.setFillColor(color);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1.f);
        text.setPosition(worldPos);
        lifetime.restart();
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
    std::vector<DamageNumber> damageNumbers;
    std::vector<DropEntry> enemyDropTable;



    Player player;
    Dungeon dungeon;
    UI ui;
	LootSystem loot;
	bool bossAlive = true;
	int enemiesDefeated = 0;
    int floorNumber = 1;
    int enemiesKilledThisFloor = 0;
    int enemiesToClear = 0;
    int enemiesToClearThisFloor = 0;
	int enemiesToSpawn = 5; 

    sf::Clock attackCooldown;
    bool canAttack() const;

    std::optional<sf::CircleShape> attackEffect;
    sf::Clock attackEffectTimer;

    // Game constants
    static constexpr float AttackRadius = 40.f;
    static constexpr int EnemiesPerRoom = 10;
    static constexpr float EnemyContactDPS = 30.f;
    static constexpr int AttackCooldownMs = 500;
	static constexpr int VisionRadiusTiles = 5;
    sf::Time AttackEffectDuration = sf::milliseconds(100);
	static constexpr float BossSpawnThreshold = 10; // enemies defeated before boss spawns
	bool bossSpawned = false;
	bool runEnded = false;
    static constexpr float BossMinSpawnDist = 6.f * TILE_SIZE;
    static constexpr float BossMaxSpawnDist = 12.f * TILE_SIZE;
	static constexpr float PickupSpawnChance = 0.9f; // X% chance to drop a pickup


    //static constexpr sf::Time AttackCooldown = sf::milliseconds(500);
    
    void processEvents();
    void update();
    void render();
    void spawnEnemies();
    void restartGame();
    void handlePlayerAttack();
	void handleEnemyAttacks(std::vector<Entity*>& blockers, float dt);
    void handleInputDebug(float dt);
	void spawnBoss();
	void endRun();
	float rollDamage(float min, float max);
    void spawnDamageNumber(
        const sf::Vector2f& worldPos,
        float value,
        const sf::Color& color
    );
	void spawnPickup(const sf::Vector2f& pos);
    void startFloor();
    void advanceFloor();
	void saveRunStats();


};