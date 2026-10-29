#pragma once

#include <string>
#include <vector>

class Game;
class Player;

// A location the player can visit. To add a new area:
//   1. Derive from Area (see Denholm, Forest and WizardTower for examples).
//   2. Register it in Game::Game() with registerArea().
//   3. Add its id to connections() of the areas that lead to it.
// Use Player::flag()/setFlag() (keys in world/Flags.h) to remember progress;
// flags are saved automatically.
class Area {
public:
    virtual ~Area() = default;

    virtual std::string id() const = 0;       // unique key, e.g. "forest"
    virtual std::string name() const = 0;     // display name
    virtual std::string summary() const = 0;  // one line shown in the travel menu
    virtual std::string description(const Player& player) const = 0;  // shown on arrival
    virtual std::vector<std::string> connections() const = 0;         // reachable area ids
    virtual bool isUnlocked(const Player&) const { return true; }

    // Runs the area's menu loop until game.shouldLeaveArea() becomes true.
    virtual void enter(Game& game) = 0;
};
