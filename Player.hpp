#pragma once
#include "Dungeon.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"

struct TimedBoost {
    float value;
    float remaining;
};

class Player : public Entity{
public:
    Player(const Dungeon& dungeon);

    void handleInput(const std::vector<Entity*>& blockers, float dt);
    void avoidEnemies(const std::vector<Enemy>& enemies);  

    void setSpeed(float s) { speed = s; }
    float getSpeed() const { return speed; }

	void updateBoosts(float dt);
    std::optional<TimedBoost> damageBoost;
    std::optional<TimedBoost> speedBoost;

private:
    const Dungeon& dungeonRef; 
};
