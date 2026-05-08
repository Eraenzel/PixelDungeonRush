#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <random>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Loot.hpp"
#include "Entity.hpp"
#include "Constants.hpp"

struct DamageNumber {
    sf::Text text;
    sf::Vector2f velocity;
    sf::Clock lifetime;

    DamageNumber(const sf::Font& font, const sf::Vector2f& worldPos, int value, const sf::Color& color)
        : text(font, std::to_string(value), 18), velocity(0.f, -30.f)
    {
        text.setFillColor(color);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1.f);
        text.setPosition(worldPos);
        lifetime.restart();
    }
};

struct AttackResult {
    int enemiesKilled = 0;
    bool bossKilled = false;
};

class CombatSystem {
public:
    CombatSystem(Player& player, std::vector<Enemy>& enemies, std::vector<Pickup>& pickups,
                 LootSystem& loot, std::mt19937& rng, const sf::Font& font, const bool& fontLoaded);

    AttackResult handlePlayerAttack();
    void handleEnemyAttacks(std::vector<Entity*>& blockers, float dt, int floorNumber);
    void updatePickups();
    void updateDamageNumbers(float dt);
    void render(sf::RenderWindow& window);
    void clear();

    bool canAttack() const;
    void setDropTable(std::vector<DropEntry> table);

private:
    Player& player;
    std::vector<Enemy>& enemies;
    std::vector<Pickup>& pickups;
    LootSystem& loot;
    std::mt19937& rng;
    const sf::Font& font;
    const bool& fontLoaded;

    sf::Clock attackCooldown;
    sf::Clock attackEffectTimer;
    std::optional<sf::CircleShape> attackEffect;
    std::vector<DamageNumber> damageNumbers;
    std::vector<DropEntry> dropTable;

    float rollDamage(float min, float max);
    void spawnDamageNumber(const sf::Vector2f& worldPos, float value, const sf::Color& color);
    void spawnPickup(const sf::Vector2f& pos);
};
