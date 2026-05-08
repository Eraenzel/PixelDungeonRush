#pragma once
#include <vector>
#include <random>
#include "Dungeon.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Loot.hpp"
#include "UI.hpp"
#include "Constants.hpp"

class FloorManager {
public:
    FloorManager(Dungeon& dungeon, Player& player, std::vector<Enemy>& enemies,
                 std::vector<Pickup>& pickups, UI& ui, std::mt19937& rng);

    void reset();
    void startFloor();
    void advanceFloor();
    void onEnemiesKilled(int count, bool bossKilled);

    int getFloorNumber() const { return floorNumber; }
    int getEnemiesDefeated() const { return enemiesDefeated; }
    int getEnemiesKilledThisFloor() const { return enemiesKilledThisFloor; }
    int getEnemiesToClear() const { return enemiesToClear; }
    int getEnemiesToClearThisFloor() const { return enemiesToClearThisFloor; }
    bool isBossSpawned() const { return bossSpawned; }

    void spawnBoss();

private:
    Dungeon& dungeon;
    Player& player;
    std::vector<Enemy>& enemies;
    std::vector<Pickup>& pickups;
    UI& ui;
    std::mt19937& rng;

    int floorNumber = 1;
    int enemiesDefeated = 0;
    int enemiesKilledThisFloor = 0;
    int enemiesToClear = 0;
    int enemiesToClearThisFloor = 0;
    int enemiesToSpawn = Constants::Spawn::InitialEnemiesToSpawn;
    bool bossSpawned = false;

    void spawnEnemies();
};
