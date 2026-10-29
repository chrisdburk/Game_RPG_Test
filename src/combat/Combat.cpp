#include "combat/Combat.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "characters/Enemy.h"
#include "characters/Player.h"
#include "core/Random.h"
#include "core/UI.h"
#include "items/Item.h"

namespace combat {

namespace {

std::string rollText(int roll, int bonus, int total) {
    return "(rolled " + std::to_string(roll) + " " + (bonus >= 0 ? "+ " : "- ") +
           std::to_string(bonus >= 0 ? bonus : -bonus) + " = " + std::to_string(total);
}

void showStatus(const Player& player, const Enemy& enemy) {
    ui::print();
    ui::print("  " + ui::padRight(player.name(), 20) + "HP " +
              ui::bar(player.hp(), player.maxHp(), 16) + " " + std::to_string(player.hp()) +
              "/" + std::to_string(player.maxHp()) + "   MP " + std::to_string(player.mana()) +
              "/" + std::to_string(player.maxMana()));
    ui::print("  " + ui::padRight(enemy.name(), 20) + "HP " +
              ui::bar(enemy.hp, enemy.info->maxHp, 16) + " " + std::to_string(enemy.hp) + "/" +
              std::to_string(enemy.info->maxHp));
    ui::print();
}

void enemyTurn(Player& player, Enemy& enemy) {
    const EnemyTemplate& t = *enemy.info;
    ui::print();

    if (t.special.chance > 0 && rng::chance(t.special.chance)) {
        int damage = rng::roll(t.special.diceCount, t.special.diceSides) + t.special.bonus;
        damage = std::max(1, damage);
        player.takeDamage(damage);
        ui::print(t.theName(true) + " uses " + t.special.name + "! " + t.special.text);
        ui::print("You take " + std::to_string(damage) + " damage.");
        return;
    }

    int roll = rng::roll(1, 20);
    int total = roll + t.attackBonus;
    std::string attempt = t.theName(true) + " " + t.attackVerb + " " +
                          rollText(roll, t.attackBonus, total) + " vs AC " +
                          std::to_string(player.armorClass()) + ")";

    bool critical = roll == 20;
    bool hit = critical || (roll != 1 && total >= player.armorClass());
    if (!hit) {
        ui::print(attempt + " but misses!");
        return;
    }

    int dice = rng::roll(t.diceCount, t.diceSides);
    if (critical) dice += rng::roll(t.diceCount, t.diceSides);
    int damage = std::max(1, dice + t.damageBonus);
    player.takeDamage(damage);
    ui::print(attempt + (critical ? " - CRITICAL HIT! " : " and hits! ") + "You take " +
              std::to_string(damage) + " damage.");
}

// Each of these returns true if the player used up their turn.
bool useAbility(Player& player, Enemy& enemy) {
    std::vector<Ability> abilities = player.characterClass().abilities();
    if (abilities.empty()) {
        ui::print("You have no special abilities.");
        return false;
    }

    std::vector<std::string> labels;
    for (const Ability& ability : abilities) {
        labels.push_back(ability.name + " (" + std::to_string(ability.manaCost) + " MP) - " +
                         ability.description);
    }
    int choice = ui::chooseOrBack(labels, "Use which ability?");
    if (choice < 0) return false;

    const Ability& ability = abilities[static_cast<std::size_t>(choice)];
    if (!player.spendMana(ability.manaCost)) {
        ui::print("You don't have enough mana for " + ability.name + ".");
        return false;
    }
    ui::print();
    ability.use(player, enemy);
    return true;
}

bool useItem(Player& player) {
    std::vector<std::string> ids;
    std::vector<std::string> labels;
    for (const auto& [id, count] : player.inventory().entries()) {
        const Item* item = items::find(id);
        if (!item || item->type != ItemType::Consumable) continue;
        ids.push_back(id);
        labels.push_back(item->name + " x" + std::to_string(count) + " - " + item->summary());
    }
    if (ids.empty()) {
        ui::print("You have no usable items.");
        return false;
    }

    int choice = ui::chooseOrBack(labels, "Use which item?");
    if (choice < 0) return false;
    ItemUseResult result = player.useConsumable(ids[static_cast<std::size_t>(choice)]);
    ui::print();
    ui::print(result.message);
    return result.used;
}

bool tryFlee(const Player& player) {
    int dexMod = player.stats().modifier(Stat::Dex);
    int roll = rng::roll(1, 20);
    int total = roll + dexMod;
    bool escaped = total >= 10;
    ui::print();
    ui::print("You try to escape " + rollText(roll, dexMod, total) + " vs 10)" +
              (escaped ? " and get away!" : " but can't break free!"));
    return escaped;
}

}  // namespace

int playerAttack(Player& player, Enemy& enemy, int hitBonus, int damageMultiplier) {
    const EnemyTemplate& t = *enemy.info;
    int bonus = player.attackBonus() + hitBonus;
    int roll = rng::roll(1, 20);
    int total = roll + bonus;
    std::string attempt = "You swing your " + player.weaponName() + " " +
                          rollText(roll, bonus, total) + " vs AC " +
                          std::to_string(t.armorClass) + ")";

    bool critical = roll == 20;
    bool hit = critical || (roll != 1 && total >= t.armorClass);
    if (!hit) {
        ui::print(attempt + " but miss!");
        return 0;
    }

    int dice = rng::roll(1, player.damageDie());
    if (critical) dice += rng::roll(1, player.damageDie());
    int damage = std::max(1, dice + player.damageBonus()) * damageMultiplier;
    enemy.hp = std::max(0, enemy.hp - damage);
    ui::print(attempt + (critical ? " - CRITICAL HIT! " : " and hit! ") + t.theName(true) +
              " takes " + std::to_string(damage) + " damage.");
    return damage;
}

Outcome fight(Player& player, Enemy& enemy, bool canFlee) {
    ui::print();
    ui::divider('=');
    if (!enemy.info->art.empty()) ui::print(enemy.info->art);
    std::string intro = enemy.info->aName() + " appears!";
    intro[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(intro[0])));
    ui::print("  " + intro);
    ui::wrap(enemy.info->description, 2);
    ui::divider('=');

    while (true) {
        showStatus(player, enemy);
        int choice = ui::choose({"Attack", "Use an ability", "Use an item", "Flee"}, "Your move");

        bool acted = false;
        switch (choice) {
            case 0:
                ui::print();
                playerAttack(player, enemy);
                acted = true;
                break;
            case 1: acted = useAbility(player, enemy); break;
            case 2: acted = useItem(player); break;
            case 3:
                if (!canFlee) {
                    ui::print("There is no escape from this battle!");
                    break;
                }
                if (tryFlee(player)) return Outcome::Fled;
                acted = true;
                break;
            default: break;
        }
        if (!acted) continue;

        if (!enemy.isAlive()) {
            ui::print();
            ui::print("*** You have defeated " + enemy.info->theName() + "! ***");
            return Outcome::Victory;
        }

        enemyTurn(player, enemy);
        if (!player.isAlive()) {
            ui::print();
            ui::print("Your vision fades as you collapse to the ground...");
            return Outcome::Defeat;
        }
    }
}

}  // namespace combat
