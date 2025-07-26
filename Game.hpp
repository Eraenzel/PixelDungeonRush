#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "Dungeon.hpp"
#include "UI.hpp"

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    sf::View camera;
    std::optional<sf::Event> event;

    Player player;
    Dungeon dungeon;
    UI ui;
    
    void processEvents();
    void update();
    void render();
};