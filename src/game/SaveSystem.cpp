#include "game/SaveSystem.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "characters/CharacterClass.h"
#include "characters/Player.h"
#include "items/Item.h"

namespace {

constexpr int kSaveVersion = 1;

// Splits on `sep`, keeping empty fields.
std::vector<std::string> split(const std::string& text, char sep) {
    std::vector<std::string> parts;
    std::string part;
    std::istringstream stream(text);
    while (std::getline(stream, part, sep)) parts.push_back(part);
    if (!text.empty() && text.back() == sep) parts.emplace_back();
    return parts;
}

// "key:value" where the value is an integer.
bool parsePair(const std::string& text, std::string& key, int& value) {
    std::size_t colon = text.rfind(':');
    if (colon == std::string::npos) return false;
    key = text.substr(0, colon);
    value = std::stoi(text.substr(colon + 1));
    return !key.empty();
}

std::string serializeQuest(const Quest& q) {
    std::ostringstream out;
    out << (q.type == QuestType::Slay ? "slay" : "gather") << '\t' << q.target << '\t'
        << q.targetPlural << '\t' << q.required << '\t' << q.progress << '\t' << q.reward.gold
        << '\t' << q.reward.xp << '\t' << q.reward.itemId << '\t' << q.reward.itemCount << '\t'
        << q.name << '\t' << q.description;
    return out.str();
}

Quest parseQuest(const std::string& text) {
    std::vector<std::string> f = split(text, '\t');
    if (f.size() != 11) throw std::runtime_error("bad quest entry");
    Quest q;
    q.type = f[0] == "gather" ? QuestType::Gather : QuestType::Slay;
    q.target = f[1];
    q.targetPlural = f[2];
    q.required = std::stoi(f[3]);
    q.progress = std::stoi(f[4]);
    q.reward.gold = std::stoi(f[5]);
    q.reward.xp = std::stoi(f[6]);
    q.reward.itemId = f[7];
    q.reward.itemCount = std::stoi(f[8]);
    q.name = f[9];
    q.description = f[10];
    return q;
}

}  // namespace

bool SaveSystem::exists(const std::string& path) {
    std::ifstream in(path);
    return in.good();
}

bool SaveSystem::save(const Player& p, const std::string& areaId, const std::string& path) {
    std::ofstream out(path);
    if (!out) return false;

    out << "version=" << kSaveVersion << '\n'
        << "name=" << p.name_ << '\n'
        << "class=" << p.class_->name() << '\n'
        << "area=" << areaId << '\n'
        << "level=" << p.level_ << '\n'
        << "xp=" << p.xp_ << '\n'
        << "hp=" << p.hp_ << '\n'
        << "maxhp=" << p.maxHp_ << '\n'
        << "mana=" << p.mana_ << '\n'
        << "maxmana=" << p.maxMana_ << '\n'
        << "gold=" << p.gold_ << '\n'
        << "weapon=" << p.weaponId_ << '\n'
        << "armor=" << p.armorId_ << '\n';
    for (Stat s : kAllStats) out << "stat." << statShortName(s) << '=' << p.stats_.get(s) << '\n';
    for (const auto& [id, count] : p.inventory_.entries()) out << "item=" << id << ':' << count << '\n';
    for (const auto& [key, value] : p.flags_) out << "flag=" << key << ':' << value << '\n';
    out << "quests_completed=" << p.quests_.completedCount() << '\n';
    for (const Quest& q : p.quests_.active()) out << "quest=" << serializeQuest(q) << '\n';

    return static_cast<bool>(out);
}

std::unique_ptr<Player> SaveSystem::load(const std::string& path, std::string& areaId,
                                         std::string& error) {
    std::ifstream in(path);
    if (!in) {
        error = "could not open " + path;
        return nullptr;
    }

    std::map<std::string, std::string> values;
    std::vector<std::string> itemLines;
    std::vector<std::string> flagLines;
    std::vector<std::string> questLines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (key == "item") itemLines.push_back(value);
        else if (key == "flag") flagLines.push_back(value);
        else if (key == "quest") questLines.push_back(value);
        else values[key] = value;
    }

    try {
        auto number = [&](const std::string& key) { return std::stoi(values.at(key)); };

        if (number("version") != kSaveVersion) {
            error = "unsupported save version";
            return nullptr;
        }
        const CharacterClass* cls = classes::find(values.at("class"));
        if (!cls) {
            error = "unknown class '" + values.at("class") + "'";
            return nullptr;
        }

        auto player = std::make_unique<Player>(values.at("name"), *cls);
        Player& p = *player;
        p.level_ = std::max(1, number("level"));
        p.xp_ = std::max(0, number("xp"));
        p.maxHp_ = std::max(1, number("maxhp"));
        p.hp_ = std::clamp(number("hp"), 1, p.maxHp_);
        p.maxMana_ = std::max(0, number("maxmana"));
        p.mana_ = std::clamp(number("mana"), 0, p.maxMana_);
        p.gold_ = std::max(0, number("gold"));
        p.weaponId_ = items::find(values["weapon"]) ? values["weapon"] : "";
        p.armorId_ = items::find(values["armor"]) ? values["armor"] : "";
        for (Stat s : kAllStats) p.stats_.set(s, number("stat." + statShortName(s)));

        p.inventory_.clear();
        for (const std::string& entry : itemLines) {
            std::string id;
            int count = 0;
            if (parsePair(entry, id, count) && items::find(id)) p.inventory_.add(id, count);
        }
        for (const std::string& entry : flagLines) {
            std::string key;
            int value = 0;
            if (parsePair(entry, key, value)) p.flags_[key] = value;
        }
        p.quests_.setCompletedCount(values.count("quests_completed") ? number("quests_completed")
                                                                     : 0);
        for (const std::string& entry : questLines) p.quests_.add(parseQuest(entry));

        areaId = values.count("area") ? values["area"] : "";
        return player;
    } catch (const std::exception&) {
        error = "the save file is damaged";
        return nullptr;
    }
}
