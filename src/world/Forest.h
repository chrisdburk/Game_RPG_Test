#pragma once

#include "world/Area.h"

// Random encounters and treasure. Exploring long enough reveals the Wizard's Tower.
class Forest : public Area {
public:
    std::string id() const override { return "forest"; }
    std::string name() const override { return "The Whisperwood"; }
    std::string summary() const override { return "a dark forest full of monsters"; }
    std::string description(const Player& player) const override;
    std::vector<std::string> connections() const override { return {"denholm", "wizard_tower"}; }
    void enter(Game& game) override;
};
