#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
class Dungeon;

class UI {
public:
    UI(const Dungeon& dungeon);

    void draw(sf::RenderWindow& window, const Player& player);
    void regenerateMinimap(); // call only when dungeon changes 
    void markMinimapDirty();  
    void drawPlayerHealth(sf::RenderWindow& window, const Player& player);
	void drawDeathScreen(sf::RenderWindow& window, const sf::Font& font);
	void drawWinScreen(sf::RenderWindow& window, const sf::Font& font);
    void setBossMarker(const sf::Vector2f& worldPos);
    void clearBossMarker();
    void drawFloorCounter(sf::RenderWindow& window, int floor, const sf::Font& font);
	void drawEnemyCounter(sf::RenderWindow& window, int toKillThisFloor, int killedOverall, const sf::Font& font);
	void drawAdvanceFloor(sf::RenderWindow& window, const sf::Font& font);


private:
    sf::RenderTexture minimapTexture;
    std::optional<sf::Sprite> minimapSprite;
    sf::RectangleShape minimapBg;
    const Dungeon& dungeonRef;
    bool minimapDirty = true;
    std::optional<sf::Vector2f> bossMarkerWorldPos;
};
