#include "UI.hpp"
#include "Dungeon.hpp"

UI::UI(const Dungeon& dungeon) : dungeonRef(dungeon) {
    sf::RenderTexture minimapTexture(sf::Vector2u{ MAP_WIDTH, MAP_HEIGHT });
    minimapTexture.clear(sf::Color::Black);

    sf::RectangleShape pixel({ 1.f, 1.f });
    const MapArray& map = dungeonRef.getMap();

    // Draw dungeon layout ONCE
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            pixel.setFillColor(map[y][x] == 1 ? sf::Color(80, 80, 80)
                : sf::Color(180, 180, 180));
            pixel.setPosition({ (float)x, (float)y });
            minimapTexture.draw(pixel);
        }
    }
    minimapTexture.display();

    minimapBg.setSize(sf::Vector2f{ MAP_WIDTH * MINIMAP_SCALE + 4, MAP_HEIGHT * MINIMAP_SCALE + 4 });
    minimapBg.setFillColor(sf::Color(20, 20, 20, 200));
    minimapBg.setPosition(sf::Vector2f{ 8, 8 });

    minimapSprite.emplace(minimapTexture.getTexture());
    minimapSprite->setScale(sf::Vector2f{ MINIMAP_SCALE, MINIMAP_SCALE });
    minimapSprite->setPosition(sf::Vector2f{ 10.f, 10.f });
}

void UI::draw(sf::RenderWindow& window, const Player& player) {

    // draw minimap frame
    window.draw(minimapBg);

    // draw minimap
    minimapSprite->setTexture(minimapTexture.getTexture());
    window.draw(*minimapSprite);

    // draw player marker on minimap
    sf::RectangleShape playerMarker(sf::Vector2f{ 5.f, 5.f });
    playerMarker.setFillColor(sf::Color::Green);
    playerMarker.setPosition(sf::Vector2f{
        minimapSprite->getPosition().x + (player.getPosition().x / TILE_SIZE) * MINIMAP_SCALE,
        minimapSprite->getPosition().y + (player.getPosition().y / TILE_SIZE) * MINIMAP_SCALE
        });
    window.draw(playerMarker);
}
