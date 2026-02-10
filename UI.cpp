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

    if (bossMarkerWorldPos.has_value() && minimapSprite.has_value()) {

        sf::CircleShape bossMarker(4.f);
        bossMarker.setFillColor(sf::Color::Red);
        bossMarker.setOrigin({ 4.f, 4.f });

        sf::Vector2f minimapPos = minimapSprite->getPosition();

        sf::Vector2f bossTilePos = {
            bossMarkerWorldPos->x / TILE_SIZE,
            bossMarkerWorldPos->y / TILE_SIZE
        };

        bossMarker.setPosition({
            minimapPos.x + bossTilePos.x * MINIMAP_SCALE,
            minimapPos.y + bossTilePos.y * MINIMAP_SCALE
            });

        window.draw(bossMarker);
    }
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

void UI::drawWinScreen(sf::RenderWindow& window, const sf::Font& font)
{
    window.setView(window.getDefaultView());

    sf::Text winText(font, "YOU WIN", 64);
    sf::FloatRect bounds = winText.getGlobalBounds();
    winText.setFillColor(sf::Color::Cyan);
    winText.setStyle(sf::Text::Bold);
    winText.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) / 2.f,
        (window.getSize().y - bounds.size.y) / 2.f - 30.f
        });
    sf::Text restartText(font, "press 'R' for restart", 32);
    restartText.setFillColor(sf::Color::Cyan);
    restartText.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) / 2.f,
        (window.getSize().y - bounds.size.y) / 2.f + 40.f
        });

    window.draw(winText);
    window.draw(restartText);
}

void UI::setBossMarker(const sf::Vector2f& worldPos) {
    bossMarkerWorldPos = worldPos;
}

void UI::clearBossMarker() {
    bossMarkerWorldPos.reset();
}

void UI::drawFloorCounter(sf::RenderWindow& window, int floor, const sf::Font& font) {
    sf::Text text(font, "Floor " + std::to_string(floor), 18);
    sf::FloatRect bounds = text.getGlobalBounds();
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) * 0.5f,
        8.f });

    window.draw(text);
}

void UI::drawEnemyCounter(sf::RenderWindow& window, int toKillThisFloor, int killedOverall, const sf::Font& font)
{
    sf::Text text(font, "Enemies to kill: " + std::to_string(toKillThisFloor), 18);
    sf::FloatRect bounds = text.getGlobalBounds();
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) * 0.35f,
        40.f });

    sf::Text text1(font, "Enemies killed: " + std::to_string(killedOverall), 18);
    sf::FloatRect bounds1 = text1.getGlobalBounds();
    text1.setFillColor(sf::Color::White);
    text1.setPosition(sf::Vector2f{
        (window.getSize().x - bounds1.size.x) * 0.35f,
        24.f });

    window.draw(text);
    window.draw(text1);
}

void UI::drawAdvanceFloor(sf::RenderWindow& window, const sf::Font& font)
{
    sf::Text text(font, "Press T to advance to next floor!", 18);
    sf::FloatRect bounds = text.getGlobalBounds();
    text.setFillColor(sf::Color::Green);
    text.setPosition(sf::Vector2f{
        (window.getSize().x - bounds.size.x) * 0.5f,
        700.f });
    window.draw(text);
}




