# The Shackled Dragon

A simple text-based RPG written in C++17, played entirely in the terminal with plain ASCII text.

> An evil wizard has taken control of the kingdom's dragon protector and is using her to wreak
> havoc across the land. You will adventure to the Wizard's Tower, defeat the Wizard, and free
> the dragon.

## Building and running

You need a C++17 compiler (g++ 8+, clang 7+, or MSVC 2019+).

**With make:**

```bash
make
```

```bash
./rpg
```

**With CMake (also works on Windows / Visual Studio):**

```bash
cmake -S . -B build
```

```bash
cmake --build build
```

```bash
./build/rpg
```

**With just a compiler:**

```bash
g++ -std=c++17 -O2 -Isrc $(find src -name '*.cpp') -o rpg
```

The game saves to `savegame.txt` in the directory you run it from.

## How to play

Every screen shows numbered choices: type the number and press Enter.

### Areas

| Area | What you can do |
|------|-----------------|
| **Denholm** (hub town) | Rest at the Sleeping Griffin Inn (restores HP and MP, saves your game), buy potions at Hilda's Provisions, buy weapons and armor at the Iron Anvil Smithy, and take side quests at the Adventurers' Guild Hall. |
| **The Whisperwood** (forest) | Explore for random monster encounters, gold, potions and herbs. Keep exploring and you'll eventually find the Wizard's Tower. |
| **The Wizard's Tower** | Five floors, each guarded by a boss, with Malgrath the Wizard waiting on the roof. Progress is kept between visits, so you can go back to town to rest between floors. |

From any area you can open your **character sheet**, **inventory**, and **quest log**, or **travel**
to a connected area.

### Character

The **Fighter** is the only class for now. Starting stats:

| Str | Dex | Con | Int | Wis | Cha |
|-----|-----|-----|-----|-----|-----|
| 15  | 13  | 14  | 12  | 10  | 8   |

Each stat gives a modifier of `(score - 10) / 2`, rounded down. Strength adds to attack and
damage, Dexterity to Armor Class and fleeing, and Constitution to hit points.

You start with:

- a **Shortsword** (+5 damage)
- a **Health Potion** (heals 1-8 HP)
- a **Mana Potion** (restores 1-10 MP)

Fighter abilities (cost mana):

- **Power Strike** (3 MP): +2 to hit and double damage.
- **Second Wind** (4 MP): heal 1d10 + your level.

### Combat

Attacks roll a d20 plus your attack bonus against the target's Armor Class (AC). A natural 20 is a
critical hit (double damage dice), and a natural 1 always misses. Weapon damage is 1d6 + Strength
modifier + the weapon's bonus. If you're defeated you wake up in Denholm, minus half your gold.

### Leveling

You need `level x 100` XP to reach the next level. Each level raises your max HP and MP, fully
restores you, and lets you raise one stat by 1 (to a maximum of 20).

### Quests

The Guild Hall's quest board posts three random side quests, refreshed every time you rest at the
inn. You can carry up to three quests at a time. Each quest has a **name**, a **description**, and
a **reward** (gold, XP, and sometimes potions). There are two kinds of quest:

- **Slay** quests, where you defeat a number of a certain monster.
- **Gather** quests, where you bring back items such as Moonpetal Herbs or Wolf Pelts.

Turn completed quests in at the Guild Hall.

## Project layout

```
src/
  main.cpp                 entry point
  core/                    UI (console I/O), Random (dice), Stats (ability scores)
  items/                   Item database, Inventory
  characters/              CharacterClass + Fighter, Player, Enemy database
  combat/                  turn-based battle logic
  quests/                  Quest, QuestLog, random quest board generator
  world/                   Area base class, Denholm, Forest, WizardTower, Flags (progress keys)
  game/                    Game (main loop, shared menus), SaveSystem
```

## Extending the game

The game is built so new content can be added without changing the core systems.

### Adding a new area

1. Create `src/world/MyArea.h/.cpp` with a class that derives from `Area` (see `Forest` for a
   small example). Implement `id()`, `name()`, `summary()`, `description()`, `connections()`,
   and `enter()`. Override `isUnlocked()` if the area should be hidden until something happens.
2. Inside `enter()`, loop on `while (!game.shouldLeaveArea())` and call
   `game.showAreaMenu(*this, { {"Label", [&game] { ... }}, ... })`. The character sheet,
   inventory, quest log, travel, and quit options are added for you.
3. Register it in `Game::Game()` in `src/game/Game.cpp`:
   `registerArea(std::make_unique<MyArea>());`
4. Add its id to the `connections()` of the areas that lead to it.
5. To remember progress, use `player.flag("key")` / `player.setFlag("key", value)` and add the
   key to `src/world/Flags.h`. Flags are saved automatically.

### Adding a new class

1. Create a subclass of `CharacterClass` (see `src/characters/Fighter.h/.cpp`) that sets base
   stats, health and mana growth, starting gear, and abilities. Abilities are lambdas, so each class
   can do anything in combat.
2. Register it in `classes::all()` in `src/characters/CharacterClass.cpp`. It will show up on the
   character creation screen and work with save files automatically.

### Adding items, enemies, and quests

- **Items:** add an entry to `buildItems()` in `src/items/Item.cpp`, then put its id in a shop's
  stock list in `src/world/Denholm.cpp` or in an enemy's loot table.
- **Enemies:** add an entry to `buildEnemies()` in `src/characters/Enemy.cpp`, then add its id to
  an area's encounter list (for example `kEncounters` in `src/world/Forest.cpp`).
- **Quests:** add a template to `templates()` in `src/quests/Quest.cpp`. `{n}` in the description
  is replaced with the required count.
