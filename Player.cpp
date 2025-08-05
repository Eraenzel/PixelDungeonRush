#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>

Player::Player(const Dungeon& dungeon) : dungeonRef(dungeon) {
    shape.setSize({TILE_SIZE - 2.f, TILE_SIZE - 2.f});
    shape.setFillColor(sf::Color::Green);
    speed = 5.f;
}

void Player::handleInput(const std::vector<Entity*>& blockers) {
    sf::Vector2f movement{ 0.f, 0.f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += speed;

    sf::FloatRect nextBounds = Player::shape.getGlobalBounds();
    nextBounds.position.x += movement.x;
    nextBounds.position.y += movement.y;

    if (canMoveTo(nextBounds, dungeonRef.getMap(), blockers)) {
        shape.move(movement);
    }
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

