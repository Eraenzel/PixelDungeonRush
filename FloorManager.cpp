#include "FloorManager.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <cmath>

FloorManager::FloorManager(Dungeon& dungeon, Player& player, std::vector<Enemy>& enemies,
                           std::vector<Pickup>& pickups, UI& ui, std::mt19937& rng)
    : dungeon(dungeon), player(player), enemies(enemies), pickups(pickups), ui(ui), rng(rng)
{
}

void FloorManager::reset() {
    floorNumber = 1;
    enemiesDefeated = 0;
    enemiesKilledThisFloor = 0;
    enemiesToClear = 0;
    enemiesToClearThisFloor = 0;
    enemiesToSpawn = Constants::Spawn::InitialEnemiesToSpawn;
    bossSpawned = false;
}

void FloorManager::startFloor() {
    dungeon.generate();
    dungeon.clearDiscovery();
    player.setPosition(dungeon.findSpawnPoint());
    enemies.clear();
    pickups.clear();
    spawnEnemies();
    enemiesKilledThisFloor = 0;
    bossSpawned = false;
    enemiesToClear = static_cast<int>(enemiesToSpawn * Constants::Spawn::EnemiesToClearFraction);
    enemiesToClearThisFloor = enemiesToClear;
    ui.markMinimapDirty();
}

void FloorManager::advanceFloor() {
    floorNumber++;
    enemiesToSpawn += floorNumber + 2;
    player.setHealth(player.getHealth() + 10.f);
    startFloor();
}

void FloorManager::onEnemiesKilled(int count, bool bossKilled) {
    enemiesDefeated += count;
    enemiesKilledThisFloor += count;
    enemiesToClearThisFloor = std::max(0, enemiesToClearThisFloor - count);

    if (!bossSpawned &&
        enemiesKilledThisFloor >= Constants::Gameplay::BossSpawnThreshold &&
        floorNumber % Constants::Gameplay::BossFloorInterval == 0) {
        spawnBoss();
        bossSpawned = true;
    }
}

void FloorManager::spawnEnemies() {
    std::vector<sf::Vector2f> validTiles = dungeon.getFloorTiles();
    sf::Vector2f playerPos = player.getPosition();

    std::vector<sf::Vector2f> filtered;
    for (const auto& pos : validTiles) {
        float dx = pos.x - playerPos.x;
        float dy = pos.y - playerPos.y;
        if (dx * dx + dy * dy > 200.f * 200.f)
            filtered.push_back(pos);
    }

    std::shuffle(filtered.begin(), filtered.end(), rng);

    for (int i = 0; i < enemiesToSpawn && i < static_cast<int>(filtered.size()); ++i)
        enemies.emplace_back(filtered[i], dungeon);
}

void FloorManager::spawnBoss() {
    std::vector<sf::Vector2f> floorTiles = dungeon.getFloorTiles();
    if (floorTiles.empty())
        return;

    sf::Vector2f playerPos = player.getPosition();

    auto hasClearance = [&](int tx, int ty) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = tx + dx;
                int ny = ty + dy;
                if (nx < 0 || ny < 0 || nx >= MAP_WIDTH || ny >= MAP_HEIGHT)
                    return false;
                if (!dungeon.isFloor(nx, ny))
                    return false;
            }
        }
        return true;
    };

    std::vector<sf::Vector2f> candidates;
    for (const auto& tilePos : floorTiles) {
        int tx = static_cast<int>(tilePos.x / TILE_SIZE);
        int ty = static_cast<int>(tilePos.y / TILE_SIZE);

        if (!hasClearance(tx, ty))
            continue;

        sf::Vector2f delta = tilePos - playerPos;
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq >= Constants::Gameplay::BossMinSpawnDist * Constants::Gameplay::BossMinSpawnDist &&
            distSq <= Constants::Gameplay::BossMaxSpawnDist * Constants::Gameplay::BossMaxSpawnDist) {
            candidates.push_back(tilePos);
        }
    }

    if (candidates.empty()) {
        for (const auto& tilePos : floorTiles) {
            int tx = static_cast<int>(tilePos.x / TILE_SIZE);
            int ty = static_cast<int>(tilePos.y / TILE_SIZE);
            if (hasClearance(tx, ty))
                candidates.push_back(tilePos);
        }
    }

    if (candidates.empty()) return;

    std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
    sf::Vector2f bossPos = candidates[dist(rng)] + sf::Vector2f{ TILE_SIZE * 0.5f, TILE_SIZE * 0.5f };

    enemies.emplace_back(bossPos, dungeon);
    enemies.back().makeBoss();
    ui.setBossMarker(bossPos);
}
