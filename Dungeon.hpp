#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

// ---- CONFIG ----
constexpr float TILE_SIZE = 32.f;
constexpr int MAP_WIDTH = 100;
constexpr int MAP_HEIGHT = 72;
constexpr float MINIMAP_SCALE = 3.0f;
constexpr int ROOM_ATTEMPTS = 40;
constexpr int MAX_ROOMS = 12;

using MapArray = std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT>;

struct Room {
    int x, y, w, h;
    int centerX() const { return x + w / 2; }
    int centerY() const { return y + h / 2; }
};

class Dungeon {
public:
    Dungeon();
    void generate();
    void draw(sf::RenderWindow& window) const;
    sf::Vector2f findSpawnPoint() const;
    const MapArray& getMap() const { return map; }

private:
    MapArray map;
    sf::RectangleShape floorTile;
    sf::RectangleShape wallTile;

    bool roomOverlaps(const Room& a, const Room& b);
    void carveRoom(const Room& r);
    void carveCorridor(int x1, int y1, int x2, int y2);
};
