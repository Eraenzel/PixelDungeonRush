#include "Game.hpp"
#include "Entity.hpp"
#include <random>

Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Pixel Dungeon Rush"),
    dungeon(),
    player(dungeon),
    ui(dungeon)   
{
    ui.regenerateMinimap();
    window.setFramerateLimit(60);
    restartGame();

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

void Game::spawnEnemies()
{
    std::vector<sf::Vector2f> validTiles = dungeon.getFloorTiles();

    sf::Vector2f playerPos = player.getPosition();

    std::vector<sf::Vector2f> filtered;
    for (const auto& pos : validTiles) {
        float dx = pos.x - playerPos.x;
        float dy = pos.y - playerPos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 200.f * 200.f) { // avoid spawning too close
            filtered.push_back(pos);
        }
    }

    std::shuffle(filtered.begin(), filtered.end(), std::mt19937{ std::random_device{}() });
    int desiredEnemyCount = 5;

    for (int i = 0; i < desiredEnemyCount && i < filtered.size(); ++i) {
        enemies.emplace_back(filtered[i], dungeon); // or however your constructor works
    }

}

void Game::restartGame()
{
    dungeon.generate();
    dungeon.clearDiscovery();
    player.setPosition(dungeon.findSpawnPoint());
    ui.markMinimapDirty();
    enemies.clear();
    spawnEnemies();
    player.setHealth(100.f);
}

void Game::processEvents() {
    while (event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
    // --- RESTART/EXIT ---
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) restartGame();    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) window.close();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.zoom(0.9f); // zoom in slowly
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.zoom(1.1f); // zoom out slowly
    //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y)) player.setSpeed(player.getSpeed() + 2.f);
    //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) player.setSpeed(std::max(1.0f, player.getSpeed() - 2.f));
    if (state == GameState::Dead) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            restartGame();
            state = GameState::Playing;
        }
        return;
    }
}

void Game::update() {
    // Prepare enemy pointers for collision checks
    std::vector<Entity*> blockers;
    for (auto& e : enemies) blockers.push_back(&e);
    blockers.push_back(&player); // prevent overlapping with player too

    player.handleInput(blockers);
    player.avoidEnemies(enemies); // pushing

    sf::Vector2f pos = player.getPosition();
    int tileX = static_cast<int>(pos.x / TILE_SIZE);
    int tileY = static_cast<int>(pos.y / TILE_SIZE);

    if (dungeon.markVisible(tileX, tileY, 4)) {  // returns true if anything was revealed
        ui.markMinimapDirty();
    }

    for (auto& enemy : enemies) {
        float damageRadius = 40.f;
        sf::Vector2f delta = enemy.getPosition() - player.getPosition();
        enemy.update(player.getPosition(), blockers);
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq < damageRadius * damageRadius) {
            player.takeDamage(0.5f);  // Adjust damage per frame as needed
        }
    }
    if (player.isDead()) {
        state = GameState::Dead;
        return;
    }

    handleCombat();

    camera.setCenter(player.getPosition());
    window.setView(camera);

    if (state == GameState::Dead) return; // Pause game updates

}

void Game::render() {
    window.clear(sf::Color::Black);
    dungeon.draw(window);
    player.draw(window);
    for (const auto& enemy : enemies)
        enemy.draw(window);

    // --- Draw player health bar ---
    float barWidth = 200.f;
    float barHeight = 20.f;
    float healthRatio = player.getHealthPercent();

    sf::RectangleShape background(sf::Vector2f{ barWidth, barHeight });
    background.setFillColor(sf::Color(50, 50, 50));
    background.setPosition(sf::Vector2f{ 10.f, window.getSize().y - 30.f });

    sf::RectangleShape fill(sf::Vector2f{ barWidth * healthRatio, barHeight });
    fill.setFillColor(sf::Color::Red);
    fill.setPosition(sf::Vector2f{ 10.f, window.getSize().y - 30.f });

    window.setView(window.getDefaultView());  // Switch to HUD view
    window.draw(background);
    window.draw(fill);

    if (attackEffect && attackEffectTimer.getElapsedTime().asMilliseconds() < 100) {
        window.draw(*attackEffect);
    }
    else {
        attackEffect.reset(); // clear it
    }


    window.setView(window.getDefaultView());
    ui.draw(window, player);

    //window.display();

    if (state == GameState::Dead) {
        window.setView(window.getDefaultView());

        sf::Font font("assets/Kenney Future.ttf"); // pass the path in the constructor

        sf::Text text(font, "YOU DIED", 64);
        sf::FloatRect bounds = text.getGlobalBounds();
        text.setFillColor(sf::Color::Red);
        text.setStyle(sf::Text::Bold);
        text.setPosition(sf::Vector2f{
            (window.getSize().x - bounds.size.x) / 2.f,
            (window.getSize().y - bounds.size.y) / 2.f - 30.f
            });
        sf::Text text1(font, "¨R¨ for restart", 32);
        text1.setFillColor(sf::Color::Red);
        text1.setPosition(sf::Vector2f{
            (window.getSize().x - bounds.size.x) / 2.f,
            (window.getSize().y - bounds.size.y) / 2.f + 40.f
            });

        window.draw(text);
        window.draw(text1);
       
    }
     window.display();
}

void Game::handleCombat() {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F) || !canAttack()) return;

    float attackRadius = 40.f;
    sf::Vector2f playerPos = player.getPosition();

    for (auto& enemy : enemies) {
        sf::Vector2f delta = enemy.getPosition() - player.getPosition();
        float distSq = delta.x * delta.x + delta.y * delta.y;
        if (distSq < attackRadius * attackRadius) {
            enemy.takeDamage(50.f);  // Deal damage
        }
    }

    // Remove dead enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.isDead(); }), enemies.end());

    attackCooldown.restart();
    attackEffect.emplace(attackRadius);
    attackEffect->setFillColor(sf::Color(0, 255, 0, 100));
    attackEffect->setOrigin(sf::Vector2f{ 20.f, 20.f });
    attackEffect->setPosition(player.getPosition());
    attackEffectTimer.restart();

}

bool Game::canAttack() const {
    return attackCooldown.getElapsedTime().asMilliseconds() > 500; // 0.5s cooldown
}



