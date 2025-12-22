#pragma once
#include "Entity.hpp"
#include "Dungeon.hpp"

class Enemy : public Entity {
public:
    Enemy(const sf::Vector2f& position, const Dungeon& dungeon);

    void update(const sf::Vector2f& playerPos, const std::vector<Entity*>& blockers, float dt);
    bool hasLineOfSightTo(const sf::Vector2f& target) const;

private:
    const Dungeon* dungeonRef = nullptr;
};
