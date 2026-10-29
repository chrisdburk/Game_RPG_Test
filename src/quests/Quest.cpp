#include "quests/Quest.h"

#include <algorithm>
#include <utility>

#include "characters/Enemy.h"
#include "core/Random.h"
#include "items/Item.h"

namespace {

std::string singularName(const Quest& quest) {
    if (quest.type == QuestType::Slay) {
        if (const EnemyTemplate* e = enemies::find(quest.target)) return e->name;
    } else if (const Item* item = items::find(quest.target)) {
        return item->name;
    }
    return quest.targetPlural;
}

struct QuestTemplate {
    QuestType type;
    const char* name;
    const char* description;  // "{n}" is replaced with the required count
    const char* target;
    const char* targetPlural;
    int minCount;
    int maxCount;
    int minLevel;
    int goldPer;  // reward gold per required kill/item
    int xpPer;    // reward XP per required kill/item
};

const std::vector<QuestTemplate>& templates() {
    static const std::vector<QuestTemplate> list = {
        {QuestType::Slay, "Wolf Cull",
         "Farmer Bram's sheep keep vanishing at night. Thin out the packs by slaying {n} Gray "
         "Wolves in the forest.",
         "gray_wolf", "Gray Wolves", 3, 5, 1, 8, 20},
        {QuestType::Slay, "Goblin Menace",
         "Goblins have been raiding the miller's grain stores. Drive them off by slaying {n} "
         "Goblins.",
         "goblin", "Goblins", 3, 5, 1, 8, 18},
        {QuestType::Slay, "Web of Trouble",
         "Woodcutters refuse to enter the eastern groves, now choked with webs. Slay {n} Giant "
         "Spiders.",
         "giant_spider", "Giant Spiders", 2, 4, 2, 12, 30},
        {QuestType::Slay, "Highway Robbery",
         "Merchants on the forest road are being robbed blind. Bring {n} Bandits to justice.",
         "bandit", "Bandits", 2, 4, 2, 15, 35},
        {QuestType::Slay, "Orc Raiders",
         "An orc warband has been spotted near the old logging camp. Slay {n} Orcs before they "
         "reach Denholm.",
         "orc", "Orcs", 2, 3, 3, 20, 50},
        {QuestType::Slay, "The Dire Pack",
         "Something huge has been howling in the deep woods. Hunt down {n} Dire Wolves.",
         "dire_wolf", "Dire Wolves", 1, 2, 4, 30, 70},
        {QuestType::Slay, "Troll Toll",
         "A Forest Troll has claimed the old stone bridge and demands a toll of flesh. Slay it.",
         "forest_troll", "Forest Trolls", 1, 1, 5, 60, 120},
        {QuestType::Gather, "Herbalist's Request",
         "Old Maren the herbalist needs {n} Moonpetal Herbs for her remedies. They grow in "
         "sunlit clearings deep in the forest.",
         "moonpetal_herb", "Moonpetal Herbs", 2, 4, 1, 10, 20},
        {QuestType::Gather, "Tanner's Order",
         "Gerrit the tanner has an order for winter cloaks. Bring him {n} Wolf Pelts.",
         "wolf_pelt", "Wolf Pelts", 2, 3, 1, 12, 20},
        {QuestType::Gather, "Curious Collector",
         "A traveling scholar studies goblin culture (such as it is). Deliver {n} Goblin "
         "Trinkets.",
         "goblin_trinket", "Goblin Trinkets", 2, 3, 1, 10, 18},
        {QuestType::Gather, "Weaver's Silk",
         "The weaver's guild pays well for strong thread. Collect {n} bundles of Spider Silk.",
         "spider_silk", "bundles of Spider Silk", 2, 3, 2, 15, 30},
    };
    return list;
}

std::string fillCount(std::string text, int count) {
    const std::string token = "{n}";
    for (std::size_t pos = text.find(token); pos != std::string::npos; pos = text.find(token)) {
        text.replace(pos, token.size(), std::to_string(count));
    }
    return text;
}

QuestReward makeReward(const QuestTemplate& t, int count, int playerLevel) {
    QuestReward reward;
    reward.gold = t.goldPer * count + rng::range(0, 10);
    reward.xp = t.xpPer * count;
    if (rng::chance(40)) {
        if (playerLevel >= 4 && rng::chance(50)) {
            reward.itemId = "greater_health_potion";
        } else {
            reward.itemId = rng::chance(60) ? "health_potion" : "mana_potion";
        }
        reward.itemCount = rng::range(1, 2);
    }
    return reward;
}

}  // namespace

std::string QuestReward::describe() const {
    std::string text = std::to_string(gold) + " gold, " + std::to_string(xp) + " XP";
    if (!itemId.empty() && itemCount > 0) {
        const Item* item = items::find(itemId);
        text += ", " + std::to_string(itemCount) + " x " + (item ? item->name : itemId);
    }
    return text;
}

int Quest::currentProgress(const Inventory& inventory) const {
    int have = type == QuestType::Gather ? inventory.count(target) : progress;
    return std::min(have, required);
}

bool Quest::isComplete(const Inventory& inventory) const {
    return currentProgress(inventory) >= required;
}

std::string Quest::objective() const {
    const std::string verb = type == QuestType::Slay ? "Slay " : "Gather ";
    const std::string noun = required == 1 ? singularName(*this) : targetPlural;
    return verb + std::to_string(required) + " " + noun;
}

std::string Quest::progressText(const Inventory& inventory) const {
    return std::to_string(currentProgress(inventory)) + "/" + std::to_string(required);
}

Quest QuestLog::remove(std::size_t index) {
    Quest quest = std::move(active_.at(index));
    active_.erase(active_.begin() + static_cast<std::ptrdiff_t>(index));
    return quest;
}

std::vector<std::string> QuestLog::recordKill(const std::string& enemyId) {
    std::vector<std::string> messages;
    for (Quest& quest : active_) {
        if (quest.type != QuestType::Slay || quest.target != enemyId) continue;
        if (quest.progress >= quest.required) continue;
        ++quest.progress;
        std::string message = "[Quest] " + quest.name + ": " + std::to_string(quest.progress) +
                              "/" + std::to_string(quest.required);
        if (quest.progress >= quest.required) message += " - Complete! Return to the Guild Hall.";
        messages.push_back(message);
    }
    return messages;
}

std::vector<std::string> QuestLog::gatherUpdates(const std::string& itemId,
                                                 const Inventory& inventory) const {
    std::vector<std::string> messages;
    for (const Quest& quest : active_) {
        if (quest.type != QuestType::Gather || quest.target != itemId) continue;
        std::string message = "[Quest] " + quest.name + ": " + quest.progressText(inventory);
        if (quest.isComplete(inventory)) message += " - Ready to turn in at the Guild Hall.";
        messages.push_back(message);
    }
    return messages;
}

namespace questboard {

std::vector<Quest> generate(std::size_t count, int playerLevel) {
    std::vector<const QuestTemplate*> eligible;
    for (const QuestTemplate& t : templates()) {
        if (t.minLevel <= playerLevel) eligible.push_back(&t);
    }
    // Shuffle so each board is different.
    for (std::size_t i = eligible.size(); i > 1; --i) {
        std::size_t j = static_cast<std::size_t>(rng::range(0, static_cast<int>(i) - 1));
        std::swap(eligible[i - 1], eligible[j]);
    }

    std::vector<Quest> quests;
    for (std::size_t i = 0; i < eligible.size() && quests.size() < count; ++i) {
        const QuestTemplate& t = *eligible[i];
        int required = rng::range(t.minCount, t.maxCount);
        Quest quest;
        quest.name = t.name;
        quest.description = fillCount(t.description, required);
        quest.type = t.type;
        quest.target = t.target;
        quest.targetPlural = t.targetPlural;
        quest.required = required;
        quest.reward = makeReward(t, required, playerLevel);
        quests.push_back(quest);
    }
    return quests;
}

}  // namespace questboard
