#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

enum class EnemyRarity {
    Common,
    Elite,
    Boss
};

struct Pickup {
    enum class Type {
        Heal,
        DamageBoost,
        SpeedBoost
    };

    sf::Vector2f position;
    Type type;
    float value; // heal amount
    float duration; // for buffs

    sf::CircleShape shape;

    Pickup(sf::Vector2f pos, Type t, float v, float d)
        : position(pos), type(t), value(v), duration(d), shape(8.f) {

        shape.setOrigin({ 8.f, 8.f });
        shape.setPosition(position);
        shape.setOutlineThickness(1.f);
		shape.getOutlineColor() = sf::Color::Black;

        /*switch (type) {
        case Pickup::Type::Heal:        shape.setFillColor(sf::Color::Green); break;
        case Pickup::Type::DamageBoost: shape.setFillColor(sf::Color::Red);   break;
        case Pickup::Type::SpeedBoost:  shape.setFillColor(sf::Color::Cyan);  break;
        }*/

        switch (type) {
        case Type::Heal:
            shape.setFillColor(sf::Color(80, 255, 80)); // green
            break;

        case Type::DamageBoost:
            shape.setFillColor(sf::Color(255, 80, 80)); // red
            break;

        case Type::SpeedBoost:
            shape.setFillColor(sf::Color(80, 200, 255)); // cyan
            break;
        }

    }
};

struct DropEntry {
    Pickup::Type type;
    float chance;   // relative weight
    float value;
    float duration;
};

struct DropTable {
    float dropChance;              // 0–1, overall chance to drop anything
	int minRolls;                  // minimum number of rolls if dropping
	int maxRolls;                  // maximum number of rolls if dropping
    std::vector<DropEntry> entries;
};

class LootSystem {
public:
    LootSystem(std::mt19937& rng);

    std::vector<Pickup> rollDrops(
        EnemyRarity rarity,
        const sf::Vector2f& position
    );

private:
    std::mt19937& rng;

    DropTable commonTable;
    DropTable eliteTable;
    DropTable bossTable;

    const DropTable& getTable(EnemyRarity rarity) const;
};
