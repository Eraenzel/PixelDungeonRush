#pragma once
#include "Entity.hpp"
#include "Dungeon.hpp"

class Enemy : public Entity {
public:
    Enemy(const sf::Vector2f& position, const Dungeon& dungeon);

    void update(const sf::Vector2f& playerPos, const std::vector<Entity*>& blockers, float dt);
    bool hasLineOfSightTo(const sf::Vector2f& target) const;
    bool canAttack() const;
    void resetAttackCooldown();
    bool isWindingUp() const;
    void startWindup();
    void cancelWindup();
	void finishAttack();
	void updateCooldown();
    bool isBoss = false;
    void makeBoss();

    enum class AttackState {
        Idle,
        WindingUp,
        Cooldown
    };

	AttackState attackState = AttackState::Idle;

    static constexpr float AttackRange = 40.f;
    static constexpr float AttackDamage = 15.f;
    sf::Clock attackCooldown;
    sf::Clock windupTimer;
    static constexpr sf::Time AttackWindupTime = sf::milliseconds(350);
    static constexpr sf::Time AttackCooldownTime = sf::milliseconds(1200);

private:
    const Dungeon* dungeonRef = nullptr;

};
