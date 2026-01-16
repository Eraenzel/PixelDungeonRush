#include "Game.hpp"
#include "Entity.hpp"

Game::Game()
    : window(sf::VideoMode({ 1280, 720 }), "Pixel Dungeon Rush"),
    dungeon(),
    player(dungeon),
    ui(dungeon),
	rng(std::mt19937(std::random_device{}()))
{
    ui.regenerateMinimap();
    window.setFramerateLimit(60);
    restartGame();

    camera.setSize(sf::Vector2f{
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y) });
    camera.zoom(0.4f);

    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/Kenney Future.ttf");
        fontLoaded = true;
	}
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

    std::shuffle(filtered.begin(), filtered.end(), rng);

    for (int i = 0; i < EnemiesPerRoom && i < filtered.size(); ++i) {
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
    state = GameState::Playing;
}

void Game::processEvents() {
    while (true) {
        event = window.pollEvent();
        if (!event)
			break;

        if (event->is<sf::Event::Closed>())
            window.close();

        // --- INPUT HANDLING ---
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

            switch (key->code) {
                case sf::Keyboard::Key::Escape:
                    window.close();
					break;

                case sf::Keyboard::Key::R:
					if (state == GameState::Dead)
                        restartGame();
					break;

                case sf::Keyboard::Key::F:
                    if (state == GameState::Playing && canAttack())
						handleCombat();
					break;

                default:
					break;
            }
        }
    }
}

void Game::handleInputDebug(float dt) {

    float zoomSpeed = 1.5f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.zoom(1.f - zoomSpeed * dt); // zoom in slowly
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.zoom(1.f + zoomSpeed * dt); // zoom out slowly
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y)) player.setSpeed(player.getSpeed() + 2.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) player.setSpeed(std::max(1.0f, player.getSpeed() - 2.f));
}

void Game::update() {
	float dt = frameClock.restart().asSeconds();

    // Prepare enemy pointers for collision checks
    std::vector<Entity*> blockers;
    for (auto& e : enemies) blockers.push_back(&e);
    blockers.push_back(&player); // prevent overlapping with player too

    player.handleInput(blockers, dt);
    player.avoidEnemies(enemies); // pushing

    sf::Vector2f pos = player.getPosition();
    int tileX = static_cast<int>(pos.x / TILE_SIZE);
    int tileY = static_cast<int>(pos.y / TILE_SIZE);

    dungeon.markVisible(tileX, tileY, VisionRadiusTiles);

    (dungeon.markVisible(tileX, tileY, VisionRadiusTiles));  // returns true if anything was revealed
    ui.markMinimapDirty();


    for (auto& enemy : enemies) {
        float damageRadius = 40.f;
        enemy.update(player.getPosition(), blockers, dt);

        sf::Vector2f enemyCenter = enemy.getCenter();
        sf::Vector2f playerCenter = player.getCenter();
        sf::Vector2f delta = enemyCenter - playerCenter;
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq < damageRadius * damageRadius) {
            player.takeDamage(EnemyContactDPS * dt);  // Adjust damage per frame as needed
        }
    }
    if (player.isDead()) {
        state = GameState::Dead;
        return;
    }

    camera.setCenter(player.getPosition());
    window.setView(camera);

	handleInputDebug(dt);

    if (state == GameState::Dead) return; // Pause game updates

}

void Game::render() {
    window.clear(sf::Color::Black);
    dungeon.draw(window);
    player.draw(window);
    for (const auto& enemy : enemies) {

        sf::Vector2f pos = enemy.getPosition();

        int tileX = static_cast<int>(pos.x / TILE_SIZE);
        int tileY = static_cast<int>(pos.y / TILE_SIZE);

        if (dungeon.isTileCurrentlyVisible(tileX, tileY)) {
            enemy.draw(window);
        }
    }

    if (attackEffect && attackEffectTimer.getElapsedTime() < AttackEffectDuration) {
        window.draw(*attackEffect);
    }
    else {
        attackEffect.reset(); // clear it
    }

    window.setView(window.getDefaultView());
    ui.draw(window, player);

    if (state == GameState::Dead && fontLoaded) {
		ui.drawDeathScreen(window, font);      
    }
     window.display();
}

void Game::handleCombat() {

    for (auto& enemy : enemies) {
        sf::Vector2f delta = enemy.getPosition() - player.getPosition();
        float distSq = delta.x * delta.x + delta.y * delta.y;
        if (distSq < AttackRadius * AttackRadius) {
            enemy.takeDamage(50.f);  // Deal damage
        }
    }

    // Remove dead enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.isDead(); }), enemies.end());

    attackCooldown.restart();
    attackEffect.emplace(AttackRadius);
    attackEffect->setFillColor(sf::Color(0, 255, 0, 100));
    attackEffect->setOrigin(sf::Vector2f{ AttackRadius, AttackRadius });
    attackEffect->setPosition(player.getCenter());
    attackEffectTimer.restart();

}

bool Game::canAttack() const {
    return attackCooldown.getElapsedTime().asMilliseconds() > AttackCooldownMs; 
}



