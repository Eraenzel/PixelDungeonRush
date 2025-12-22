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

    void setHealth(float health) { currentHealth = std::clamp(health, 0.f, maxHealth); }
    void takeDamage(float amount) { currentHealth = std::max(0.f, currentHealth - amount); damageFlashTimer.restart();
    }
    float getHealth() const { return currentHealth; }
    float getHealthPercent() const { return currentHealth / maxHealth; }
    bool isDead() const { return currentHealth <= 0.f; }
    sf::Vector2f getCenter() const;

protected:
    sf::RectangleShape shape;
    float speed = 120.f;
    float maxHealth = 100.f;
    float currentHealth = 100.f;
    sf::Clock damageFlashTimer;

};
