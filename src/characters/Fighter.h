#pragma once

#include "characters/CharacterClass.h"

// A sturdy melee warrior who relies on strength and toughness.
class Fighter : public CharacterClass {
public:
    std::string name() const override { return "Fighter"; }
    std::string description() const override;
    Stats baseStats() const override { return Stats(15, 13, 14, 12, 10, 8); }

    int baseHealth() const override { return 20; }
    int healthPerLevel() const override { return 6; }
    int baseMana() const override { return 6; }
    int manaPerLevel() const override { return 2; }

    std::string startingWeapon() const override { return "shortsword"; }
    std::vector<StartingItem> startingItems() const override;
    int startingGold() const override { return 30; }

    std::vector<Ability> abilities() const override;
};
