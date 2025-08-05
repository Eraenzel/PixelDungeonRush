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
    for (auto& row : currentlyVisible) row.fill(false);


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
            if (!discovered[y][x]) continue;

            sf::RectangleShape tile = (map[y][x] == 1) ? wallTile : floorTile;
            tile.setPosition(sf::Vector2f{ x * TILE_SIZE, y * TILE_SIZE });

            if (!currentlyVisible[y][x]) {
                tile.setFillColor(tile.getFillColor() * sf::Color(80, 80, 80, 255)); // darken
            }
            window.draw(tile);
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

bool Dungeon::markVisible(int centerX, int centerY, int radius) {
    // Clear previous frame’s visibility
    for (auto& row : currentlyVisible) row.fill(false);

    bool revealedSomething = false;

    sf::Vector2f from = sf::Vector2f(centerX * TILE_SIZE + TILE_SIZE / 2.f,
        centerY * TILE_SIZE + TILE_SIZE / 2.f);

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int nx = centerX + dx;
            int ny = centerY + dy;
            if (nx < 0 || ny < 0 || nx >= MAP_WIDTH || ny >= MAP_HEIGHT) continue;

            sf::Vector2f to = sf::Vector2f(nx * TILE_SIZE + TILE_SIZE / 2.f,
                ny * TILE_SIZE + TILE_SIZE / 2.f);
            if (lineOfSightClear(from, to)) {
                if (!discovered[ny][nx]) revealedSomething = true;
                discovered[ny][nx] = true;
                currentlyVisible[ny][nx] = true;
            }
        }
    }
    return revealedSomething;
}


std::vector<sf::Vector2f> Dungeon::getFloorTiles() const {
    std::vector<sf::Vector2f> floorTiles;
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map[y][x] == 0) {
                floorTiles.emplace_back(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
    return floorTiles;
}

bool Dungeon::lineOfSightClear(const sf::Vector2f& from, const sf::Vector2f& to) const {
    sf::Vector2i fromTile = sf::Vector2i(from / TILE_SIZE);
    sf::Vector2i toTile = sf::Vector2i(to / TILE_SIZE);

    int dx = std::abs(toTile.x - fromTile.x);
    int dy = std::abs(toTile.y - fromTile.y);
    int sx = (fromTile.x < toTile.x) ? 1 : -1;
    int sy = (fromTile.y < toTile.y) ? 1 : -1;
    int err = dx - dy;

    int x = fromTile.x;
    int y = fromTile.y;

    while (x != toTile.x || y != toTile.y) {
        if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
        if (map[y][x] == 1) return false;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }

    return true;
}


