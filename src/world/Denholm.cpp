#include "world/Denholm.h"

#include "core/UI.h"
#include "game/Game.h"
#include "items/Item.h"
#include "world/Flags.h"

namespace {

constexpr int kInnPrice = 10;

const std::vector<std::string> kProvisionsStock = {"health_potion", "greater_health_potion",
                                                   "mana_potion"};
const std::vector<std::string> kSmithyStock = {"longsword",     "battleaxe",   "greatsword",
                                               "leather_armor", "chain_shirt", "plate_armor"};

void inn(Game& game) {
    Player& p = game.player();
    while (true) {
        ui::header("The Sleeping Griffin Inn");
        ui::wrap(
            "A fire crackles in the hearth and the smell of stew fills the room. Innkeeper "
            "Tomas polishes a mug and nods at you.");
        int choice = ui::chooseOrBack(
            {"Rest for the night (" + std::to_string(kInnPrice) + " gold)", "Save your game"},
            "What will you do?");
        if (choice < 0) return;

        if (choice == 0) {
            if (p.spendGold(kInnPrice)) {
                ui::wrap("You pay for a room and sleep soundly through the night.");
            } else {
                ui::wrap(
                    "You can't afford a room, but Tomas lets you sleep by the fire anyway. "
                    "\"Pay me back when you're a hero.\"");
            }
            p.fullRestore();
            game.refreshQuestBoard();
            ui::print("HP and MP fully restored. New quests are posted at the Guild Hall.");
        } else {
            game.saveGame();
        }
    }
}

void buy(Game& game, const std::vector<std::string>& stock) {
    Player& p = game.player();
    while (true) {
        ui::print();
        ui::print("You have " + std::to_string(p.gold()) + " gold.");
        std::vector<std::string> labels;
        for (const std::string& id : stock) {
            const Item& item = items::get(id);
            labels.push_back(ui::padRight(item.name, 24) +
                             ui::padRight(std::to_string(item.price) + "g", 7) + item.summary());
        }
        int choice = ui::chooseOrBack(labels, "Buy what?");
        if (choice < 0) return;

        const Item& item = items::get(stock[static_cast<std::size_t>(choice)]);
        int affordable = p.gold() / item.price;
        if (affordable == 0) {
            ui::print("You can't afford that.");
            continue;
        }
        int quantity = 1;
        if (item.type == ItemType::Consumable) {
            quantity = ui::readNumber("How many? (0-" + std::to_string(affordable) + ")", 0,
                                      affordable);
            if (quantity == 0) continue;
        }
        p.spendGold(item.price * quantity);
        game.giveItem(item.id, quantity);

        bool isGear = item.type == ItemType::Weapon || item.type == ItemType::Armor;
        if (isGear && ui::confirm("Equip the " + item.name + " now?") && p.equip(item.id)) {
            ui::print("You equip the " + item.name + ".");
        }
    }
}

void sell(Game& game) {
    Player& p = game.player();
    while (true) {
        std::vector<std::string> ids;
        std::vector<std::string> labels;
        for (const auto& [id, count] : p.inventory().entries()) {
            const Item* item = items::find(id);
            if (!item) continue;
            ids.push_back(id);
            labels.push_back(ui::padRight(item->name + " x" + std::to_string(count), 28) +
                             std::to_string(item->sellPrice()) + "g each");
        }
        ui::print();
        if (ids.empty()) {
            ui::print("You have nothing to sell. (Equipped gear can't be sold.)");
            return;
        }
        ui::print("You have " + std::to_string(p.gold()) + " gold.");
        int choice = ui::chooseOrBack(labels, "Sell what?");
        if (choice < 0) return;

        const Item& item = items::get(ids[static_cast<std::size_t>(choice)]);
        int have = p.inventory().count(item.id);
        int quantity = have == 1
                           ? 1
                           : ui::readNumber("How many? (0-" + std::to_string(have) + ")", 0, have);
        if (quantity == 0) continue;
        p.inventory().remove(item.id, quantity);
        p.addGold(item.sellPrice() * quantity);
        ui::print("You sell " + std::to_string(quantity) + " x " + item.name + " for " +
                  std::to_string(item.sellPrice() * quantity) + " gold.");
    }
}

void shop(Game& game, const std::string& title, const std::string& greeting,
          const std::vector<std::string>& stock) {
    while (true) {
        ui::header(title);
        ui::wrap(greeting);
        int choice = ui::chooseOrBack({"Buy", "Sell"}, "What will you do?");
        if (choice < 0) return;
        if (choice == 0) buy(game, stock);
        else sell(game);
    }
}

void browseQuestBoard(Game& game) {
    std::vector<Quest>& board = game.questBoard();
    QuestLog& log = game.player().quests();
    while (true) {
        ui::print();
        if (board.empty()) {
            ui::wrap(
                "The quest board is bare. New notices go up every morning - rest at the inn "
                "and check back.");
            return;
        }
        std::vector<std::string> labels;
        for (const Quest& quest : board) labels.push_back(quest.name + " - " + quest.objective());
        int choice = ui::chooseOrBack(labels, "Read which notice?");
        if (choice < 0) return;

        std::size_t index = static_cast<std::size_t>(choice);
        const Quest& quest = board[index];
        ui::header(quest.name);
        ui::wrap(quest.description, 2);
        ui::print();
        ui::print("  Objective: " + quest.objective());
        ui::print("  Reward:    " + quest.reward.describe());
        ui::print();
        if (!ui::confirm("Accept this quest?")) continue;
        if (log.isFull()) {
            ui::print("You can only take on " + std::to_string(QuestLog::kMaxActive) +
                      " quests at a time. Finish one first.");
            continue;
        }
        log.add(quest);
        board.erase(board.begin() + static_cast<std::ptrdiff_t>(index));
        ui::print("Quest accepted! Check your progress in the quest log.");
    }
}

void turnInQuests(Game& game) {
    Player& p = game.player();
    bool any = false;
    // Iterate backwards so removing a finished quest doesn't skip the next one.
    for (std::size_t i = p.quests().active().size(); i-- > 0;) {
        if (p.quests().active()[i].isComplete(p.inventory())) {
            game.completeQuest(i);
            any = true;
        }
    }
    if (!any) ui::print("\nYou have no completed quests to turn in.");
}

void guildHall(Game& game) {
    while (true) {
        ui::header("The Adventurers' Guild Hall");
        ui::wrap(
            "Maps and bounty notices cover every wall. Guildmaster Orla looks up from her "
            "ledger. \"Looking for work? The board's over there.\"");
        int choice = ui::chooseOrBack(
            {"Browse the quest board", "Turn in completed quests", "Review your quest log"},
            "What will you do?");
        if (choice < 0) return;
        if (choice == 0) browseQuestBoard(game);
        if (choice == 1) turnInQuests(game);
        if (choice == 2) game.showQuestLog();
    }
}

}  // namespace

std::string Denholm::description(const Player& player) const {
    if (player.flag(flags::kGameWon)) {
        return "Banners fly over Denholm and children chase each other through the square. "
               "Above the rooftops, the golden dragon Seraphine circles in the sunlight, free "
               "once more. People cheer as you pass.";
    }
    return "A walled market town of timber houses and cobbled streets. Smoke from the "
           "blacksmith's forge drifts over the square, and townsfolk glance nervously at the "
           "sky, watching for dragon fire.";
}

void Denholm::enter(Game& game) {
    while (!game.shouldLeaveArea()) {
        game.showAreaMenu(
            *this,
            {
                {"Sleeping Griffin Inn (rest & save)", [&game] { inn(game); }},
                {"Hilda's Provisions (potions)",
                 [&game] {
                     shop(game, "Hilda's Provisions",
                          "Shelves of bottles glint in the lamplight. \"Potions, remedies, "
                          "tonics!\" Hilda chirps.",
                          kProvisionsStock);
                 }},
                {"Iron Anvil Smithy (weapons & armor)",
                 [&game] {
                     shop(game, "The Iron Anvil Smithy",
                          "Heat rolls from the forge. Borin the smith wipes his brow. \"Need "
                          "something sharper? Or thicker?\"",
                          kSmithyStock);
                 }},
                {"Adventurers' Guild Hall (quests)", [&game] { guildHall(game); }},
            });
    }
}
