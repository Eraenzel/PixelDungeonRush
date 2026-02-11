#include "Loot.hpp"

LootSystem::LootSystem(std::mt19937& rng)
    : rng(rng)
{
    commonTable = {
        0.7f,
        1, 1,
        {
            { Pickup::Type::Heal, 0.9f, 15.f, 1.0f },
            { Pickup::Type::SpeedBoost, 0.1f, 15.f, 15.f}
        }
    };

    eliteTable = {
        0.75f,
		1, 2,
        {
            { Pickup::Type::Heal, 1.f, 30.f, 1.0f },
            { Pickup::Type::DamageBoost, 1.f, 10.f, 20.f },
            { Pickup::Type::SpeedBoost, 1.f, 25.f, 20.f}
        }
    };

    bossTable = {
        1.0f,
		2, 4,
        {
            { Pickup::Type::Heal, 1.f, 50.f, 1.0f },
            { Pickup::Type::DamageBoost, 1.f, 20.f, 30.0f },
            { Pickup::Type::SpeedBoost, 1.f, 30.f, 30.f}
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

    std::uniform_int_distribution<int> rollCountDist(
        table.minRolls, table.maxRolls);

    int rolls = rollCountDist(rng);

    float totalWeight = 0.f;
    for (const auto& e : table.entries)
        totalWeight += e.chance;

    std::uniform_real_distribution<float> pickDist(0.f, totalWeight);

    for (int i = 0; i < rolls; ++i) {

        float roll = pickDist(rng);

        for (const auto& e : table.entries) {
            roll -= e.chance;
            if (roll <= 0.f) {
                result.emplace_back(position, e.type, e.value, e.duration);
                break;
            }
        }
    }

    return result;
}


