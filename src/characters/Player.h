#pragma once

#include <map>
#include <string>

#include "characters/CharacterClass.h"
#include "core/Stats.h"
#include "items/Inventory.h"
#include "quests/Quest.h"

struct LevelUpResult {
    int hpGained = 0;
    int manaGained = 0;
};

struct ItemUseResult {
    bool used = false;  // false if the item was not consumed
    std::string message;
};

class Player {
public:
    Player(std::string name, const CharacterClass& characterClass);

    // --- Identity & resources ---------------------------------------------
    const std::string& name() const { return name_; }
    const CharacterClass& characterClass() const { return *class_; }
    int level() const { return level_; }
    int xp() const { return xp_; }
    int xpToNextLevel() const { return level_ * 100; }
    int hp() const { return hp_; }
    int maxHp() const { return maxHp_; }
    int mana() const { return mana_; }
    int maxMana() const { return maxMana_; }
    int gold() const { return gold_; }
    const Stats& stats() const { return stats_; }
    bool isAlive() const { return hp_ > 0; }

    Inventory& inventory() { return inventory_; }
    const Inventory& inventory() const { return inventory_; }
    QuestLog& quests() { return quests_; }
    const QuestLog& quests() const { return quests_; }

    // --- Combat numbers -----------------------------------------------------
    int proficiency() const { return 2 + (level_ - 1) / 4; }
    int attackBonus() const;
    int armorClass() const;
    int damageDie() const;     // sides of the weapon die (d6 armed, d4 unarmed)
    int damageBonus() const;   // stat modifier + weapon bonus
    std::string weaponName() const;
    std::string armorName() const;

    // --- Changes ------------------------------------------------------------
    int heal(int amount);          // returns HP actually restored
    int restoreMana(int amount);   // returns MP actually restored
    void takeDamage(int amount);
    bool spendMana(int amount);    // false if not enough
    void fullRestore();
    void addGold(int amount);
    bool spendGold(int amount);    // false if not enough
    void loseGold(int amount);     // never drops below zero

    void addXp(int amount) { xp_ += amount; }
    bool canLevelUp() const { return xp_ >= xpToNextLevel(); }
    LevelUpResult levelUp(Stat improvedStat);

    // --- Equipment & items ---------------------------------------------------
    const std::string& weaponId() const { return weaponId_; }
    const std::string& armorId() const { return armorId_; }
    // Equips a weapon or armor from the inventory; the old piece goes back in the pack.
    bool equip(const std::string& itemId);
    ItemUseResult useConsumable(const std::string& itemId);

    // --- World progress -----------------------------------------------------
    // Generic named counters so areas can track progress without changing Player.
    int flag(const std::string& key) const;
    void setFlag(const std::string& key, int value) { flags_[key] = value; }
    const std::map<std::string, int>& flags() const { return flags_; }

private:
    friend class SaveSystem;

    std::string name_;
    const CharacterClass* class_;
    Stats stats_;
    int level_ = 1;
    int xp_ = 0;
    int hp_ = 1;
    int maxHp_ = 1;
    int mana_ = 0;
    int maxMana_ = 0;
    int gold_ = 0;
    std::string weaponId_;
    std::string armorId_;
    Inventory inventory_;
    QuestLog quests_;
    std::map<std::string, int> flags_;
};
