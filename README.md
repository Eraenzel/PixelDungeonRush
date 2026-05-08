# Pixel Dungeon Rush

A fast-paced 2D roguelike dungeon crawler game built with **C++17** and **SFML 3.0**, featuring procedural dungeon generation, dynamic enemy AI, loot systems, and boss encounters.

## 🎮 Game Features

- **Top-down action gameplay** with tile-based movement and collision
- **Procedurally generated dungeons** with progressive difficulty
- **Dynamic enemy AI** with pathfinding and line-of-sight mechanics
- **Boss encounters** that spawn at key progression points
- **Loot system** with randomized item drops and rarity tiers
- **Power-ups** including health recovery, damage boost, and speed boost
- **Minimap display** showing current position and visibility
- **Progressive floors** with increasing difficulty
- **Damage feedback** with floating damage numbers

## 🛠️ Technology Stack

- **Language**: C++17 (MSVC v143)
- **Graphics**: SFML 3.0 (Simple Fast Multimedia Library)
- **Build System**: Visual Studio 2026 / MSBuild
- **Platform**: Windows (x64, x86)

## 📋 Project Structure

```
PixelDungeonRush/
├── src/                    # Source files
│   ├── Game.cpp/hpp       # Main game loop and logic
│   ├── Player.cpp/hpp     # Player entity with movement
│   ├── Enemy.cpp/hpp      # Enemy AI and pathfinding
│   ├── Dungeon.cpp/hpp    # Procedural dungeon generation
│   ├── Entity.cpp/hpp     # Base class for game objects
│   ├── UI.cpp/hpp         # User interface (minimap, HUD)
│   ├── Loot.cpp/hpp       # Loot drop system
│   ├── Assets.cpp/hpp     # Asset management
│   ├── Constants.hpp      # Game constants and settings
│   └── Main.cpp           # Entry point
├── assets/                # Game resources (fonts, textures)
├── PixelDungeonRush.sln   # Solution file
├── PixelDungeonRush.vcxproj # Project file
└── README.md              # This file
```

## 🚀 Getting Started

### Prerequisites
- Visual Studio 2019 or newer
- SFML 3.0 library
- C++17 support

### Building
1. Clone the repository
2. Update SFML path in `PixelDungeonRush.vcxproj` if needed:
   - `C:\SFML-3.0.0\include` (includes)
   - `C:\SFML-3.0.0\lib` (libraries)
3. Open `PixelDungeonRush.sln` in Visual Studio
4. Build solution (Ctrl+Shift+B)
5. Run (F5)

### Game Controls
- **W/Up Arrow** - Move up
- **S/Down Arrow** - Move down
- **A/Left Arrow** - Move left
- **D/Right Arrow** - Move right
- **Left Mouse Button** - Attack
- **Close window** - Quit

## 📚 Documentation

This project includes comprehensive documentation:

- **[IMPROVEMENTS_SUMMARY.md](IMPROVEMENTS_SUMMARY.md)** - Overview of code improvements and refactoring done
- **[CODE_CHANGES_DETAILED.md](CODE_CHANGES_DETAILED.md)** - Before/after code examples and detailed change explanations
- **[REFACTORING_ROADMAP.md](REFACTORING_ROADMAP.md)** - Future improvements, optimization strategies, and architectural recommendations

### Key Improvements Made
- ✅ Removed 5 unused/incomplete files (Projectile, Room, SaveSystem)
- ✅ Replaced raw pointers with smart references for memory safety
- ✅ Centralized all magic numbers into `Constants.hpp`
- ✅ Improved error handling and error messages
- ✅ Simplified state management (reduced redundancy)
- ✅ Better code organization and naming conventions

## 🎯 Gameplay

### Core Loop
1. **Exploration** - Navigate procedurally generated dungeon
2. **Combat** - Defeat enemies and collect loot
3. **Progression** - Complete floors, face increasingly difficult enemies
4. **Boss Battles** - Encounter boss enemies at progression milestones
5. **Death** - Game ends when player health reaches zero

### Enemy Types
- **Normal enemies** - Basic melee attackers
- **Elite enemies** - Stronger variants with better loot
- **Boss enemies** - Unique powerful enemies at floor milestones

### Power-ups
- **Health Restore** - Recover player health
- **Damage Boost** - Increase attack damage temporarily
- **Speed Boost** - Increase movement speed temporarily

## 🔧 Configuration

Game constants can be adjusted in `Constants.hpp`:

```cpp
namespace Constants {
    namespace Gameplay {
        inline constexpr float AttackRadius = 40.f;           // Player attack range
        inline constexpr int AttackCooldownMs = 500;          // Attack cooldown in ms
        inline constexpr int VisionRadiusTiles = 5;           // Enemy vision range
        inline constexpr float BossSpawnThreshold = 10;       // Enemies before boss
        inline constexpr int BossFloorInterval = 3;           // Spawn boss every N floors
    }
}
```

## 🐛 Known Issues

- Save/load functionality not implemented (SaveSystem.cpp)
- Projectiles not yet implemented
- Room system structure exists but not fully integrated

## 📈 Performance

- Target: 60 FPS at 1280x720 resolution
- Supports hundreds of enemies on-screen
- Optimized collision detection with AABB checks
- Line-of-sight calculations for enemy vision

## 🎨 Future Enhancements

### High Priority
- [ ] Extract Game class into smaller managers
- [ ] Add unit test project
- [ ] Implement configuration file system

### Medium Priority
- [ ] Spatial partitioning (quadtree) for optimization
- [ ] Object pooling for frequent allocations
- [ ] Sound and music system
- [ ] Particle effects system

### Nice to Have
- [ ] Cross-platform build support (CMake)
- [ ] Save/load game functionality
- [ ] Additional visual effects and shaders
- [ ] Leaderboard system

See [REFACTORING_ROADMAP.md](REFACTORING_ROADMAP.md) for detailed improvement strategies.

## 📊 Code Quality

- **Build Status**: ✅ Successful (0 errors, 0 warnings)
- **Code Standard**: C++17
- **Memory Safety**: Reference-based design
- **Error Handling**: Validated font loading and asset paths

## 🤝 Contributing

When contributing code, please follow these guidelines:
- Use PascalCase for class names
- Use camelCase for functions and variables
- Use UPPER_SNAKE_CASE for constants
- Maintain const correctness
- Add meaningful comments for complex logic
- Keep functions focused and under 50 lines when possible

## 📄 License

[Add your license information here]

## 👤 Author

Pixel Dungeon Rush Development Team

---

**Last Updated**: 2026-05-08  
**Status**: Active Development  
**Version**: 1.0.0-beta