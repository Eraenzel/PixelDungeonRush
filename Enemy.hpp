#pragma once
#include "Entity.hpp"
#include "Dungeon.hpp"
#include "Loot.hpp"

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
    //bool isBoss = false;
    void makeBoss();

    enum class AttackState {
        Idle,
        WindingUp,
        Cooldown
    };

	AttackState attackState = AttackState::Idle;

    static constexpr float AttackRange = 40.f;
    static constexpr float AttackDamageMax = 10.f;
    static constexpr float AttackDamageMin = 20.f;
    sf::Clock attackCooldown;
    sf::Clock windupTimer;
    static constexpr sf::Time AttackWindupTime = sf::milliseconds(350);
    static constexpr sf::Time AttackCooldownTime = sf::milliseconds(900);

    EnemyRarity rarity = EnemyRarity::Common;

    bool isElite() const { return rarity == EnemyRarity::Elite; }
    bool isBoss()  const { return rarity == EnemyRarity::Boss; }


private:
    const Dungeon* dungeonRef = nullptr;

};
