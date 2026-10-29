#include "world/WizardTower.h"

#include "characters/Enemy.h"
#include "core/UI.h"
#include "game/Game.h"
#include "world/Flags.h"

namespace {

struct Floor {
    std::string name;
    std::string description;
    std::string bossId;
    int recommendedLevel;
};

// Floors in climbing order. The last entry is the roof and the final battle.
const std::vector<Floor>& floors() {
    static const std::vector<Floor> list = {
        {"The Hall of Statues",
         "Dozens of stone figures line the walls. As you step inside, one of them unfolds its "
         "wings.",
         "gargoyle_sentinel", 4},
        {"The Bone Armory",
         "Racks of rusted weapons fill the room. An armored skeleton rises from a throne of "
         "shields.",
         "skeleton_knight", 5},
        {"The Ember Library",
         "Bookshelves smoulder. In the center, the burning pages swirl together into a living "
         "flame.",
         "flame_elemental", 5},
        {"The Gallery of Shadows",
         "Every lamp here burns black. Something moves between them, faster than your eye can "
         "follow.",
         "shadow_stalker", 6},
        {"The Warlord's Barracks",
         "Broken bunks and gnawed bones litter the floor. A huge ogre in spiked iron hefts "
         "his club and grins.",
         "ogre_warlord", 7},
        {"The Wizard's Perch",
         "Wind howls across the rooftop. The golden dragon Seraphine lies bound in chains of "
         "violet light, her eyes clouded. Malgrath the Wizard turns to face you, his staff "
         "blazing. \"So, the little hero arrives. Kneel - or burn.\"",
         "the_wizard", 8},
    };
    return list;
}

std::string floorLabel(std::size_t index, bool capitalized = false) {
    if (index + 1 == floors().size()) return capitalized ? "The Roof" : "the Roof";
    return "Floor " + std::to_string(index + 1);
}

const char* kDragonArt =
    "                         ___\n"
    "              __        /o  \\___\n"
    "             /  \\___   /   ___  >\n"
    "    /\\/\\/\\  /       \\_/   /   \\/\n"
    "   <  ___ \\/     ___      /\n"
    "    \\/   \\__/\\  /   \\    /\n"
    "              \\/     \\__/\n";

void ending(Game& game) {
    Player& p = game.player();
    p.setFlag(flags::kGameWon, 1);
    ui::header("Victory!");
    ui::wrap(
        "Malgrath staggers back, his staff splitting in two. With a final scream he crumbles "
        "into ash and is scattered by the wind.\n"
        "\n"
        "The violet chains flicker... and shatter. Seraphine lifts her great head, and the "
        "cloud lifts from her golden eyes.");
    ui::print();
    ui::print(kDragonArt);
    ui::wrap(
        "\"You have freed me, " + p.name() +
        ",\" the dragon rumbles, her voice like distant thunder. \"The kingdom will not forget "
        "this. Nor will I.\"\n"
        "\n"
        "She carries you home on her back, and all of Denholm turns out to cheer.\n"
        "\n"
        "*** Congratulations! You have completed The Shackled Dragon. ***\n"
        "You may keep adventuring - don't forget to save at the inn.");
    ui::pause();
    game.travelTo(Game::kStartingArea);
}

void challengeFloor(Game& game, std::size_t index) {
    Player& p = game.player();
    const Floor& floor = floors()[index];
    bool isRoof = index + 1 == floors().size();

    ui::header(floorLabel(index, true) + ": " + floor.name);
    ui::wrap(floor.description, 2);
    ui::print();
    if (p.level() < floor.recommendedLevel) {
        ui::print("  (Recommended level " + std::to_string(floor.recommendedLevel) +
                  ". You are level " + std::to_string(p.level()) + ".)");
    }
    if (isRoof) ui::print("  There will be no running from this fight.");
    if (!ui::confirm("Face the guardian?")) {
        ui::print("You slip back down the stairs for now.");
        return;
    }

    combat::Outcome outcome = game.battle(enemies::get(floor.bossId), !isRoof);
    if (outcome != combat::Outcome::Victory) return;

    p.setFlag(flags::kTowerFloorsCleared, static_cast<int>(index) + 1);
    if (isRoof) {
        ending(game);
    } else {
        ui::print("The guardian has fallen. A stairway leading up to " + floorLabel(index + 1) +
                  " stands open.");
    }
}

}  // namespace

bool WizardTower::isUnlocked(const Player& player) const {
    return player.flag(flags::kTowerFound) != 0;
}

std::string WizardTower::description(const Player& player) const {
    if (player.flag(flags::kGameWon)) {
        return "The tower stands silent and empty now, its violet glow gone. Birds have "
               "already begun to nest on the roof.";
    }
    int cleared = player.flag(flags::kTowerFloorsCleared);
    return "A spire of black stone crackling with violet light. Five floors of guardians "
           "stand between you and the roof, where the Wizard holds the dragon. Floors "
           "cleared: " +
           std::to_string(cleared < 5 ? cleared : 5) + "/5.";
}

void WizardTower::enter(Game& game) {
    while (!game.shouldLeaveArea()) {
        std::size_t cleared = static_cast<std::size_t>(game.player().flag(flags::kTowerFloorsCleared));
        std::vector<MenuAction> actions;
        if (cleared < floors().size()) {
            actions.push_back({"Climb to " + floorLabel(cleared) + ": " + floors()[cleared].name,
                               [&game, cleared] { challengeFloor(game, cleared); }});
        } else {
            actions.push_back({"Gaze out from the rooftop", [] {
                                   ui::wrap(
                                       "The Whisperwood stretches to the horizon. Far away, "
                                       "a golden shape glides over Denholm.");
                               }});
        }
        game.showAreaMenu(*this, actions);
    }
}
