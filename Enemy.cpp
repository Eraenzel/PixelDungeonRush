#include "Enemy.hpp"
#include <random>

Enemy::Enemy(const sf::Vector2f& position, const Dungeon& dungeon) : dungeonRef(&dungeon) {
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(position);
    speed = 100.f;
}

void Enemy::update(const sf::Vector2f& playerPos, const std::vector<Entity*>& blockers, float dt) {
    
    if (!dungeonRef) return;
    if (attackState == AttackState::WindingUp)
    {
		shape.setFillColor(sf::Color(255, 120, 120));
        return;
    }
    else shape.setFillColor(sf::Color::Red);

    if (isBoss && attackState == AttackState::Idle) {
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Magenta);
    }

    
    sf::Vector2f direction = playerPos - shape.getPosition();
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance > 300.f || distance <= 0.f || !hasLineOfSightTo(playerPos)) return;  // Ignore if too far

        direction /= distance; // Normalize
        sf::Vector2f movement = direction * speed * dt;

        sf::FloatRect nextBounds = shape.getGlobalBounds();        
        nextBounds.position += movement;

        const MapArray& map = dungeonRef->getMap();
        if (canMoveTo(nextBounds, map, blockers)) {
            shape.move(movement);
        }
}

bool Enemy::hasLineOfSightTo(const sf::Vector2f& target) const {
    if (!dungeonRef) return false;

    const MapArray& map = dungeonRef->getMap();

    sf::Vector2i fromTile = sf::Vector2i(shape.getPosition() / TILE_SIZE);
    sf::Vector2i toTile = sf::Vector2i(target / TILE_SIZE);

    // Bresenham-style line check
    int dx = std::abs(toTile.x - fromTile.x);
    int dy = std::abs(toTile.y - fromTile.y);
    int sx = (fromTile.x < toTile.x) ? 1 : -1;
    int sy = (fromTile.y < toTile.y) ? 1 : -1;
    int err = dx - dy;

    int x = fromTile.x;
    int y = fromTile.y;

    while (x != toTile.x || y != toTile.y) {
        if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
        if (map[y][x] == 1) return false; // hit wall

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }

    return true; // no wall hit
}

bool Enemy::canAttack() const {
    return attackState == AttackState::Idle && 
        attackCooldown.getElapsedTime() >= AttackCooldownTime;
}

void Enemy::resetAttackCooldown() {
	attackCooldown.restart();
}

bool Enemy::isWindingUp() const {
    return attackState == AttackState::WindingUp;
}

void Enemy::startWindup() {
    attackState = AttackState::WindingUp;
    windupTimer.restart();
}

void Enemy::cancelWindup() {
    if (attackState == AttackState::WindingUp)
        attackState = AttackState::Idle;
}

void Enemy::finishAttack() {
    attackState = AttackState::Cooldown;
    attackCooldown.restart();
}

void Enemy::updateCooldown() {
    if (attackState == AttackState::Cooldown &&
        attackCooldown.getElapsedTime() >= AttackCooldownTime) {
        attackState = AttackState::Idle;
    }
}

void Enemy::makeBoss()
{
    isBoss = true;
    maxHealth = 300.f;
    currentHealth = maxHealth;
    speed *= 0.6f;
    shape.setSize(shape.getSize() * 1.5f);
    shape.setFillColor(sf::Color(180, 80, 80));
}






