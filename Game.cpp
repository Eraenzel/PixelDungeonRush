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

    fontLoaded = font.openFromFile("assets/Kenney Future.ttf");
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
	//spawnBoss();
    player.setHealth(100.f);
    state = GameState::Playing;
	enemiesDefeated = 0;
	bossAlive = true;
	bossSpawned = false;
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
						handlePlayerAttack();
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
    if (state == GameState::Dead) return; // Pause game updates
    
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
    ui.markMinimapDirty();

    handleEnemyAttacks(blockers, dt);

    auto pit = pickups.begin();
    while (pit != pickups.end()) {

        sf::Vector2f delta = pit->position - player.getCenter();
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq < 20.f * 20.f) { // pickup radius

            if (pit->type == Pickup::Type::Heal) {
                player.setHealth(player.getHealth() + pit->value);
            }

            pit = pickups.erase(pit);
        }
        else {
            ++pit;
        }
    }

    if (player.isDead()) {
        state = GameState::Dead;
        return;
    }

    camera.setCenter(player.getPosition());
    window.setView(camera);

	handleInputDebug(dt);
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

    for (const auto& pickup : pickups) {
        window.draw(pickup.shape);
    }

    if (attackEffect && attackEffectTimer.getElapsedTime() < AttackEffectDuration) {
        window.draw(*attackEffect);
    }
    else {
        attackEffect.reset(); // clear it
    }

    window.setView(window.getDefaultView());
    ui.draw(window, player);

    if (state == GameState::Dead && fontLoaded && player.getHealth() <= 0) {
		ui.drawDeathScreen(window, font);      
    }

    if (state == GameState::Dead && fontLoaded && !bossAlive) {
        ui.drawWinScreen(window, font);
    }
        window.display();
}

void Game::handlePlayerAttack() {

    for (auto& enemy : enemies) {

        sf::Vector2f playerCenter = player.getCenter();
        sf::FloatRect enemyBounds = enemy.getBounds();

        float left = enemyBounds.position.x;
        float right = enemyBounds.position.x + enemyBounds.size.x;
        float top = enemyBounds.position.y;
        float bottom = enemyBounds.position.y + enemyBounds.size.y;

        float closestX = std::clamp(playerCenter.x, left, right);
        float closestY = std::clamp(playerCenter.y, top, bottom);

        float dx = playerCenter.x - closestX;
        float dy = playerCenter.y - closestY;
        
        float distSq = dx * dx + dy * dy;

        if (distSq <= AttackRadius * AttackRadius) {
            enemy.takeDamage(40.f);  // Deal damage
        }
    }

    bool bossKilledThisFrame = false;

    // Remove dead enemies
    auto it = enemies.begin();
    while (it != enemies.end()) {
        if (it->isDead()) {

            if (it->isBoss) {
                bossKilledThisFrame = true;
			}
            // Spawn heal pickup at enemy center
            pickups.emplace_back(
                it->getCenter(),
                Pickup::Type::Heal,
                20.f
            );

            it = enemies.erase(it);
            enemiesDefeated++;
        }
        else {
            ++it;
        }
    }

    if (!bossSpawned && enemiesDefeated >= BossSpawnThreshold) {
        spawnBoss();
        bossSpawned = true;
        bossAlive = true;
    }

    if (bossKilledThisFrame) {
        endRun();
        return;
    }

    attackCooldown.restart();
    attackEffect.emplace(AttackRadius);
    attackEffect->setFillColor(sf::Color(0, 255, 0, 100));
    attackEffect->setOrigin(sf::Vector2f{ AttackRadius, AttackRadius });
    attackEffect->setPosition(player.getCenter());
    attackEffectTimer.restart();

}

void Game::handleEnemyAttacks(std::vector<Entity*>& blockers, float dt)
{
    for (auto& enemy : enemies) {
        enemy.update(player.getPosition(), blockers, dt);
        enemy.updateCooldown();

        sf::FloatRect playerBounds = player.getBounds();
        sf::Vector2f enemyCenter = enemy.getCenter();


        float left = playerBounds.position.x;
        float right = playerBounds.position.x + playerBounds.size.x;
        float top = playerBounds.position.y;
        float bottom = playerBounds.position.y + playerBounds.size.y;

        float closestX = std::clamp(enemyCenter.x, left, right);
        float closestY = std::clamp(enemyCenter.y, top, bottom);

        float dx = enemyCenter.x - closestX;
        float dy = enemyCenter.y - closestY;

        //sf::Vector2f delta = enemy.getCenter() - player.getCenter();;
        float distSq = dx * dx + dy * dy;
        float rangeSq = Enemy::AttackRange * Enemy::AttackRange;

        if (distSq <= rangeSq)
        {
            if (enemy.canAttack())
            {
                enemy.startWindup();
            }

            if (enemy.isWindingUp() && enemy.windupTimer.getElapsedTime() >= Enemy::AttackWindupTime)
            {
                //player.takeDamage(EnemyContactDPS * dt);  
                player.takeDamage(Enemy::AttackDamage);

                attackEffect.emplace(Enemy::AttackRange);
                attackEffect->setOrigin(sf::Vector2f{ AttackRadius, AttackRadius });
                attackEffect->setPosition(enemy.getCenter());
                attackEffect->setFillColor(sf::Color(255, 80, 80, 120));
                attackEffectTimer.restart();

                enemy.finishAttack();
            }
        }
        else
        {
            enemy.cancelWindup();
        }
    }
}

bool Game::canAttack() const {
    return attackCooldown.getElapsedTime().asMilliseconds() > AttackCooldownMs; 
}

void Game::spawnBoss()
{
    /*const auto& rooms = dungeon.getRooms();
    if (rooms.empty()) return;

    const Room& bossRoom = rooms.back();

    sf::Vector2f bossPos(
        bossRoom.centerX() * TILE_SIZE,
        bossRoom.centerY() * TILE_SIZE
    );*/

    std::vector<sf::Vector2f> floorTiles = dungeon.getFloorTiles();
    if (floorTiles.empty())
        return;

    sf::Vector2f playerPos = player.getPosition();

    std::vector<sf::Vector2f> candidates;
    for (const auto& tilePos : floorTiles) {
        sf::Vector2f delta = tilePos - playerPos;
        float distSq = delta.x * delta.x + delta.y * delta.y;

        if (distSq >= BossMinSpawnDist * BossMinSpawnDist &&
            distSq <= BossMaxSpawnDist * BossMaxSpawnDist) {
            candidates.push_back(tilePos);
        }
    }

    // Fallback: if no tile fits, relax the constraint
    if (candidates.empty()) {
        candidates = floorTiles;
    }

    std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
    sf::Vector2f bossPos = candidates[dist(rng)] + sf::Vector2f{ TILE_SIZE * 0.5f, TILE_SIZE * 0.5f };

    enemies.emplace_back(bossPos, dungeon);
    enemies.back().makeBoss();
}

void Game::endRun() {
    state = GameState::Dead;
    bossAlive = false; // reuse Dead for now
}


