#include "Game.hpp"
#include "Entity.hpp"
#include "Constants.hpp"
#include "Config.hpp"
#include <iostream>
#include <fstream>

Game::Game()
	: window(sf::VideoMode({
		static_cast<unsigned int>(Config::instance().getInt("graphics.windowWidth", 1280)),
		static_cast<unsigned int>(Config::instance().getInt("graphics.windowHeight", 720))
	  }), "Pixel Dungeon Rush"),
	rng(std::random_device{}()),
	dungeon(),
	player(dungeon),
	ui(dungeon),
	loot(rng),
	combat(player, enemies, pickups, loot, rng, font, fontLoaded),
	floors(dungeon, player, enemies, pickups, ui, rng)
{
	if (!Config::instance().loadFromFile("assets/config.json")) {
		std::cerr << "Warning: Using default configuration values\n";
	}

	ui.regenerateMinimap();
	window.setFramerateLimit(Config::instance().getInt("graphics.targetFPS", 60));

	camera.setSize(sf::Vector2f{
		static_cast<float>(window.getSize().x),
		static_cast<float>(window.getSize().y) });
	camera.zoom(Config::instance().getFloat("graphics.zoomLevel", 0.4f));

	std::string fontPath = Config::instance().getString("graphics.fontPath", "assets/Kenney Future.ttf");
	fontLoaded = font.openFromFile(fontPath);
	if (!fontLoaded) {
		std::cerr << "Warning: Failed to load font from '" << fontPath << "'\n"
				  << "UI text will not render properly. Please ensure the font file exists.\n";
	}

	if (auto dropsArray = Config::instance().getArray("gameplay.loot.drops")) {
		std::vector<DropEntry> table;
		for (const auto& drop : dropsArray.value()) {
			std::string typeStr = drop["type"];
			Pickup::Type type;
			if (typeStr == "Heal") type = Pickup::Type::Heal;
			else if (typeStr == "DamageBoost") type = Pickup::Type::DamageBoost;
			else if (typeStr == "SpeedBoost") type = Pickup::Type::SpeedBoost;
			else continue;

			table.push_back({ type, drop["chance"].get<float>(), drop["value"].get<float>(), drop["duration"].get<float>() });
		}
		combat.setDropTable(std::move(table));
	}
	else {
		combat.setDropTable({
			{ Pickup::Type::Heal,        60.f, 20.f, 0.f },
			{ Pickup::Type::DamageBoost, 25.f, 0.3f, 6.f },
			{ Pickup::Type::SpeedBoost,  15.f, 0.25f, 6.f }
		});
	}

	restartGame();
}

void Game::run() {
	while (window.isOpen()) {
		processEvents();
		update();
		render();
	}
}

void Game::restartGame() {
	player.setHealth(100.f);
	state = GameState::Playing;
	runEnded = false;
	combat.clear();
	floors.reset();
	floors.startFloor();
}

void Game::processEvents() {
	while (true) {
		event = window.pollEvent();
		if (!event)
			break;

		if (event->is<sf::Event::Closed>())
			window.close();

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
					if (state == GameState::Playing && combat.canAttack()) {
						auto result = combat.handlePlayerAttack();
						floors.onEnemiesKilled(result.enemiesKilled, result.bossKilled);
					}
					break;

				case sf::Keyboard::Key::T:
					if (state == GameState::Playing && floors.getEnemiesKilledThisFloor() >= floors.getEnemiesToClear())
						floors.advanceFloor();
					break;

				default:
					break;
			}
		}
	}
}

void Game::update() {
	if (state == GameState::Dead) return;

	float dt = frameClock.restart().asSeconds();

	std::vector<Entity*> blockers;
	for (auto& e : enemies) blockers.push_back(&e);
	blockers.push_back(&player);

	player.handleInput(blockers, dt);
	player.avoidEnemies(enemies);

	sf::Vector2f pos = player.getPosition();
	int tileX = std::clamp(static_cast<int>(pos.x / TILE_SIZE), 0, MAP_WIDTH - 1);
	int tileY = std::clamp(static_cast<int>(pos.y / TILE_SIZE), 0, MAP_HEIGHT - 1);
	dungeon.markVisible(tileX, tileY, Constants::Gameplay::VisionRadiusTiles);
	ui.markMinimapDirty();

	combat.handleEnemyAttacks(blockers, dt, floors.getFloorNumber());
	combat.updatePickups();
	combat.updateDamageNumbers(dt);

	if (floors.isBossSpawned()) {
		for (const auto& enemy : enemies) {
			if (enemy.isBoss()) {
				ui.setBossMarker(enemy.getCenter());
				break;
			}
		}
	}

	if (fontLoaded && player.getHealth() <= 0 && !runEnded) {
		runEnded = true;
		endRun();
	}

	if (player.isDead()) {
		state = GameState::Dead;
		return;
	}

	camera.setCenter(player.getPosition());
	window.setView(camera);

	handleInputDebug(dt);

	player.updateBoosts(dt);

	if (player.speedBoost)
		player.setSpeed(140.f + player.speedBoost->value);
	else
		player.setSpeed(140.f);
}

void Game::render() {
	window.clear(sf::Color::Black);
	dungeon.draw(window);
	player.draw(window);

	for (const auto& enemy : enemies) {
		sf::Vector2f pos = enemy.getPosition();
		int tileX = static_cast<int>(pos.x / TILE_SIZE);
		int tileY = static_cast<int>(pos.y / TILE_SIZE);
		if (dungeon.isTileCurrentlyVisible(tileX, tileY))
			enemy.draw(window);
	}

	for (const auto& pickup : pickups) {
		window.draw(pickup.shape);
	}

	combat.render(window);

	window.setView(window.getDefaultView());
	ui.draw(window, player);

	if (state == GameState::Dead) {
		sf::RectangleShape overlay;
		overlay.setSize(sf::Vector2f(window.getSize()));
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		window.draw(overlay);
	}

	if (state == GameState::Dead && fontLoaded && player.getHealth() <= 0)
		ui.drawDeathScreen(window, font);

	if (fontLoaded && state == GameState::Dead)
		ui.clearBossMarker();

	if (state == GameState::Playing && floors.getEnemiesKilledThisFloor() >= floors.getEnemiesToClear())
		ui.drawAdvanceFloor(window, font);

	ui.drawFloorCounter(window, floors.getFloorNumber(), font);
	ui.drawEnemyCounter(window, floors.getEnemiesToClearThisFloor(), floors.getEnemiesDefeated(), font);

	window.display();
}

void Game::handleInputDebug(float dt) {
	float zoomSpeed = 1.5f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.zoom(1.f - zoomSpeed * dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.zoom(1.f + zoomSpeed * dt);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y)) player.setSpeed(player.getSpeed() + 2.f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) player.setSpeed(std::max(1.0f, player.getSpeed() - 2.f));
}

void Game::endRun() {
	state = GameState::Dead;
	combat.clear();
	pickups.clear();
	ui.clearBossMarker();
	saveRunStats();
}

void Game::saveRunStats() {
	std::ofstream file("runs.txt", std::ios::app);
	if (!file.is_open())
		return;
	file << "Floor: " << floors.getFloorNumber()
		 << " | Enemies: " << floors.getEnemiesDefeated()
		 << "\n";
}
