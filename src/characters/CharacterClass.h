#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/Stats.h"

class Player;
struct Enemy;

// A special combat action that costs mana.
struct Ability {
    std::string name;
    std::string description;
    int manaCost = 0;
    // Performs the ability and prints what happened. Mana is already spent.
    std::function<void(Player&, Enemy&)> use;
};

struct StartingItem {
    std::string itemId;
    int count = 1;
};

// Base for all playable classes. To add a class:
//   1. Create a subclass (see Fighter.h / Fighter.cpp).
//   2. Register it in classes::all() in CharacterClass.cpp.
// It will then appear on the character creation screen and in save files.
class CharacterClass {
public:
    virtual ~CharacterClass() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual Stats baseStats() const = 0;

    // Hit points at level 1 and gained per level, each before the Con modifier.
    virtual int baseHealth() const = 0;
    virtual int healthPerLevel() const = 0;
    // Mana at level 1 and gained per level, each before the manaStat() modifier.
    virtual int baseMana() const = 0;
    virtual int manaPerLevel() const = 0;

    virtual Stat attackStat() const { return Stat::Str; }
    virtual Stat manaStat() const { return Stat::Wis; }

    virtual std::string startingWeapon() const = 0;
    virtual std::string startingArmor() const { return ""; }
    virtual std::vector<StartingItem> startingItems() const { return {}; }
    virtual int startingGold() const { return 25; }

    virtual std::vector<Ability> abilities() const = 0;
};

namespace classes {

const std::vector<std::unique_ptr<CharacterClass>>& all();
const CharacterClass* find(const std::string& name);  // nullptr if unknown

}  // namespace classes
