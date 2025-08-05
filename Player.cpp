#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>

Player::Player(const Dungeon& dungeon) : dungeonRef(dungeon) {
    playerShape.setSize({ TILE_SIZE - 2.f, TILE_SIZE - 2.f });
    playerShape.setFillColor(sf::Color::Green);
}

void Player::handleInput() {
    sf::Vector2f movement{ 0.f, 0.f };

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += speed;

    sf::FloatRect nextBounds = playerShape.getGlobalBounds();
    nextBounds.position.x += movement.x;
    nextBounds.position.y += movement.y;

    if (canMoveTo(nextBounds)) {
        playerShape.move(movement);
    }
}

bool Player::canMoveTo(const sf::FloatRect& bounds) const {
    // Check all four corners of the player's bounding box
    std::array<sf::Vector2f, 4> corners = {
        sf::Vector2f{bounds.position.x, bounds.position.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y},
        sf::Vector2f{bounds.position.x, bounds.position.y + bounds.size.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y}
    };

    const MapArray& map = dungeonRef.getMap();
    for (const auto& corner : corners) {
        int tileX = static_cast<int>(corner.x / TILE_SIZE);
        int tileY = static_cast<int>(corner.y / TILE_SIZE);

        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT)
            return false;

        if (map[tileY][tileX] == 1)
            return false;
    }
    return true;
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(playerShape);
}
