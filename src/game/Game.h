#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "characters/Player.h"
#include "combat/Combat.h"
#include "quests/Quest.h"
#include "world/Area.h"

struct EnemyTemplate;

// One entry in an area's menu.
struct MenuAction {
    std::string label;
    std::function<void()> run;
};

// Owns the player and the world, and runs the main loop.
class Game {
public:
    static constexpr const char* kSavePath = "savegame.txt";
    static constexpr const char* kStartingArea = "denholm";

    Game();
    void run();  // title screen; returns when the player quits

    Player& player() { return *player_; }

    // --- World --------------------------------------------------------------
    void registerArea(std::unique_ptr<Area> area);
    Area* findArea(const std::string& id) const;
    void travelTo(const std::string& areaId);
    // True once the current area should hand control back (travel, defeat or quit).
    bool shouldLeaveArea() const { return leavingArea_ || quitting_; }
    // Shows the area's own actions plus the common ones (character, inventory,
    // quests, travel, quit) and runs the one the player picks.
    void showAreaMenu(const Area& area, std::vector<MenuAction> actions);

    // --- Rewards & battles -------------------------------------------------
    combat::Outcome battle(const EnemyTemplate& enemy, bool canFlee = true);
    void awardXp(int amount);
    void giveItem(const std::string& itemId, int count = 1);

    // --- Quests -------------------------------------------------------------
    std::vector<Quest>& questBoard() { return questBoard_; }
    void refreshQuestBoard();
    void completeQuest(std::size_t index);

    // --- Saving & shared screens ------------------------------------------
    bool saveGame();
    void showCharacterSheet();
    void inventoryMenu();
    void showQuestLog();

private:
    void showTitle() const;
    void showHelp() const;
    bool newGame();
    bool loadGame();
    void playLoop();
    void travelMenu(const Area& from);
    void rewardVictory(const EnemyTemplate& enemy);
    void handleDefeat();

    std::map<std::string, std::unique_ptr<Area>> areas_;
    std::unique_ptr<Player> player_;
    std::vector<Quest> questBoard_;
    std::string currentArea_ = kStartingArea;
    bool leavingArea_ = false;
    bool quitting_ = false;
};
