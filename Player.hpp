#pragma once
#include <SFML/Graphics.hpp>
#include "Dungeon.hpp"

class Player {
public:
    Player(const Dungeon& dungeon);

    void handleInput();
    void draw(sf::RenderWindow& window) const;
    void setPosition(const sf::Vector2f& pos) { playerShape.setPosition(pos); }
    sf::Vector2f getPosition() const { return playerShape.getPosition(); }
    const sf::FloatRect getBounds() const { return playerShape.getGlobalBounds(); }

private:
    const Dungeon& dungeonRef; // to check collisions against the map
    sf::RectangleShape playerShape;
    float speed = 2.5f;

    bool canMoveTo(const sf::FloatRect& bounds) const;
};
