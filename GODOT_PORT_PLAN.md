# PixelDungeonRush — Godot Port Plan

## Feature Summary

**Original stack:** C++17 + SFML 3.0, tile-based, procedurally generated, 1280×720 @ 60 FPS.
All visuals are SFML shapes (no image files) — enemies are red rectangles, player is a green square.

### Core Systems

| System | Summary |
|--------|---------|
| **Movement** | WASD/arrows, 140 px/s, AABB + tile collision with axis-sliding |
| **Dungeon Gen** | 100×72 tile map, BSP-ish room placement (5 rooms max), 3-wide corridors |
| **FOV / Fog of War** | Bresenham LOS, 5-tile radius, persistent "discovered" layer |
| **Enemy AI** | LOS detection (300 px), pathfinding toward player, contact DPS |
| **Combat** | Player AoE circle attack (40 px radius, 35–45 dmg, 500 ms cooldown) |
| **Enemy Rarities** | Common / Elite / Boss — different outlines, HP, and drop tables |
| **Loot Drops** | Heal (+15–50 HP), Speed Boost, Damage Boost — timed buffs |
| **Floor Progression** | Kill 40% of enemies → press T to advance; bosses every 3rd floor |
| **Boss System** | Triggers at 10 kills on boss floors, spawns 6–12 tiles away |
| **Minimap** | Real-time render texture (200×144 px), top-left, shows FOV + player |
| **HUD** | Health bar, floor counter, enemy kill counter, floating damage numbers |
| **Config** | `assets/config.json` with nested keys — all tunables externalized |
| **Stats** | Appends floor + enemy count to `runs.txt` on death |
| **Stubs** | `Projectile`, `SaveSystem` exist in C++ but are unimplemented |

### Key Constants

| Constant | Value |
|----------|-------|
| Tile size | 32 px |
| Map size | 100 × 72 tiles |
| Player speed | 140 px/s |
| Attack radius | 40 px |
| Attack damage | 35–45 + boost |
| Attack cooldown | 500 ms |
| Vision radius | 5 tiles |
| Enemy contact DPS | 30 |
| Enemy attack range | 40 px |
| Enemy attack windup | 350 ms |
| Enemy attack cooldown | 900 ms |
| Boss spawn threshold | 10 kills |
| Boss floor interval | every 3rd floor |
| Floor clear threshold | 40% of spawned enemies |
| Pickup auto-radius | 1000 px |

---

## Recommended Godot Setup

- **Godot 4.x**
- **Language:** GDScript (or C# — logic complexity is low)
- **Rendering:** 2D, pixel-art camera
- **Physics:** `CharacterBody2D` + `move_and_slide()` — matches AABB+slide collision model exactly

---

## Phase 1 — Foundation

**Goal:** Walking player in a hand-built room.

- [ ] New Godot 4 project, 1280×720 viewport, pixel-perfect camera settings
- [ ] Import `assets/Kenney Future.ttf`
- [ ] Single TileMapLayer with 32×32 floor/wall tiles (colored rects are fine to start)
- [ ] `Player` scene — `CharacterBody2D` + `CollisionShape2D` (28×28 rect)
- [ ] WASD movement using `move_and_slide()`
- [ ] `Config` autoload — reads `assets/config.json` via `FileAccess` + `JSON.parse_string()`

---

## Phase 2 — Dungeon Generation

**Goal:** Procedural dungeon each run.

- [ ] `DungeonGenerator` autoload (or Node) that outputs a `int[][]` grid (0 = floor, 1 = wall)
- [ ] Room placement: attempt 40 placements, fit up to 5 rooms (8–16 w, 6–12 h, 2-tile buffer)
- [ ] Corridor carving: horizontal then vertical, 3 tiles wide
- [ ] Write grid to `TileMapLayer` via `set_cell()`
- [ ] `find_spawn_point()` — pick random floor tile for player start

**Reference:** `src/Dungeon.cpp` — port the room + corridor logic directly.

---

## Phase 3 — FOV / Fog of War

**Goal:** Classic roguelike visibility with persistent discovery.

- [ ] Bresenham LOS helper function (port from `Dungeon.cpp:lineOfSightClear()`)
- [ ] Two overlay `TileMapLayer`s:
  - **Undiscovered** (solid black) — cleared permanently once seen
  - **Out-of-sight** (dark tint ~50% black) — toggled per-frame
- [ ] `mark_visible()` called each frame from player tile position, radius 5 tiles
- [ ] Only entities on currently-visible tiles are drawn

---

## Phase 4 — Enemy System

**Goal:** Enemies that chase and attack.

- [ ] `Enemy` scene — `CharacterBody2D`, rarity enum (`COMMON / ELITE / BOSS`), color modulate
  - Common: red rect
  - Elite: red rect + blue outline
  - Boss: red rect + magenta outline
- [ ] AI state machine (simple enum + `match` in `_physics_process()`):
  - `IDLE` → `DETECTED` (LOS + within 300 px) → `CHASING` → `ATTACKING`
- [ ] Reuse Bresenham LOS for per-enemy sight check
- [ ] Attack windup: 350 ms color telegraph → apply damage → 900 ms cooldown
- [ ] Enemy HP and damage scale with floor number: `(floor - 1) * 2` bonus damage
- [ ] `EnemyManager` node tracks the active enemies vector, handles spawning

---

## Phase 5 — Combat & Loot

**Goal:** Satisfying combat loop.

- [ ] Player attack: `Area2D` circle (40 px radius), enabled for 100 ms on F/LMB press
  - Damage: 35–45 + damage boost modifier
  - Cooldown: 500 ms
  - Visual: green circle flash
- [ ] Damage numbers: `Label` nodes that tween upward (−30 px/s) and fade over 0.6 s
- [ ] Loot drops on enemy death:
  - Position: random 5–18 px offset from corpse
  - Weighted random roll against drop table from `config.json`
  - `Area2D` pickup zone, auto-collect within 1000 px of player
- [ ] Buff types applied to player:
  - **Heal** — instant HP restore
  - **Speed Boost** — speed multiplier, timed
  - **Damage Boost** — flat damage add, timed
- [ ] Timed buffs: one `Timer` per active buff, remove on timeout

**Drop table reference** (from `src/Loot.cpp`):

| Enemy | Drop Chance | Rolls | Loot |
|-------|-------------|-------|------|
| Common | 70% | 1 | Heal 15 HP (90%), Speed +15 px/s 15 s (10%) |
| Elite | 75% | 1–2 | Heal 30 HP / Dmg +10 20 s / Speed +25 20 s |
| Boss | 100% | 2–4 | Heal 50 HP / Dmg +20 30 s / Speed +30 30 s |

---

## Phase 6 — Floor Progression & Bosses

**Goal:** Full run loop with escalating difficulty.

- [ ] `FloorManager` autoload:
  - Tracks enemies spawned and killed per floor
  - Clear threshold: `floor(spawned * 0.4)` kills required
  - Press T to advance when threshold met
- [ ] On floor advance:
  - Regenerate dungeon
  - Clear and respawn enemies: `initial_count + (floor + 2)` per floor
  - Heal player +10 HP
  - Increment floor counter
- [ ] Boss spawn logic:
  - Only on floors divisible by 3
  - Triggers after 10 kills on that floor
  - Spawn 6–12 tiles away from player (random valid floor tile in range)
  - Boss has magenta outline, guaranteed 2–4 loot rolls on death

---

## Phase 7 — UI & Polish

**Goal:** Functional HUD matching the original.

- [ ] **Minimap** — `SubViewport` rendering a top-down view, displayed top-left as `TextureRect`
  - Scale: 2× (200×144 px on screen)
  - Colors: wall = dark, floor = light, player = green, boss = red
  - Only shows discovered + currently visible tiles
- [ ] **Health bar** — `ProgressBar` or custom `Control`, top-right, 200×20 px, red fill
- [ ] **HUD labels** — floor number, "X of Y enemies defeated this floor"
- [ ] **Death screen** — `CanvasLayer` overlay, semi-transparent panel, "Press R to restart"
- [ ] **Floor-clear prompt** — text hint when advance threshold is met
- [ ] **Stats** — append floor number + enemies killed to `user://runs.txt` on death

---

## Phase 8 — Extras (Optional)

These are stubbed in the original C++ project and not yet implemented:

- [ ] **Projectiles** — ranged player attack, `Area2D` + tween movement
- [ ] **Save/Load** — persist best run to `user://save.json`
- [ ] **Audio** — attack SFX, pickup chime, floor-advance jingle (no audio in original)
- [ ] **Sprites** — replace colored rects with pixel-art sprite sheets
- [ ] **Gamepad support** — `InputMap` remapping for controller play
- [ ] **More enemy types** — different movement patterns or attack styles
- [ ] **More loot types** — shields, area effects, etc.

---

## Godot-Specific Implementation Notes

### Physics & Collision
`CharacterBody2D.move_and_slide()` handles axis-sliding automatically — no need to manually implement the X/Y fallback logic from `Entity.cpp`.

### Dungeon Generation
The BSP room + corridor algorithm in `src/Dungeon.cpp` translates nearly line-for-line to GDScript. Port the logic directly rather than using a Godot plugin.

### Bresenham LOS
Also nearly line-for-line portable. Write it as a static helper on `DungeonGenerator`.

### Enemy AI
A simple state enum + `match` in `_physics_process()` is sufficient. No behavior tree needed.

### Performance (many enemies)
If targeting "hundreds of enemies on screen" (original design intent), consider `MultiMeshInstance2D` for enemy rendering instead of individual `CharacterBody2D` nodes. The physics can still use `CharacterBody2D` but the draw calls collapse into one.

### Scene Tree Layout (suggested)

```
Main (Node2D)
├── DungeonLayer (TileMapLayer)        ← floor/wall tiles
├── FogUndiscovered (TileMapLayer)     ← permanent black
├── FogOutOfSight (TileMapLayer)       ← per-frame dim
├── Pickups (Node2D)                   ← loot items
├── Enemies (Node2D)                   ← enemy instances
├── Player (CharacterBody2D)
├── Camera2D
└── UI (CanvasLayer)
    ├── Minimap (SubViewport + TextureRect)
    ├── HealthBar
    ├── HUDLabels
    └── DeathScreen
```

### Autoloads (suggested)

| Autoload | Role |
|----------|------|
| `Config` | Loads and exposes `config.json` values |
| `DungeonGenerator` | Generates and holds the current floor grid |
| `FloorManager` | Tracks floor state, kill counts, progression |
| `LootTable` | Weighted random loot generation |

---

## File Reference (Original C++ Sources)

| Godot System | Port From |
|---|---|
| DungeonGenerator | `src/Dungeon.cpp` |
| Player movement | `src/Player.cpp`, `src/Entity.cpp` |
| Enemy AI | `src/Enemy.cpp` |
| Combat | `src/CombatSystem.cpp` |
| Loot | `src/Loot.cpp` |
| Floor progression | `src/FloorManager.cpp` |
| UI / Minimap | `src/UI.cpp` |
| Config loading | `src/Config.cpp` |
| Constants | `src/Constants.hpp` |
