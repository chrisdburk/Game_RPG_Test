#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "items/Inventory.h"

enum class QuestType {
    Slay,    // defeat `required` enemies with id `target`
    Gather,  // deliver `required` items with id `target`
};

struct QuestReward {
    int gold = 0;
    int xp = 0;
    std::string itemId;  // optional
    int itemCount = 0;

    std::string describe() const;  // "60 gold, 75 XP, 1 x Health Potion"
};

struct Quest {
    std::string name;
    std::string description;
    QuestType type = QuestType::Slay;
    std::string target;        // enemy id or item id
    std::string targetPlural;  // e.g. "Gray Wolves"
    int required = 1;
    int progress = 0;  // kills so far (Slay only; Gather counts the inventory)
    QuestReward reward;

    int currentProgress(const Inventory& inventory) const;
    bool isComplete(const Inventory& inventory) const;
    std::string objective() const;                               // "Slay 3 Gray Wolves"
    std::string progressText(const Inventory& inventory) const;  // "2/3"
};

// The player's accepted quests.
class QuestLog {
public:
    static constexpr std::size_t kMaxActive = 3;

    const std::vector<Quest>& active() const { return active_; }
    bool isFull() const { return active_.size() >= kMaxActive; }
    void add(Quest quest) { active_.push_back(std::move(quest)); }
    Quest remove(std::size_t index);

    // Updates Slay quests and returns progress messages for the ones that advanced.
    std::vector<std::string> recordKill(const std::string& enemyId);
    // Progress messages for Gather quests that want `itemId`.
    std::vector<std::string> gatherUpdates(const std::string& itemId,
                                           const Inventory& inventory) const;

    int completedCount() const { return completed_; }
    void setCompletedCount(int count) { completed_ = count; }
    void recordCompletion() { ++completed_; }

private:
    std::vector<Quest> active_;
    int completed_ = 0;
};

namespace questboard {

// Creates up to `count` random side quests suited to the player's level.
// Add new quest ideas to the template list in Quest.cpp.
std::vector<Quest> generate(std::size_t count, int playerLevel);

}  // namespace questboard
