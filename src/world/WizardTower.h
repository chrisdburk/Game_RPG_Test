#pragma once

#include "world/Area.h"

// Five floors of bosses, with the Wizard waiting on the roof.
class WizardTower : public Area {
public:
    std::string id() const override { return "wizard_tower"; }
    std::string name() const override { return "The Wizard's Tower"; }
    std::string summary() const override { return "five floors of guardians, and the Wizard"; }
    std::string description(const Player& player) const override;
    std::vector<std::string> connections() const override { return {"forest"}; }
    bool isUnlocked(const Player& player) const override;
    void enter(Game& game) override;
};
