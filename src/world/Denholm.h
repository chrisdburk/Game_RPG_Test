#pragma once

#include "world/Area.h"

// The hub town: inn (rest & save), shops, and the Adventurers' Guild Hall.
class Denholm : public Area {
public:
    std::string id() const override { return "denholm"; }
    std::string name() const override { return "Denholm"; }
    std::string summary() const override { return "the hub town: rest, shop and find quests"; }
    std::string description(const Player& player) const override;
    std::vector<std::string> connections() const override { return {"forest"}; }
    void enter(Game& game) override;
};
