#include "Loot.hpp"

LootSystem::LootSystem(std::mt19937& rng)
    : rng(rng)
{
    commonTable = {
        0.7f,
        {
            { Pickup::Type::Heal, 1.f, 15.f, 1.0f }
        }
    };

    eliteTable = {
        0.75f,
        {
            { Pickup::Type::Heal, 30.f, 1.0f },
            { Pickup::Type::DamageBoost, 10.f, 0.5f }
        }
    };

    bossTable = {
        1.0f,
        {
            { Pickup::Type::Heal, 50.f, 1.0f },
            { Pickup::Type::DamageBoost, 20.f, 1.0f }
        }
    };
}

const DropTable& LootSystem::getTable(EnemyRarity rarity) const {
    switch (rarity) {
    case EnemyRarity::Elite: return eliteTable;
    case EnemyRarity::Boss:  return bossTable;
    default:                 return commonTable;
    }
}

std::vector<Pickup> LootSystem::rollDrops(
    EnemyRarity rarity,
    const sf::Vector2f& position)
{
    std::vector<Pickup> result;
    const DropTable& table = getTable(rarity);

    std::uniform_real_distribution<float> chanceDist(0.f, 1.f);
    if (chanceDist(rng) > table.dropChance)
        return result;

    float totalWeight = 0.f;
    for (const auto& e : table.entries)
        totalWeight += e.chance;

    std::uniform_real_distribution<float> pickDist(0.f, totalWeight);
    float roll = pickDist(rng);

    for (const auto& e : table.entries) {
        roll -= e.chance;
        if (roll <= 0.f) {
            result.emplace_back(position, e.type, e.value, e.duration);
            break;
        }
    }

    return result;
}

