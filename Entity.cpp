#include "Entity.hpp"

Entity::Entity() {
    shape.setSize({ TILE_SIZE - 4.f, TILE_SIZE - 4.f }); // Default size
    shape.setFillColor(sf::Color::White);                // Default color
}

void Entity::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

sf::FloatRect Entity::getBounds() const {
    return shape.getGlobalBounds();
}

sf::Vector2f Entity::getPosition() const {
    return shape.getPosition();
}

bool Entity::canMoveTo(const sf::FloatRect& bounds, const MapArray& map, const std::vector<Entity*>& blockers) const {
    // Wall collision
    for (const auto& corner : {
        sf::Vector2f{bounds.position.x, bounds.position.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y},
        sf::Vector2f{bounds.position.x, bounds.position.y + bounds.size.y},
        sf::Vector2f{bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y}
        }) {
        int tileX = static_cast<int>(corner.x / TILE_SIZE);
        int tileY = static_cast<int>(corner.y / TILE_SIZE);
        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT || map[tileY][tileX] == 1)
            return false;
    }

    // Entity collision
    for (const Entity* other : blockers) {
        if (other == this) continue;
        if (bounds.findIntersection(other->getBounds()).has_value()) {
            return false;
        }
    }

    return true;
}


void Entity::setPosition(const sf::Vector2f& pos) {
    shape.setPosition(pos);
}

bool Entity::overlapsWith(const Entity& other) const {
    return shape.getGlobalBounds().findIntersection(other.getBounds()).has_value();
}

sf::FloatRect Entity::nextPositionWithMove(sf::Vector2f movement) const {
    sf::FloatRect next = shape.getGlobalBounds();
    next.position += movement;
    return next;
}



