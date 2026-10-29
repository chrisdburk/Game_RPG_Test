#include "game/Game.h"

#include <cctype>

#include "characters/Enemy.h"
#include "core/Random.h"
#include "core/UI.h"
#include "game/SaveSystem.h"
#include "items/Item.h"
#include "world/Denholm.h"
#include "world/Forest.h"
#include "world/WizardTower.h"

namespace {

constexpr std::size_t kQuestBoardSize = 3;
constexpr std::size_t kMaxNameLength = 20;

const char* kTitleArt =
    "================================================================\n"
    "                  __/\\__                  __/\\__\n"
    "                  \\    /   <>=====<>      \\    /\n"
    "                  /_  _\\                  /_  _\\\n"
    "                    \\/                      \\/\n"
    "\n"
    "          T H E   S H A C K L E D   D R A G O N\n"
    "\n"
    "              A text adventure in the land of Denholm\n"
    "================================================================";

std::string sanitizeName(const std::string& raw) {
    std::string name;
    for (char c : raw) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc) || c == ' ' || c == '-' || c == '\'') name += c;
        if (name.size() >= kMaxNameLength) break;
    }
    while (!name.empty() && name.back() == ' ') name.pop_back();
    while (!name.empty() && name.front() == ' ') name.erase(name.begin());
    return name;
}

std::string statusLine(const Player& p) {
    return p.name() + "  Lv " + std::to_string(p.level()) + "  HP " + std::to_string(p.hp()) +
           "/" + std::to_string(p.maxHp()) + "  MP " + std::to_string(p.mana()) + "/" +
           std::to_string(p.maxMana()) + "  Gold " + std::to_string(p.gold());
}

}  // namespace

Game::Game() {
    // Register every area here. Travel between them is defined by Area::connections().
    registerArea(std::make_unique<Denholm>());
    registerArea(std::make_unique<Forest>());
    registerArea(std::make_unique<WizardTower>());
}

void Game::run() {
    while (true) {
        showTitle();
        int choice = ui::choose({"New Game", "Load Game", "How to Play", "Quit"});
        switch (choice) {
            case 0:
                if (newGame()) playLoop();
                break;
            case 1:
                if (loadGame()) playLoop();
                break;
            case 2: showHelp(); break;
            default: ui::print("\nFarewell, adventurer. The kingdom awaits your return."); return;
        }
    }
}

void Game::showTitle() const {
    ui::print();
    ui::print(kTitleArt);
}

void Game::showHelp() const {
    ui::header("How to Play");
    ui::wrap(
        "An evil wizard has enthralled Seraphine, the dragon who protects the kingdom, and "
        "turned her against the land. Journey from the town of Denholm into the Whisperwood, "
        "find the Wizard's Tower, defeat the guardians on each of its five floors, and face "
        "the Wizard on the roof to set the dragon free.\n"
        "\n"
        "Every screen shows numbered choices: type the number and press Enter.\n"
        "\n"
        "DENHOLM: rest at the inn to recover HP and MP (and save your game), buy potions and "
        "gear, and take side quests from the Guild Hall for gold and experience.\n"
        "THE FOREST: explore to fight monsters, find treasure and herbs, and eventually "
        "discover the tower.\n"
        "COMBAT: attacks roll a d20 plus your attack bonus against the enemy's Armor Class (AC). "
        "A natural 20 is a critical hit. Abilities cost mana (MP). Fleeing uses a Dexterity "
        "check.\n"
        "LEVELING: earn XP from battles and quests. Each level raises your HP and MP, and lets "
        "you raise one stat by 1.\n"
        "DEFEAT: you'll wake up in Denholm having lost half your gold.");
    ui::pause();
}

bool Game::newGame() {
    ui::header("A New Hero");
    std::string name;
    while (name.empty()) {
        name = sanitizeName(ui::readLine("What is your name, hero?"));
        if (name.empty()) ui::print("  Please enter a name using letters or numbers.");
    }

    const auto& allClasses = classes::all();
    std::vector<std::string> options;
    for (const auto& cls : allClasses) options.push_back(cls->name() + " - " + cls->description());
    ui::print();
    ui::print("Choose your class:");
    const CharacterClass& cls = *allClasses[static_cast<std::size_t>(ui::choose(options))];

    ui::print();
    ui::print("Starting stats for the " + cls.name() + ":");
    Stats stats = cls.baseStats();
    for (Stat s : kAllStats) {
        ui::print("  " + statShortName(s) + ": " + std::to_string(stats.get(s)));
    }
    if (!ui::confirm("Begin your adventure as " + name + " the " + cls.name() + "?")) {
        return false;
    }

    player_ = std::make_unique<Player>(name, cls);
    currentArea_ = kStartingArea;
    refreshQuestBoard();

    ui::header("Prologue");
    ui::wrap(
        "For three hundred years the golden dragon Seraphine watched over the kingdom. Then the "
        "wizard Malgrath came. With chains of dark magic he bound her mind to his will, and now "
        "she burns the very villages she once protected.\n"
        "\n"
        "From his tower, hidden somewhere in the Whisperwood, the Wizard plots to rule the "
        "land. The king's knights rode out and did not return.\n"
        "\n"
        "You, " +
        name +
        ", have arrived in the town of Denholm with a shortsword, a couple of potions and a "
        "head full of courage. It will have to be enough.");
    ui::pause();
    return true;
}

bool Game::loadGame() {
    if (!SaveSystem::exists(kSavePath)) {
        ui::print("\nNo saved game found.");
        ui::pause();
        return false;
    }
    std::string areaId;
    std::string error;
    std::unique_ptr<Player> loaded = SaveSystem::load(kSavePath, areaId, error);
    if (!loaded) {
        ui::print("\nCould not load the saved game: " + error);
        ui::pause();
        return false;
    }
    player_ = std::move(loaded);
    currentArea_ = findArea(areaId) ? areaId : kStartingArea;
    refreshQuestBoard();
    ui::print("\nWelcome back, " + player_->name() + "!");
    return true;
}

bool Game::saveGame() {
    if (SaveSystem::save(*player_, currentArea_, kSavePath)) {
        ui::print("Game saved.");
        return true;
    }
    ui::print("Could not write the save file.");
    return false;
}

void Game::playLoop() {
    quitting_ = false;
    while (!quitting_) {
        Area* area = findArea(currentArea_);
        if (!area) {
            currentArea_ = kStartingArea;
            area = findArea(currentArea_);
        }
        leavingArea_ = false;
        ui::header(area->name());
        ui::wrap(area->description(*player_));
        area->enter(*this);
    }
}

void Game::registerArea(std::unique_ptr<Area> area) {
    std::string id = area->id();
    areas_[id] = std::move(area);
}

Area* Game::findArea(const std::string& id) const {
    auto it = areas_.find(id);
    return it == areas_.end() ? nullptr : it->second.get();
}

void Game::travelTo(const std::string& areaId) {
    currentArea_ = areaId;
    leavingArea_ = true;
}

void Game::showAreaMenu(const Area& area, std::vector<MenuAction> actions) {
    ui::print();
    ui::divider();
    ui::print(" " + area.name() + "  |  " + statusLine(*player_));
    ui::divider();

    actions.push_back({"Character sheet", [this] { showCharacterSheet(); }});
    actions.push_back({"Inventory", [this] { inventoryMenu(); }});
    actions.push_back({"Quest log", [this] { showQuestLog(); }});
    actions.push_back({"Travel", [this, &area] { travelMenu(area); }});
    actions.push_back({"Quit to title screen", [this] {
                           if (ui::confirm("Quit to the title screen? Unsaved progress is lost.")) {
                               quitting_ = true;
                           }
                       }});

    std::vector<std::string> labels;
    for (const MenuAction& action : actions) labels.push_back(action.label);
    int choice = ui::choose(labels, "What will you do?");
    actions[static_cast<std::size_t>(choice)].run();
}

void Game::travelMenu(const Area& from) {
    std::vector<Area*> destinations;
    std::vector<std::string> labels;
    for (const std::string& id : from.connections()) {
        Area* area = findArea(id);
        if (!area || !area->isUnlocked(*player_)) continue;
        destinations.push_back(area);
        labels.push_back(area->name() + " - " + area->summary());
    }
    if (destinations.empty()) {
        ui::print("There is nowhere to travel from here.");
        return;
    }
    ui::print();
    int choice = ui::chooseOrBack(labels, "Where will you go?");
    if (choice >= 0) travelTo(destinations[static_cast<std::size_t>(choice)]->id());
}

combat::Outcome Game::battle(const EnemyTemplate& enemyTemplate, bool canFlee) {
    Enemy enemy(enemyTemplate);
    combat::Outcome outcome = combat::fight(*player_, enemy, canFlee);
    if (outcome == combat::Outcome::Victory) rewardVictory(enemyTemplate);
    if (outcome == combat::Outcome::Defeat) handleDefeat();
    ui::pause();
    return outcome;
}

void Game::rewardVictory(const EnemyTemplate& enemy) {
    int gold = rng::range(enemy.goldMin, enemy.goldMax);
    if (gold > 0) {
        player_->addGold(gold);
        ui::print("You find " + std::to_string(gold) + " gold.");
    }
    for (const LootDrop& drop : enemy.loot) {
        if (rng::chance(drop.chance)) giveItem(drop.itemId);
    }
    for (const std::string& message : player_->quests().recordKill(enemy.id)) ui::print(message);
    awardXp(enemy.xp);
}

void Game::handleDefeat() {
    ui::header("Defeat");
    ui::wrap(
        "Darkness takes you... You awaken in a soft bed at the Sleeping Griffin Inn. A passing "
        "patrol found you and carried you back to Denholm, but your coin purse is lighter "
        "than you remember.");
    int lost = player_->gold() / 2;
    player_->loseGold(lost);
    if (lost > 0) ui::print("You lost " + std::to_string(lost) + " gold.");
    player_->fullRestore();
    travelTo(kStartingArea);
}

void Game::awardXp(int amount) {
    player_->addXp(amount);
    ui::print("You gain " + std::to_string(amount) + " XP.");

    while (player_->canLevelUp()) {
        ui::header("LEVEL UP!");
        ui::print(player_->name() + " has reached level " + std::to_string(player_->level() + 1) +
                  "!");
        ui::print("Choose a stat to increase by 1:");

        std::vector<Stat> choices;
        std::vector<std::string> labels;
        const Stats& stats = player_->stats();
        for (Stat s : kAllStats) {
            if (stats.get(s) >= Stats::kMaxScore) continue;
            choices.push_back(s);
            labels.push_back(ui::padRight(statLongName(s), 14) + std::to_string(stats.get(s)) +
                             " (" + ui::signedNumber(stats.modifier(s)) + ")");
        }
        Stat chosen = choices.empty() ? Stat::Con
                                      : choices[static_cast<std::size_t>(ui::choose(labels))];
        LevelUpResult gains = player_->levelUp(chosen);
        ui::print(statLongName(chosen) + " is now " + std::to_string(player_->stats().get(chosen)) +
                  ". Max HP +" + std::to_string(gains.hpGained) + ", Max MP +" +
                  std::to_string(gains.manaGained) + ".");
        ui::print("You feel completely refreshed.");
    }
}

void Game::giveItem(const std::string& itemId, int count) {
    const Item* item = items::find(itemId);
    if (!item || count <= 0) return;
    player_->inventory().add(itemId, count);
    ui::print("You obtain " + std::to_string(count) + " x " + item->name + ".");
    for (const std::string& message :
         player_->quests().gatherUpdates(itemId, player_->inventory())) {
        ui::print(message);
    }
}

void Game::refreshQuestBoard() {
    questBoard_ = questboard::generate(kQuestBoardSize, player_->level());
}

void Game::completeQuest(std::size_t index) {
    QuestLog& log = player_->quests();
    if (index >= log.active().size()) return;
    const Quest& pending = log.active()[index];
    if (!pending.isComplete(player_->inventory())) return;

    if (pending.type == QuestType::Gather) {
        player_->inventory().remove(pending.target, pending.required);
    }
    Quest quest = log.remove(index);
    log.recordCompletion();

    ui::print();
    ui::print("*** Quest complete: " + quest.name + " ***");
    if (quest.reward.gold > 0) {
        player_->addGold(quest.reward.gold);
        ui::print("You receive " + std::to_string(quest.reward.gold) + " gold.");
    }
    if (!quest.reward.itemId.empty()) giveItem(quest.reward.itemId, quest.reward.itemCount);
    if (quest.reward.xp > 0) awardXp(quest.reward.xp);
}

void Game::showCharacterSheet() {
    const Player& p = *player_;
    const Stats& stats = p.stats();
    ui::header(p.name() + " the " + p.characterClass().name());
    ui::print("  Level " + std::to_string(p.level()) + "      XP " + std::to_string(p.xp()) + "/" +
              std::to_string(p.xpToNextLevel()));
    ui::print("  HP " + ui::bar(p.hp(), p.maxHp()) + " " + std::to_string(p.hp()) + "/" +
              std::to_string(p.maxHp()));
    ui::print("  MP " + ui::bar(p.mana(), p.maxMana()) + " " + std::to_string(p.mana()) + "/" +
              std::to_string(p.maxMana()));
    ui::print("  Armor Class " + std::to_string(p.armorClass()) + "   Attack " +
              ui::signedNumber(p.attackBonus()) + "   Damage 1d" + std::to_string(p.damageDie()) +
              ui::signedNumber(p.damageBonus()));
    ui::print("  Gold " + std::to_string(p.gold()));
    ui::print();
    ui::print("  -- Stats --");
    std::string row;
    int column = 0;
    for (Stat s : kAllStats) {
        std::string value = std::to_string(stats.get(s));
        if (value.size() < 2) value = " " + value;
        row += ui::padRight(statShortName(s) + " " + value + " (" +
                                ui::signedNumber(stats.modifier(s)) + ")",
                            18);
        if (++column % 3 == 0) {
            ui::print("  " + row);
            row.clear();
        }
    }
    ui::print();
    ui::print("  -- Equipment --");
    const Item* weapon = items::find(p.weaponId());
    const Item* armor = items::find(p.armorId());
    ui::print("  Weapon: " + p.weaponName() + (weapon ? " (" + weapon->summary() + ")" : ""));
    ui::print("  Armor:  " + p.armorName() + (armor ? " (" + armor->summary() + ")" : ""));
    ui::print();
    ui::print("  -- Abilities --");
    for (const Ability& ability : p.characterClass().abilities()) {
        ui::wrap(ability.name + " (" + std::to_string(ability.manaCost) + " MP): " +
                     ability.description,
                 2);
    }
    ui::print();
    ui::print("  Quests completed: " + std::to_string(p.quests().completedCount()));
    ui::pause();
}

void Game::inventoryMenu() {
    Player& p = *player_;
    while (true) {
        ui::header("Inventory");
        ui::print("  Weapon: " + p.weaponName() + "    Armor: " + p.armorName() +
                  "    Gold: " + std::to_string(p.gold()));
        ui::print();
        if (p.inventory().empty()) {
            ui::print("  Your pack is empty.");
            ui::pause();
            return;
        }

        std::vector<std::string> ids;
        std::vector<std::string> labels;
        for (const auto& [id, count] : p.inventory().entries()) {
            const Item* item = items::find(id);
            if (!item) continue;
            ids.push_back(id);
            labels.push_back(ui::padRight(item->name + " x" + std::to_string(count), 28) +
                             item->summary());
        }
        int choice = ui::chooseOrBack(labels, "Select an item");
        if (choice < 0) return;

        const Item& item = items::get(ids[static_cast<std::size_t>(choice)]);
        ui::print();
        ui::wrap(item.name + " (" + item.summary() + "): " + item.description);
        switch (item.type) {
            case ItemType::Consumable:
                if (ui::confirm("Use it now?")) ui::print(p.useConsumable(item.id).message);
                break;
            case ItemType::Weapon:
            case ItemType::Armor:
                if (ui::confirm("Equip it?") && p.equip(item.id)) {
                    ui::print("You equip the " + item.name + ".");
                }
                break;
            case ItemType::Material:
                ui::print("This can be sold at a shop or delivered for a quest.");
                ui::pause();
                break;
        }
    }
}

void Game::showQuestLog() {
    const Player& p = *player_;
    const QuestLog& log = p.quests();
    ui::header("Quest Log");
    if (log.active().empty()) {
        ui::wrap("You have no active quests. Visit the Guild Hall in Denholm to find work.", 2);
    }
    for (std::size_t i = 0; i < log.active().size(); ++i) {
        const Quest& quest = log.active()[i];
        bool done = quest.isComplete(p.inventory());
        ui::print("  " + std::to_string(i + 1) + ") " + quest.name +
                  (done ? "  [COMPLETE - turn in at the Guild Hall]" : ""));
        ui::wrap(quest.description, 5);
        ui::print("     Objective: " + quest.objective() + "  (" +
                  quest.progressText(p.inventory()) + ")");
        ui::print("     Reward:    " + quest.reward.describe());
        ui::print();
    }
    ui::print("  Active quests: " + std::to_string(log.active().size()) + "/" +
              std::to_string(QuestLog::kMaxActive) +
              "    Completed: " + std::to_string(log.completedCount()));
    ui::pause();
}
