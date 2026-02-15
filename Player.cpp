#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

Player::Player(const Dungeon& dungeon) : dungeonRef(dungeon) {
    shape.setSize({TILE_SIZE - 2.f, TILE_SIZE - 2.f});
    shape.setFillColor(sf::Color::Green);
    speed = 140.f;
}

void Player::handleInput(const std::vector<Entity*>& blockers, float dt) {
    sf::Vector2f movement{ 0.f, 0.f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movement.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movement.y += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movement.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movement.x += speed;

	movement *= dt;

    if (movement.x == 0.f && movement.y == 0.f)
        return;

    const MapArray& map = dungeonRef.getMap();

    // Attempt full movement first
    sf::FloatRect currBounds = shape.getGlobalBounds();
    sf::FloatRect fullNext = currBounds;
    fullNext.position += movement;

    if (canMoveTo(fullNext, map, blockers)) {
        shape.move(movement);
        return;
    }

    // If blocked, try sliding: attempt larger axis first for more natural feel
    if (std::abs(movement.x) >= std::abs(movement.y)) {
        // try X
        sf::FloatRect nextX = currBounds;
        nextX.position.x += movement.x;
        if (canMoveTo(nextX, map, blockers)) {
            shape.move(sf::Vector2f{ movement.x, 0.f });
            return;
        }
        // try Y
        sf::FloatRect nextY = currBounds;
        nextY.position.y += movement.y;
        if (canMoveTo(nextY, map, blockers)) {
            shape.move(sf::Vector2f{ 0.f, movement.y });
            return;
        }
    }
    else {
        // try Y first
        sf::FloatRect nextY = currBounds;
        nextY.position.y += movement.y;
        if (canMoveTo(nextY, map, blockers)) {
            shape.move(sf::Vector2f{ 0.f, movement.y });
            return;
        }
        // try X
        sf::FloatRect nextX = currBounds;
        nextX.position.x += movement.x;
        if (canMoveTo(nextX, map, blockers)) {
            shape.move(sf::Vector2f{ movement.x, 0.f });
            return;
        }
    }

    // If all fail, don't move (still prevents tunneling because movement is clamped per frame)

}

void Player::avoidEnemies(const std::vector<Enemy>& enemies) {
    sf::FloatRect playerBounds = shape.getGlobalBounds();
    std::vector<Entity*>blockers;
    for (const auto& e : enemies)
        blockers.push_back(const_cast<Enemy*>(&e));

    for (const auto& enemy : enemies) {
        if (overlapsWith(enemy)) {
            sf::Vector2f away = getPosition() - enemy.getPosition();
            float lenSq = away.x * away.x + away.y * away.y;
            if (lenSq > 0.01f) {
                away /= std::sqrt(lenSq);
                sf::Vector2f push = away * 3.f;
                if (canMoveTo(nextPositionWithMove(push), dungeonRef.getMap(), blockers)) {
                    shape.move(push);
                }
            }
        }
    }
}
void Player::updateBoosts(float dt) {
    if (damageBoost) {
        damageBoost->remaining -= dt;
        if (damageBoost->remaining <= 0.f)
            damageBoost.reset();
    }

    if (speedBoost) {
        speedBoost->remaining -= dt;
        if (speedBoost->remaining <= 0.f)
            speedBoost.reset();
    }
}




