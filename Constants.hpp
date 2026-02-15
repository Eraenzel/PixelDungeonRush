#pragma once
#include <SFML/System.hpp>

// Centralized compile-time constants.
// Grouped by subsystem to keep readability.
namespace Constants {

    namespace Map {
        inline constexpr float TILE_SIZE = 32.f;
        inline constexpr int MAP_WIDTH = 100;
        inline constexpr int MAP_HEIGHT = 72;
        inline constexpr float MINIMAP_SCALE = 2.0f;
        inline constexpr int ROOM_ATTEMPTS = 40;
        inline constexpr int MAX_ROOMS = 5;
    }

    namespace Gameplay {
        inline constexpr float AttackRadius = 40.f;
        inline constexpr int AttackCooldownMs = 500;
        inline constexpr float EnemyContactDPS = 30.f;
        inline constexpr int VisionRadiusTiles = 5;
        inline constexpr float PickupRadius = 20.f;           // pixels
        inline constexpr float DamageNumberLifetime = 0.6f;   // seconds
        inline constexpr float PickupSpawnChance = 0.9f;
        inline constexpr int BossSpawnThreshold = 10;
        inline constexpr int BossFloorInterval = 3;           // every N floors
        inline constexpr float BossMinSpawnDist = 6.f * Map::TILE_SIZE;
        inline constexpr float BossMaxSpawnDist = 12.f * Map::TILE_SIZE;
        inline const sf::Time AttackEffectDuration = sf::milliseconds(100);
    }

    namespace Spawn {
        inline constexpr int InitialEnemiesToSpawn = 6;
        inline constexpr float EnemiesToClearFraction = 0.4f; // portion of enemies needed to advance
    }

    namespace Player {
        inline constexpr float DefaultSpeed = 140.f; // px/s
        inline constexpr float MinSpeed = 60.f;
        inline constexpr float MaxSpeed = 500.f;
    }

    namespace UI {
        inline constexpr int MinimapScale = 2;
    }

} // namespace Constants