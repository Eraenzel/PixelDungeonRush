#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "UI.hpp"
#include "Loot.hpp"
#include "Constants.hpp"
#include <random>

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

	// Game state
	bool bossSpawned = false;
	bool runEnded = false;
	int enemiesDefeated = 0;
	int floorNumber = 1;
	int enemiesKilledThisFloor = 0;
	int enemiesToClear = 0;
	int enemiesToClearThisFloor = 0;
	int enemiesToSpawn = Constants::Spawn::InitialEnemiesToSpawn;

	sf::Clock attackCooldown;
	bool canAttack() const;

	std::optional<sf::CircleShape> attackEffect;
	sf::Clock attackEffectTimer;

	// Game constants (moved to Constants.hpp)

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