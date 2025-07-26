#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
class Dungeon;

class UI {
public:
    UI(const Dungeon& dungeon);

    void draw(sf::RenderWindow& window, const Player& player);

private:
    sf::RenderTexture minimapTexture;
    std::optional<sf::Sprite> minimapSprite;
    sf::RectangleShape minimapBg;
    const Dungeon& dungeonRef;
};
