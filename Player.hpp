#pragma once
#include "Dungeon.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"

class Player : public Entity{
public:
    Player(const Dungeon& dungeon);

    void handleInput(const std::vector<Entity*>& blockers);
    void avoidEnemies(const std::vector<Enemy>& enemies);  

    void setSpeed(float s) { speed = s; }
    float getSpeed() const { return speed; }

private:
    const Dungeon& dungeonRef; 
};
