#include "Dungeon.hpp"
#include <random>

Dungeon::Dungeon() {
    floorTile.setSize({ TILE_SIZE, TILE_SIZE });
    floorTile.setFillColor(sf::Color(50, 50, 50));
    wallTile.setSize({ TILE_SIZE, TILE_SIZE });
    wallTile.setFillColor(sf::Color(100, 100, 100));
}

bool Dungeon::roomOverlaps(const Room& a, const Room& b) {
    return !(a.x + a.w + 1 < b.x || b.x + b.w + 1 < a.x ||
        a.y + a.h + 1 < b.y || b.y + b.h + 1 < a.y);
}

void Dungeon::carveRoom(const Room& r) {
    for (int yy = r.y; yy < r.y + r.h && yy < MAP_HEIGHT - 1; ++yy) {
        for (int xx = r.x; xx < r.x + r.w && xx < MAP_WIDTH - 1; ++xx) {
            map[yy][xx] = 0;
        }
    }
}

void Dungeon::carveCorridor(int x1, int y1, int x2, int y2) {
    int x = x1;
    int y = y1;
    while (x != x2) {
        map[y][x] = 0;
        if (y + 1 < MAP_HEIGHT) map[y + 1][x] = 0;
        x += (x2 > x) ? 1 : -1;
    }
    while (y != y2) {
        map[y][x] = 0;
        if (x + 1 < MAP_WIDTH) map[y][x + 1] = 0;
        y += (y2 > y) ? 1 : -1;
    }
}

void Dungeon::generate() {

    for (auto& row : map) row.fill(1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> roomXDist(1, MAP_WIDTH - 10);
    std::uniform_int_distribution<int> roomYDist(1, MAP_HEIGHT - 10);
    std::uniform_int_distribution<int> roomSize(5, 9);

    std::vector<Room> rooms;

    for (int i = 0; i < ROOM_ATTEMPTS && rooms.size() < MAX_ROOMS; ++i) {
        Room r{ roomXDist(gen), roomYDist(gen), roomSize(gen), roomSize(gen) };

        bool overlaps = false;
        for (const auto& other : rooms) {
            if (roomOverlaps(r, other)) { overlaps = true; break; }
        }

        if (!overlaps) {
            carveRoom(r);
            rooms.push_back(r);
        }
    }

    for (size_t i = 1; i < rooms.size(); ++i) {
        carveCorridor(rooms[i - 1].centerX(), rooms[i - 1].centerY(),
            rooms[i].centerX(), rooms[i].centerY());
    }
}

void Dungeon::draw(sf::RenderWindow& window) const {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            const sf::RectangleShape& tile = (map[y][x] == 1) ? wallTile : floorTile;
            sf::RectangleShape copy = tile;
            copy.setPosition(sf::Vector2f{ x * TILE_SIZE, y * TILE_SIZE });
            window.draw(copy);
        }
    }
}

sf::Vector2f Dungeon::findSpawnPoint() const {
    std::vector<sf::Vector2f> floorTiles;
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map[y][x] == 0) {
                floorTiles.emplace_back(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
    if (floorTiles.empty()) return { TILE_SIZE, TILE_SIZE }; // fallback

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, floorTiles.size() - 1);
    return floorTiles[dist(gen)];
}
