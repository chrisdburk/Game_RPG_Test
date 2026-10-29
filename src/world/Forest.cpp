#include "world/Forest.h"

#include "characters/Enemy.h"
#include "core/Random.h"
#include "core/UI.h"
#include "game/Game.h"
#include "world/Flags.h"

namespace {

// Enemies that can appear here (filtered by player level in enemies::randomEncounter).
const std::vector<std::string> kEncounters = {"gray_wolf", "goblin", "giant_spider", "bandit",
                                              "orc",       "dire_wolf", "forest_troll"};

// The tower can be found after this many explorations, and always is by kTowerGuaranteed.
constexpr int kTowerEarliest = 6;
constexpr int kTowerGuaranteed = 12;
constexpr int kTowerChance = 30;

const char* kTowerArt =
    "                 |>>>\n"
    "                 |\n"
    "             _  _|_  _\n"
    "            |;|_|;|_|;|\n"
    "            \\\\.    .  /\n"
    "             \\\\:  .  /\n"
    "              ||:   |\n"
    "              ||:.  |\n"
    "              ||:  .|\n"
    "              ||:   |\n"
    "         _____||:___|_____\n";

void discoverTower(Player& player) {
    player.setFlag(flags::kTowerFound, 1);
    ui::print(kTowerArt);
    ui::wrap(
        "The trees part and the ground turns to black, scorched earth. Before you rises a "
        "tower of dark stone, wrapped in crackling violet light. High above, chained to its "
        "roof, a great golden dragon thrashes and roars.\n"
        "\n"
        "You have found the Wizard's Tower! You can now travel here from the forest.");
}

void explore(Game& game) {
    Player& player = game.player();
    int explorations = player.flag(flags::kForestExplorations) + 1;
    player.setFlag(flags::kForestExplorations, explorations);
    bool towerFound = player.flag(flags::kTowerFound) != 0;

    ui::print();
    if (!towerFound && explorations >= kTowerEarliest &&
        (explorations >= kTowerGuaranteed || rng::chance(kTowerChance))) {
        discoverTower(player);
        ui::pause();
        return;
    }
    if (!towerFound && explorations >= 3 && rng::chance(35)) {
        ui::wrap(
            "Through a gap in the canopy you glimpse a dark spire, wreathed in violet light, "
            "far to the north...\n");
    }

    int roll = rng::range(1, 100);
    if (roll <= 55) {
        const EnemyTemplate& enemy = enemies::randomEncounter(kEncounters, player.level());
        ui::wrap("Branches snap nearby. Something has caught your scent!");
        game.battle(enemy);
        return;
    }

    if (roll <= 70) {
        int gold = rng::range(3, 12) + player.level() * 2;
        ui::wrap("Beneath the roots of an old oak you find a rotted pouch holding " +
                 std::to_string(gold) + " gold.");
        player.addGold(gold);
    } else if (roll <= 85) {
        ui::wrap("A shaft of sunlight falls on a small clearing where pale flowers bloom.");
        game.giveItem("moonpetal_herb", rng::range(1, 2));
    } else if (roll <= 92) {
        ui::wrap("You find a fallen adventurer's pack. Inside, a potion is still intact.");
        game.giveItem(rng::chance(65) ? "health_potion" : "mana_potion");
    } else {
        int healed = player.heal(rng::roll(1, 6) + player.level());
        ui::wrap("You find a quiet glade with a clear spring and rest for a while. (+" +
                 std::to_string(healed) + " HP)");
    }
    ui::pause();
}

}  // namespace

std::string Forest::description(const Player& player) const {
    std::string text =
        "Ancient trees crowd together, their branches blotting out the sun. Strange calls "
        "echo through the gloom, and the path splits a dozen ways.";
    if (player.flag(flags::kTowerFound)) {
        text += " To the north, the Wizard's Tower looms above the canopy.";
    }
    return text;
}

void Forest::enter(Game& game) {
    while (!game.shouldLeaveArea()) {
        game.showAreaMenu(*this, {{"Explore the forest", [&game] { explore(game); }}});
    }
}
