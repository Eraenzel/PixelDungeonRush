#include <SFML/Graphics.hpp>
#include <optional>
#include <array>
#include <random>


// ----------------- CONFIG -----------------
constexpr float TILE_SIZE = 32.f;          // zoomed-in main view
constexpr int MAP_WIDTH = 100;             // much bigger dungeon
constexpr int MAP_HEIGHT = 72;
constexpr float MINIMAP_SCALE = 3.0f;      // minimap size (20% scale)
constexpr int ROOM_ATTEMPTS = 40;          // tries to place rooms
constexpr int MAX_ROOMS = 12;

using MapArray = std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT>;
using DiscoverArray = std::array<std::array<bool, MAP_WIDTH>, MAP_HEIGHT>;

// ----------------- ROOM STRUCT -----------------
struct Room {
    int x, y, w, h;
    int centerX() const { return x + w / 2; }
    int centerY() const { return y + h / 2; }
};

// ----------------- ROOM HELPERS -----------------
bool roomOverlaps(const Room& a, const Room& b) {
    // Add padding of 1 tile around rooms so they don't touch
    return !(a.x + a.w + 1 < b.x || b.x + b.w + 1 < a.x ||
        a.y + a.h + 1 < b.y || b.y + b.h + 1 < a.y);
}


void carveRoom(MapArray& map, const Room& r) {
    for (int yy = r.y; yy < r.y + r.h && yy < MAP_HEIGHT - 1; ++yy) {
        for (int xx = r.x; xx < r.x + r.w && xx < MAP_WIDTH - 1; ++xx) {
            map[yy][xx] = 0;
        }
    }
}


void carveCorridor(MapArray& map, int x1, int y1, int x2, int y2) {
    // L-shaped corridors, 2 tiles wide
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

// ----------------- DUNGEON GENERATION -----------------
void generateDungeon(MapArray& map) {
    for (auto& row : map) row.fill(1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> roomXDist(1, MAP_WIDTH - 10);
    std::uniform_int_distribution<int> roomYDist(1, MAP_HEIGHT - 10);
    std::uniform_int_distribution<int> roomSize(5, 9);

    std::vector<Room> rooms;

    // place up to MAX_ROOMS rooms with overlap check
    for (int i = 0; i < ROOM_ATTEMPTS && rooms.size() < MAX_ROOMS; ++i) {
        Room r{ roomXDist(gen), roomYDist(gen), roomSize(gen), roomSize(gen) };

        bool overlaps = false;
        for (const auto& other : rooms) {
            if (roomOverlaps(r, other)) { overlaps = true; break; }
        }

        if (!overlaps) {
            carveRoom(map, r);
            rooms.push_back(r);
        }
    }

    // connect rooms
    for (size_t i = 1; i < rooms.size(); ++i) {
        carveCorridor(map, rooms[i - 1].centerX(), rooms[i - 1].centerY(),
            rooms[i].centerX(), rooms[i].centerY());
    }
}

// ----------------- PLAYER SPAWN -----------------
sf::Vector2f findSpawnPoint(const MapArray& map) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map[y][x] == 0) {
                return sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
    return { TILE_SIZE, TILE_SIZE }; // fallback (shouldn’t happen)
}

// ----------------- COLLISION -----------------
bool canMoveTo(const MapArray& map, const sf::FloatRect& bounds) {
    // Check all four corners of the player's bounding box
    std::array<sf::Vector2f, 4> corners = {
        sf::Vector2f{bounds.position.x, bounds.position.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y},
        sf::Vector2f{bounds.position.x, bounds.position.y + bounds.size.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y}
    };

    for (const auto& corner : corners) {
        int tileX = static_cast<int>(corner.x / TILE_SIZE);
        int tileY = static_cast<int>(corner.y / TILE_SIZE);

        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT)
            return false; // outside map is treated as wall

        if (map[tileY][tileX] == 1)
            return false;
    }

    return true;
}
void createMinimap()
{

}

// ----------------- MAIN -----------------
int main() {
    // --- WINDOW SETUP ---
    sf::RenderWindow window(sf::VideoMode({ static_cast<unsigned int>(1280),
                                            static_cast<unsigned int>(720) }),
        "Pixel Dungeon Rush");
    window.setFramerateLimit(60);

    // --- MAIN DATA ---
    MapArray map{};
    generateDungeon(map);

    DiscoverArray discovered{};
    for (auto& row : discovered) row.fill(false);

    // --- TILE SHAPES ---
    sf::RectangleShape floorTile({ TILE_SIZE, TILE_SIZE });
    floorTile.setFillColor(sf::Color(50, 50, 50));

    sf::RectangleShape wallTile({ TILE_SIZE, TILE_SIZE });
    wallTile.setFillColor(sf::Color(100, 100, 100));

    // --- PLAYER ---
    sf::RectangleShape player({ TILE_SIZE - 2.f, TILE_SIZE - 2.f }); // slightly smaller than tile
    player.setFillColor(sf::Color::Green);
    player.setPosition(findSpawnPoint(map));

    // --- CAMERA ---
    sf::View camera;
    camera.setSize(sf::Vector2f{
    static_cast<float>(window.getSize().x),
    static_cast<float>(window.getSize().y) });
    camera.zoom(0.4f);

    // --- MINIMAP RENDER TEXTURE ---
    sf::RenderTexture minimapTexture(sf::Vector2u{ MAP_WIDTH, MAP_HEIGHT });
    
    // draw the dungeon base ONCE into the minimap
    sf::RectangleShape pixel(sf::Vector2f{ 1.f, 1.f });
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            pixel.setFillColor(map[y][x] == 1 ? sf::Color(80, 80, 80) : sf::Color(180, 180, 180));
            pixel.setPosition(sf::Vector2f{ static_cast<float>(x), static_cast<float>(y) });
            minimapTexture.draw(pixel);
        }
    }
    minimapTexture.display();
    
    sf::Sprite minimapSprite(minimapTexture.getTexture());  
    minimapSprite.setScale(sf::Vector2f{ MINIMAP_SCALE, MINIMAP_SCALE });
    minimapSprite.setPosition(sf::Vector2f{ 10.f, 10.f });

    // minimap background frame
    sf::RectangleShape minimapBg;
    minimapBg.setSize(sf::Vector2f{ MAP_WIDTH * MINIMAP_SCALE + 4,
                                    MAP_HEIGHT * MINIMAP_SCALE + 4 });
    //minimapBg.setFillColor(sf::Color(20, 20, 20, 200));
    minimapBg.setFillColor(sf::Color(255, 0, 0, 150)); // red transparent for now
    minimapBg.setPosition(sf::Vector2f{ 8, 8 });

    sf::RectangleShape debugSquare;
    debugSquare.setSize(sf::Vector2f{ 50, 50 });
    debugSquare.setFillColor(sf::Color::Magenta);
    debugSquare.setPosition(sf::Vector2f{ 10.f, 10.f });
    window.draw(debugSquare);

    // --- GAME LOOP ---
    while (window.isOpen()) {
        // --- EVENT HANDLING ---
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // --- RESTART/EXIT ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            generateDungeon(map);
            player.setPosition(findSpawnPoint(map));
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            window.close();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.zoom(0.9f); // zoom in slowly
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.zoom(1.1f); // zoom out slowly

        // --- MOVEMENT INPUT ---
        float speed = 2.5f;
        sf::Vector2f movement{ 0.f, 0.f };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += speed;

        // --- COLLISION CHECK ---
        sf::FloatRect nextBounds = player.getGlobalBounds();
        nextBounds.position.x += movement.x;
        nextBounds.position.y += movement.y;

        if (canMoveTo(map, nextBounds)) {
            player.move(movement);
        }

        // --- UPDATE CAMERA ---
        camera.setCenter(player.getPosition());
        window.setView(camera);

        // --- MARK FOG DISCOVERY ---
        int tileX = static_cast<int>(player.getPosition().x / TILE_SIZE);
        int tileY = static_cast<int>(player.getPosition().y / TILE_SIZE);
        for (int yy = -2; yy <= 2; ++yy) {
            for (int xx = -2; xx <= 2; ++xx) {
                int nx = tileX + xx;
                int ny = tileY + yy;
                if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                    discovered[ny][nx] = true;
                }
            }
        }

        // --- DRAW EVERYTHING ---
        window.clear(sf::Color::Black);

        // draw visible tiles only
        float halfWidth = camera.getSize().x / 2.f;
        float halfHeight = camera.getSize().y / 2.f;
        float left = camera.getCenter().x - halfWidth;
        float top = camera.getCenter().y - halfHeight;

        int startX = std::max(0, static_cast<int>(left / TILE_SIZE) - 1);
        int startY = std::max(0, static_cast<int>(top / TILE_SIZE) - 1);
        int endX = std::min(MAP_WIDTH, startX + static_cast<int>(window.getSize().x / TILE_SIZE) + 3);
        int endY = std::min(MAP_HEIGHT, startY + static_cast<int>(window.getSize().y / TILE_SIZE) + 3);

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                sf::RectangleShape& tile = (map[y][x] == 1) ? wallTile : floorTile;
                tile.setPosition(sf::Vector2f{ x * TILE_SIZE, y * TILE_SIZE });
                window.draw(tile);
            }
        }
        window.draw(player);

        // --- HUD / MINIMAP ---
        window.setView(window.getDefaultView());

        // draw minimap frame
        window.draw(minimapBg);

        // draw minimap sprite (already has map)
        window.draw(minimapSprite);

        // draw player marker on top of minimap
        sf::RectangleShape playerMarker(sf::Vector2f{ 5.f, 5.f });
        playerMarker.setFillColor(sf::Color::Green);
        playerMarker.setPosition(sf::Vector2f{
            minimapSprite.getPosition().x + (player.getPosition().x / TILE_SIZE) * MINIMAP_SCALE,
            minimapSprite.getPosition().y + (player.getPosition().y / TILE_SIZE) * MINIMAP_SCALE
            });
        window.draw(playerMarker);

        window.display();    
    }
}
