#include "CombatSystem.hpp"
#include "Constants.hpp"
#include <cmath>

CombatSystem::CombatSystem(Player& player, std::vector<Enemy>& enemies, std::vector<Pickup>& pickups,
                           LootSystem& loot, std::mt19937& rng, const sf::Font& font, const bool& fontLoaded)
    : player(player), enemies(enemies), pickups(pickups), loot(loot), rng(rng), font(font), fontLoaded(fontLoaded)
{
}

bool CombatSystem::canAttack() const {
    return attackCooldown.getElapsedTime().asMilliseconds() > Constants::Gameplay::AttackCooldownMs;
}

void CombatSystem::setDropTable(std::vector<DropEntry> table) {
    dropTable = std::move(table);
}

AttackResult CombatSystem::handlePlayerAttack() {
    AttackResult result;

    for (auto& enemy : enemies) {
        sf::Vector2f playerCenter = player.getCenter();
        sf::FloatRect enemyBounds = enemy.getBounds();

        float left   = enemyBounds.position.x;
        float right  = enemyBounds.position.x + enemyBounds.size.x;
        float top    = enemyBounds.position.y;
        float bottom = enemyBounds.position.y + enemyBounds.size.y;

        float closestX = std::clamp(playerCenter.x, left, right);
        float closestY = std::clamp(playerCenter.y, top, bottom);

        float dx = playerCenter.x - closestX;
        float dy = playerCenter.y - closestY;
        float distSq = dx * dx + dy * dy;

        if (distSq <= Constants::Gameplay::AttackRadius * Constants::Gameplay::AttackRadius) {
            float damage = rollDamage(35.f, 45.f);
            if (player.damageBoost)
                damage += player.damageBoost->value;
            enemy.takeDamage(damage);
            spawnDamageNumber(
                enemy.getCenter(),
                damage,
                enemy.isBoss() ? sf::Color(255, 120, 120) : sf::Color::White
            );
        }
    }

    auto it = enemies.begin();
    while (it != enemies.end()) {
        if (it->isDead()) {
            if (it->isBoss())
                result.bossKilled = true;

            auto drops = loot.rollDrops(it->rarity, it->getCenter());

            std::uniform_real_distribution<float> angleDist(0.f, 2.f * 3.1415926f);
            std::uniform_real_distribution<float> radiusDist(5.f, 18.f);

            for (auto& p : drops) {
                float angle  = angleDist(rng);
                float radius = radiusDist(rng);
                sf::Vector2f offset(std::cos(angle) * radius, std::sin(angle) * radius);
                pickups.emplace_back(it->getCenter() + offset, p.type, p.value, p.duration);
            }

            it = enemies.erase(it);
            result.enemiesKilled++;
        }
        else {
            ++it;
        }
    }

    attackCooldown.restart();
    attackEffect.emplace(Constants::Gameplay::AttackRadius);
    attackEffect->setFillColor(sf::Color(0, 255, 0, 100));
    attackEffect->setOrigin(sf::Vector2f{ Constants::Gameplay::AttackRadius, Constants::Gameplay::AttackRadius });
    attackEffect->setPosition(player.getCenter());
    attackEffectTimer.restart();

    return result;
}

void CombatSystem::handleEnemyAttacks(std::vector<Entity*>& blockers, float dt, int floorNumber) {
    for (auto& enemy : enemies) {
        enemy.update(player.getPosition(), blockers, dt);
        enemy.updateCooldown();

        sf::FloatRect playerBounds = player.getBounds();
        sf::Vector2f enemyCenter = enemy.getCenter();

        float left   = playerBounds.position.x;
        float right  = playerBounds.position.x + playerBounds.size.x;
        float top    = playerBounds.position.y;
        float bottom = playerBounds.position.y + playerBounds.size.y;

        float closestX = std::clamp(enemyCenter.x, left, right);
        float closestY = std::clamp(enemyCenter.y, top, bottom);

        float dx = enemyCenter.x - closestX;
        float dy = enemyCenter.y - closestY;
        float distSq = dx * dx + dy * dy;
        float rangeSq = Enemy::AttackRange * Enemy::AttackRange;

        if (distSq <= rangeSq) {
            if (enemy.canAttack())
                enemy.startWindup();

            if (enemy.isWindingUp() && enemy.windupTimer.getElapsedTime() >= Enemy::AttackWindupTime) {
                float damage = rollDamage(Enemy::AttackDamageMax, Enemy::AttackDamageMin);
                damage += (floorNumber - 1) * 2.f;
                player.takeDamage(damage);

                spawnDamageNumber(player.getCenter(), damage, sf::Color(255, 80, 80));

                attackEffect.emplace(Enemy::AttackRange);
                attackEffect->setOrigin(sf::Vector2f{ Enemy::AttackRange, Enemy::AttackRange });
                attackEffect->setPosition(enemy.getCenter());
                int alpha = static_cast<int>(std::clamp(damage * 10.f, 80.f, 160.f));
                attackEffect->setFillColor(sf::Color(255, 80, 80, alpha));
                attackEffectTimer.restart();

                enemy.finishAttack();
            }
        }
        else {
            enemy.cancelWindup();
        }
    }
}

void CombatSystem::updatePickups() {
    auto pit = pickups.begin();
    while (pit != pickups.end()) {
        sf::Vector2f delta = pit->position - player.getCenter();
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq < Constants::Gameplay::PickupRadius * Constants::Gameplay::PickupRadius) {
            switch (pit->type) {
            case Pickup::Type::Heal:
                player.setHealth(player.getHealth() + pit->value);
                break;
            case Pickup::Type::DamageBoost:
                player.damageBoost = { pit->value, pit->duration };
                break;
            case Pickup::Type::SpeedBoost:
                player.speedBoost = { pit->value, pit->duration };
                break;
            }
            pit = pickups.erase(pit);
        }
        else {
            ++pit;
        }
    }
}

void CombatSystem::updateDamageNumbers(float dt) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end(); ) {
        float t = it->lifetime.getElapsedTime().asSeconds();

        if (t > 0.6f) {
            it = damageNumbers.erase(it);
            continue;
        }

        it->text.move(it->velocity * dt);

        sf::Color c = it->text.getFillColor();
        c.a = static_cast<std::uint8_t>(255.f * (1.f - t / 0.6f));
        it->text.setFillColor(c);

        ++it;
    }
}

void CombatSystem::render(sf::RenderWindow& window) {
    if (attackEffect && attackEffectTimer.getElapsedTime() < Constants::Gameplay::AttackEffectDuration) {
        window.draw(*attackEffect);
    }
    else {
        attackEffect.reset();
    }

    for (const auto& dn : damageNumbers) {
        window.draw(dn.text);
    }
}

void CombatSystem::clear() {
    damageNumbers.clear();
    attackEffect.reset();
}

float CombatSystem::rollDamage(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

void CombatSystem::spawnDamageNumber(const sf::Vector2f& worldPos, float value, const sf::Color& color) {
    if (!fontLoaded) return;
    damageNumbers.emplace_back(font, worldPos, static_cast<int>(value), color);
}

void CombatSystem::spawnPickup(const sf::Vector2f& pos) {
    if (dropTable.empty()) return;

    float totalWeight = 0.f;
    for (const auto& e : dropTable)
        totalWeight += e.chance;

    std::uniform_real_distribution<float> dist(0.f, totalWeight);
    float roll = dist(rng);

    float accum = 0.f;
    for (const auto& e : dropTable) {
        accum += e.chance;
        if (roll <= accum) {
            pickups.emplace_back(pos, e.type, e.value, e.duration);
            return;
        }
    }
}
