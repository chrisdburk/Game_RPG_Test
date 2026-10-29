#include "characters/Player.h"

#include <algorithm>
#include <utility>

#include "core/Random.h"
#include "items/Item.h"

Player::Player(std::string name, const CharacterClass& characterClass)
    : name_(std::move(name)),
      class_(&characterClass),
      stats_(characterClass.baseStats()),
      gold_(characterClass.startingGold()),
      weaponId_(characterClass.startingWeapon()),
      armorId_(characterClass.startingArmor()) {
    maxHp_ = std::max(1, class_->baseHealth() + stats_.modifier(Stat::Con));
    maxMana_ = std::max(0, class_->baseMana() + stats_.modifier(class_->manaStat()));
    fullRestore();
    for (const StartingItem& item : class_->startingItems()) {
        inventory_.add(item.itemId, item.count);
    }
}

int Player::attackBonus() const {
    return proficiency() + stats_.modifier(class_->attackStat());
}

int Player::armorClass() const {
    int ac = 10 + stats_.modifier(Stat::Dex);
    if (const Item* armor = items::find(armorId_)) ac += armor->armorBonus;
    return ac;
}

int Player::damageDie() const { return weaponId_.empty() ? 4 : 6; }

int Player::damageBonus() const {
    int bonus = stats_.modifier(class_->attackStat());
    if (const Item* weapon = items::find(weaponId_)) bonus += weapon->damageBonus;
    return bonus;
}

std::string Player::weaponName() const {
    const Item* weapon = items::find(weaponId_);
    return weapon ? weapon->name : "Bare Fists";
}

std::string Player::armorName() const {
    const Item* armor = items::find(armorId_);
    return armor ? armor->name : "None";
}

int Player::heal(int amount) {
    int before = hp_;
    hp_ = std::min(maxHp_, hp_ + std::max(0, amount));
    return hp_ - before;
}

int Player::restoreMana(int amount) {
    int before = mana_;
    mana_ = std::min(maxMana_, mana_ + std::max(0, amount));
    return mana_ - before;
}

void Player::takeDamage(int amount) { hp_ = std::max(0, hp_ - std::max(0, amount)); }

bool Player::spendMana(int amount) {
    if (mana_ < amount) return false;
    mana_ -= amount;
    return true;
}

void Player::fullRestore() {
    hp_ = maxHp_;
    mana_ = maxMana_;
}

void Player::addGold(int amount) { gold_ += std::max(0, amount); }

bool Player::spendGold(int amount) {
    if (gold_ < amount) return false;
    gold_ -= amount;
    return true;
}

void Player::loseGold(int amount) { gold_ = std::max(0, gold_ - amount); }

LevelUpResult Player::levelUp(Stat improvedStat) {
    LevelUpResult result;
    if (!canLevelUp()) return result;

    xp_ -= xpToNextLevel();
    ++level_;
    stats_.increase(improvedStat);

    result.hpGained = std::max(1, class_->healthPerLevel() + stats_.modifier(Stat::Con));
    result.manaGained =
        std::max(1, class_->manaPerLevel() + std::max(0, stats_.modifier(class_->manaStat())));
    maxHp_ += result.hpGained;
    maxMana_ += result.manaGained;
    fullRestore();
    return result;
}

bool Player::equip(const std::string& itemId) {
    const Item* item = items::find(itemId);
    if (!item) return false;

    std::string* slot = nullptr;
    if (item->type == ItemType::Weapon) slot = &weaponId_;
    if (item->type == ItemType::Armor) slot = &armorId_;
    if (!slot || !inventory_.remove(itemId)) return false;

    if (!slot->empty()) inventory_.add(*slot);
    *slot = itemId;
    return true;
}

ItemUseResult Player::useConsumable(const std::string& itemId) {
    const Item* item = items::find(itemId);
    if (!item || item->type != ItemType::Consumable || inventory_.count(itemId) == 0) {
        return {false, "You can't use that."};
    }
    if (item->effect == ItemEffect::Heal && hp_ == maxHp_) {
        return {false, "You are already at full health."};
    }
    if (item->effect == ItemEffect::RestoreMana && mana_ == maxMana_) {
        return {false, "Your mana is already full."};
    }

    inventory_.remove(itemId);
    int roll = rng::range(item->effectMin, item->effectMax);
    switch (item->effect) {
        case ItemEffect::Heal: {
            int healed = heal(roll);
            return {true, "You drink the " + item->name + " (rolled " + std::to_string(roll) +
                              ") and recover " + std::to_string(healed) + " HP."};
        }
        case ItemEffect::RestoreMana: {
            int restored = restoreMana(roll);
            return {true, "You drink the " + item->name + " (rolled " + std::to_string(roll) +
                              ") and recover " + std::to_string(restored) + " MP."};
        }
        case ItemEffect::None: break;
    }
    return {true, "You use the " + item->name + ". Nothing happens."};
}

int Player::flag(const std::string& key) const {
    auto it = flags_.find(key);
    return it == flags_.end() ? 0 : it->second;
}
