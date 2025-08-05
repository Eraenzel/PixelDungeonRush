#include "Game.hpp"

Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Pixel Dungeon Rush"),
    dungeon(),
    player(dungeon),
    ui(dungeon)
{
    window.setFramerateLimit(60);

    dungeon.generate();
    ui.regenerateMinimap();
    player.setPosition(dungeon.findSpawnPoint());

    camera.setSize(sf::Vector2f{
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y) });
    camera.zoom(0.4f);
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents() {
    while (event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
    // --- RESTART/EXIT ---
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        dungeon.generate();
        player.setPosition(dungeon.findSpawnPoint());
        player.speed = 2.5f;
        ui.regenerateMinimap();
        render();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
        window.close();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.zoom(0.9f); // zoom in slowly
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.zoom(1.1f); // zoom out slowly
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y)) player.speed += 2.f; 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) player.speed = std::max(1.0f, player.speed - 2.f);

}

void Game::update() {
    player.handleInput();
    camera.setCenter(player.getPosition());
    window.setView(camera);
}

void Game::render() {
    window.clear(sf::Color::Black);
    dungeon.draw(window);
    player.draw(window);

    window.setView(window.getDefaultView());
    ui.draw(window, player);

    window.display();
}
