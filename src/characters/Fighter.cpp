#include "characters/Fighter.h"

#include "characters/Enemy.h"
#include "characters/Player.h"
#include "combat/Combat.h"
#include "core/Random.h"
#include "core/UI.h"

std::string Fighter::description() const {
    return "A master of arms who wins battles with strength and steel.";
}

std::vector<StartingItem> Fighter::startingItems() const {
    return {{"health_potion", 1}, {"mana_potion", 1}};
}

std::vector<Ability> Fighter::abilities() const {
    return {
        {"Power Strike", "A mighty blow: +2 to hit and double damage.", 3,
         [](Player& player, Enemy& enemy) {
             ui::print("You plant your feet and put everything into a Power Strike!");
             combat::playerAttack(player, enemy, 2, 2);
         }},
        {"Second Wind", "Catch your breath and heal 1d10 + your level in HP.", 4,
         [](Player& player, Enemy&) {
             int healed = player.heal(rng::roll(1, 10) + player.level());
             ui::print("You catch your second wind and recover " + std::to_string(healed) +
                       " HP.");
         }},
    };
}
