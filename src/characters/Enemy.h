#pragma once

#include <string>
#include <vector>

// An attack that automatically hits (spells, breath weapons, slams...).
struct EnemySpecial {
    std::string name;
    std::string text;  // flavor shown when used
    int chance = 0;    // percent chance per turn; 0 = no special
    int diceCount = 0;
    int diceSides = 0;
    int bonus = 0;
};

struct LootDrop {
    std::string itemId;
    int chance = 0;  // percent
};

// Static description of a monster. Add new enemies in Enemy.cpp.
struct EnemyTemplate {
    std::string id;
    std::string name;
    std::string description;
    int maxHp = 1;
    int armorClass = 10;
    int attackBonus = 0;
    int diceCount = 1;  // damage = diceCount d diceSides + damageBonus
    int diceSides = 4;
    int damageBonus = 0;
    int xp = 0;
    int goldMin = 0;
    int goldMax = 0;
    int minLevel = 1;  // lowest player level for random encounters
    std::string attackVerb = "attacks you";
    EnemySpecial special;
    std::vector<LootDrop> loot;
    std::string art;  // optional ASCII art shown when the fight starts
    bool properName = false;  // true for named foes ("Malgrath the Wizard")

    // Name for use in sentences: "the Goblin" / "Malgrath the Wizard".
    std::string theName(bool capitalized = false) const {
        if (properName) return name;
        return (capitalized ? "The " : "the ") + name;
    }
    // Name with an indefinite article: "a Goblin", "an Orc".
    std::string aName() const {
        if (properName) return name;
        bool vowel = !name.empty() && std::string("AEIOUaeiou").find(name[0]) != std::string::npos;
        return (vowel ? "an " : "a ") + name;
    }
};

// A live enemy in a fight.
struct Enemy {
    explicit Enemy(const EnemyTemplate& t) : info(&t), hp(t.maxHp) {}

    const std::string& name() const { return info->name; }
    bool isAlive() const { return hp > 0; }

    const EnemyTemplate* info;
    int hp;
};

namespace enemies {

const EnemyTemplate* find(const std::string& id);  // nullptr if unknown
const EnemyTemplate& get(const std::string& id);   // throws std::out_of_range if unknown
const std::vector<EnemyTemplate>& all();

// Picks a random enemy from `pool` that suits the player's level,
// favouring enemies close to that level.
const EnemyTemplate& randomEncounter(const std::vector<std::string>& pool, int playerLevel);

}  // namespace enemies
