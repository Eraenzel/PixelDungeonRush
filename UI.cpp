#include "UI.hpp"
#include "Dungeon.hpp"

UI::UI(const Dungeon& dungeon) : dungeonRef(dungeon) {
    minimapTexture = sf::RenderTexture(sf::Vector2u{ MAP_WIDTH, MAP_HEIGHT });


    minimapBg.setSize(sf::Vector2f{ MAP_WIDTH * MINIMAP_SCALE + 4, MAP_HEIGHT * MINIMAP_SCALE + 4 });
    minimapBg.setFillColor(sf::Color(20, 20, 20, 200));
    minimapBg.setPosition(sf::Vector2f{ 8, 8 });

    regenerateMinimap();
}

void UI::draw(sf::RenderWindow& window, const Player& player) {
    if (minimapDirty) {
        regenerateMinimap();
        minimapDirty = false;
    }
    // draw minimap frame
    window.draw(minimapBg);

    // draw minimap
    if (minimapSprite.has_value())
        window.draw(*minimapSprite);

    // draw player marker on minimap
    sf::RectangleShape playerMarker(sf::Vector2f{ 5.f, 5.f });
    playerMarker.setFillColor(sf::Color::Green);
    playerMarker.setPosition(sf::Vector2f{
        minimapSprite->getPosition().x + (player.getPosition().x / TILE_SIZE) * MINIMAP_SCALE,
        minimapSprite->getPosition().y + (player.getPosition().y / TILE_SIZE) * MINIMAP_SCALE
        });
    window.draw(playerMarker);
    drawPlayerHealth(window, player);

}

void UI::regenerateMinimap() {
    minimapTexture.clear(sf::Color::Black);

    sf::RectangleShape pixel(sf::Vector2f{ 1.f, 1.f });
    const MapArray& map = dungeonRef.getMap();

    const auto& discovered = dungeonRef.getDiscovered();

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (!discovered[y][x]) continue;

            pixel.setFillColor(map[y][x] == 1 ? sf::Color(80, 80, 80) : sf::Color(180, 180, 180));
            pixel.setPosition(sf::Vector2f{ static_cast<float>(x), static_cast<float>(y) });
            minimapTexture.draw(pixel);
        }
    }

    minimapTexture.display();
    minimapSprite.emplace(minimapTexture.getTexture());
    minimapSprite->setScale(sf::Vector2f{ MINIMAP_SCALE, MINIMAP_SCALE });
    minimapSprite->setPosition(sf::Vector2f{ 10.f, 10.f });
}

void UI::markMinimapDirty() {
    minimapDirty = true;
}

void UI::drawPlayerHealth(sf::RenderWindow& window, const Player& player) {
    float barWidth = 200.f;
    float barHeight = 20.f;
    float healthPercent = player.getHealthPercent();

    sf::RectangleShape bg({ barWidth, barHeight });
    bg.setFillColor(sf::Color(50, 0, 0));
    bg.setPosition(sf::Vector2f{ 10.f, window.getSize().y - 30.f });

    sf::RectangleShape fill({ barWidth * healthPercent, barHeight });
    fill.setFillColor(sf::Color::Red);
    fill.setPosition(sf::Vector2f{ 10.f, window.getSize().y - 30.f });

    window.draw(bg);
    window.draw(fill);
}

void UI::drawDeathScreen(sf::RenderWindow& window, const sf::Font& font)
{
    window.setView(window.getDefaultView());

    sf::Text deathText(font, "YOU DIED", 64);
    sf::FloatRect bounds = deathText.getGlobalBounds();
    deathText.setFillColor(sf::Color::Red);
    deathText.setStyle(sf::Text::Bold);
    deathText.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) / 2.f,
        (window.getSize().y - bounds.size.y) / 2.f - 30.f
        });
    sf::Text restartText(font, "press 'R' for restart", 32);
    restartText.setFillColor(sf::Color::Red);
    restartText.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) / 2.f,
        (window.getSize().y - bounds.size.y) / 2.f + 40.f
        });

    window.draw(deathText);
    window.draw(restartText);
}


