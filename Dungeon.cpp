#include "Dungeon.hpp"
#include <random>

Dungeon::Dungeon() {
    floorTile.setSize({ TILE_SIZE, TILE_SIZE });
    floorTile.setFillColor(sf::Color(50, 50, 50));
    wallTile.setSize({ TILE_SIZE, TILE_SIZE });
    wallTile.setFillColor(sf::Color(100, 100, 100));
}

bool Dungeon::roomOverlaps(const Room& a, const Room& b) {
    return !(a.x + a.w + 2 < b.x - 1 || b.x + b.w + 2 < a.x - 1 ||
        a.y + a.h + 2 < b.y - 1 || b.y + b.h + 2 < a.y - 1);
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

    // Horizontal segment
    while (x != x2) {
        for (int dy = -1; dy <= 1; ++dy) {
            int ny = y + dy;
            if (ny >= 0 && ny < MAP_HEIGHT && map[ny][x] == 1) {
                map[ny][x] = 0;
            }
        }
        x += (x2 > x) ? 1 : -1;
    }

    // Vertical segment
    while (y != y2) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            if (nx >= 0 && nx < MAP_WIDTH && map[y][nx] == 1) {
                map[y][nx] = 0;
            }
        }
        y += (y2 > y) ? 1 : -1;
    }
}

void Dungeon::generate() {

    for (auto& row : map) row.fill(1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> roomXDist(1, MAP_WIDTH - 10);
    std::uniform_int_distribution<int> roomYDist(1, MAP_HEIGHT - 10);
    std::uniform_int_distribution<int> roomWidth(8, 16);
    std::uniform_int_distribution<int> roomHeight(6, 12);

    std::vector<Room> rooms;

    for (int i = 0; i < ROOM_ATTEMPTS && rooms.size() < MAX_ROOMS; ++i) {
        Room r{ roomXDist(gen), roomYDist(gen), roomWidth(gen), roomHeight(gen) };

        bool overlaps = false;
        for (const auto& other : rooms) {
            if (roomOverlaps(r, other)) { overlaps = true; break; }
        }

        if (!overlaps) {
            carveRoom(r);
            rooms.push_back(r);
        }
    }

    std::uniform_int_distribution<int> roomPicker(0, static_cast<int>(rooms.size() - 1));
    for (size_t i = 1; i < rooms.size(); ++i) {
        int closestIndex = 0;
        int minDist = std::numeric_limits<int>::max();

        for (size_t j = 0; j < i; ++j) {
            int dx = rooms[i].centerX() - rooms[j].centerX();
            int dy = rooms[i].centerY() - rooms[j].centerY();
            int dist = dx * dx + dy * dy;  // Squared distance (faster than sqrt)

            if (dist < minDist) {
                minDist = dist;
                closestIndex = static_cast<int>(j);
            }
        }

        carveCorridor(rooms[i].centerX(), rooms[i].centerY(),
            rooms[closestIndex].centerX(), rooms[closestIndex].centerY());
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

void Dungeon::clearDiscovery() {
    for (auto& row : discovered) row.fill(false);
}

void Dungeon::markVisible(int centerX, int centerY, int radius) {
    for (int yy = -radius; yy <= radius; ++yy) {
        for (int xx = -radius; xx <= radius; ++xx) {
            int nx = centerX + xx;
            int ny = centerY + yy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                discovered[ny][nx] = true;
            }
        }
    }
}

