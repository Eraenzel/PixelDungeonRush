# Refactoring Recommendations for Pixel Dungeon Rush

## Current Architecture Issues

### Problem 1: God Class Syndrome
The `Game` class handles too many responsibilities:

**Current responsibilities:**
- Window management
- Game loop control
- Player input handling
- Enemy AI and attacks
- Collision detection
- Pickup management
- Damage calculation
- Loot system
- UI updates
- Rendering
- Game state management

**Solution: Extract Game Manager Pattern**

```cpp
// Proposed structure:
class Game {
    void run();
private:
    sf::RenderWindow window;
    GameLogic logic;
    GameRenderer renderer;
    InputHandler input;
};

class GameLogic {
    void update(float dt);
private:
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;
    Dungeon dungeon;
    LootSystem loot;
};

class GameRenderer {
    void render(sf::RenderWindow& window, const GameLogic& logic);
private:
    sf::Font font;
    UI ui;
    // All drawing logic
};
```

---

## Performance Optimizations

### Issue 1: Line-of-Sight Every Frame
**Current:** `Enemy::hasLineOfSightTo()` called every update for each enemy
**Impact:** O(n*m) per frame where n=enemies, m=map tiles checked

**Solution: Cache LOS Results**
```cpp
class Enemy {
    struct LOSCache {
        sf::Vector2f lastPlayerPos;
        bool canSee = false;
        sf::Clock updateTime;
    } losCache;

    bool hasLineOfSightTo(const sf::Vector2f& target) {
        // Only recalculate if player moved significantly or time expired
        if ((target - losCache.lastPlayerPos).length() > TILE_SIZE || 
            losCache.updateTime.getElapsedTime() > sf::milliseconds(250)) {
            losCache.canSee = computeLOS(target);
            losCache.lastPlayerPos = target;
            losCache.updateTime.restart();
        }
        return losCache.canSee;
    }
};
```

**Expected improvement:** 30-50% reduction in frame time with many enemies

---

### Issue 2: Collision Detection is O(n²)
**Current:** Every object checks against every other object

**Solution: Quadtree Spatial Partitioning**
```cpp
class Quadtree {
    static constexpr int MAX_OBJECTS = 8;
    std::array<std::unique_ptr<Quadtree>, 4> children;
    std::vector<Entity*> objects;

    void insert(Entity* entity);
    std::vector<Entity*> retrieve(const sf::FloatRect& area);
};

class GameLogic {
    Quadtree spatialIndex;

    void update(float dt) {
        spatialIndex.clear();

        // Insert all entities
        for (auto& enemy : enemies) spatialIndex.insert(&enemy);
        spatialIndex.insert(&player);

        // Check collisions only for nearby objects
        for (auto& enemy : enemies) {
            auto nearby = spatialIndex.retrieve(enemy.getBounds());
            for (auto* entity : nearby) {
                checkCollision(enemy, *entity);
            }
        }
    }
};
```

**Expected improvement:** O(n²) → O(n log n)

---

### Issue 3: Object Pooling
**Current:** Creating/destroying enemies and pickups every frame

**Solution: Object Pools**
```cpp
template<typename T>
class ObjectPool {
    std::vector<T> available;
    std::vector<T> active;

    T& acquire() {
        if (available.empty()) {
            available.emplace_back();
        }
        active.push_back(available.back());
        available.pop_back();
        return active.back();
    }

    void release(T& object) {
        // Find and move back to available
        auto it = std::find(active.begin(), active.end(), object);
        if (it != active.end()) {
            available.push_back(*it);
            active.erase(it);
        }
    }
};

class GameLogic {
    ObjectPool<Enemy> enemyPool;
    ObjectPool<Pickup> pickupPool;
    ObjectPool<DamageNumber> damageNumberPool;
};
```

**Benefits:**
- Reduced memory fragmentation
- Predictable memory usage
- Faster allocation/deallocation

---

## Code Quality Improvements

### Issue 1: Missing const Correctness
**Current:**
```cpp
void Game::update();  // Should be clearer what's being modified
```

**Solution:**
```cpp
void GameLogic::update(float dt);  // Modifies internal state
const GameLogic& getLogic() const; // Renderer accesses read-only
```

### Issue 2: Magic String Paths
**Current:**
```cpp
fontLoaded = font.openFromFile("assets/Kenney Future.ttf");
```

**Solution:**
```cpp
namespace AssetPaths {
    inline constexpr std::string_view FONT_MAIN = "assets/Kenney Future.ttf";
    // Other asset paths
}

fontLoaded = font.openFromFile(std::string(AssetPaths::FONT_MAIN));
```

### Issue 3: Hardcoded Window Size
**Current:**
```cpp
window(sf::VideoMode({ 1280, 720 }), "Pixel Dungeon Rush")
```

**Solution:**
```cpp
// In Constants.hpp
namespace Window {
    inline constexpr int WIDTH = 1280;
    inline constexpr int HEIGHT = 720;
    inline constexpr const char* TITLE = "Pixel Dungeon Rush";
}
```

---

## Testing Strategy

### Unit Test Examples
```cpp
// Tests for enemy AI
TEST(EnemyAI, ShouldMoveTowardPlayerWhenInRange) {
    Dungeon dungeon;
    Enemy enemy({0, 0}, dungeon);
    sf::Vector2f playerPos{100, 0};

    enemy.update(playerPos, {}, 0.016f);

    EXPECT_GT(enemy.getPosition().x, 0);
}

// Tests for damage calculation
TEST(Combat, DamageRollWithinRange) {
    float damage = rollDamage(35.f, 45.f);
    EXPECT_GE(damage, 35.f);
    EXPECT_LE(damage, 45.f);
}

// Tests for pickup system
TEST(Pickup, PlayerCollectsPickupInRange) {
    Player player(dungeon);
    Pickup pickup({0, 0}, Pickup::Type::Heal, 20.f);

    player.setPosition({5, 0});
    applyPickup(player, pickup);

    EXPECT_GT(player.getHealth(), 0);
}
```

---

## Configuration System Example

```json
// config.json
{
  "gameplay": {
    "attackRadius": 40.0,
    "attackCooldownMs": 500,
    "enemyVisionRange": 300.0,
    "bossSpawnThreshold": 7,
    "difficultyScaling": 2.0
  },
  "graphics": {
    "windowWidth": 1280,
    "windowHeight": 720,
    "targetFPS": 60,
    "zoomLevel": 0.4
  },
  "audio": {
    "enabled": true,
    "masterVolume": 0.8
  }
}
```

**Usage:**
```cpp
class ConfigManager {
    static ConfigManager& instance();
    int getInt(const std::string& key, int default);
    float getFloat(const std::string& key, float default);
    bool loadFromFile(const std::string& path);
};
```

---

## Priority Roadmap

### Phase 1 (High Priority) - Week 1
- [ ] Extract `GameLogic` from `Game` class
- [ ] Add unit test project
- [ ] Implement `ConfigManager` for constants

### Phase 2 (Medium Priority) - Week 2
- [ ] Implement quadtree spatial partitioning
- [ ] Add LOS caching for enemies
- [ ] Add object pooling system

### Phase 3 (Low Priority) - Week 3
- [ ] Improve logging system
- [ ] Add sound/music system
- [ ] Implement save/load functionality

### Phase 4 (Nice to Have) - Ongoing
- [ ] Cross-platform build (CMake)
- [ ] Shader system for visual effects
- [ ] Particle system for effects
- [ ] Networking for potential multiplayer

---

## Code Style Guidelines

Based on analysis, establish these standards:

```cpp
// Naming conventions
class MyClass { };              // PascalCase for classes
void myFunction() { }           // camelCase for functions
int myVariable = 0;             // camelCase for variables
constexpr int MAX_ENEMIES = 100; // UPPER_SNAKE_CASE for constants

// Const correctness
const Dungeon& getDungeon() const;
void setHealth(float health);

// Spacing
if (condition) {
    // 4-space indentation
}

// Comments
// Use single-line for brief comments
int health = 100; // Current health

/* Use multi-line for detailed explanations
   that span multiple lines */
```

---

## Success Metrics

After implementing these improvements:

| Metric | Target |
|--------|--------|
| Code coverage | >60% |
| Frame time (1000 enemies) | <16ms (60 FPS) |
| Lines of code in Game class | <500 (from ~628) |
| Test count | >30 tests |
| Build time | <30 seconds |
| Cyclomatic complexity | <5 per method |

---

## Resources for Implementation

1. **Design Patterns**: Game Programming Patterns by Robert Nystrom
2. **Performance**: Game Engine Architecture by Jason Gregory
3. **Testing**: Google Test (gtest) framework
4. **JSON Config**: nlohmann/json library
5. **Build**: CMake 3.16+

