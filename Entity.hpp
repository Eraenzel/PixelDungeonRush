#pragma once
#include <SFML/Graphics.hpp>
#include "Dungeon.hpp"

class Entity {
public:
    Entity();
    virtual ~Entity() = default;

    virtual void update() {};
    virtual void draw(sf::RenderWindow& window) const;

    virtual sf::FloatRect getBounds() const;
    virtual sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f& pos);
    bool canMoveTo(const sf::FloatRect& bounds, const MapArray& map, const std::vector<Entity*>& blockers) const;
    bool overlapsWith(const Entity& other) const;
    sf::FloatRect nextPositionWithMove(sf::Vector2f movement) const;

protected:
    sf::RectangleShape shape;
    float speed = 2.5f;
};
