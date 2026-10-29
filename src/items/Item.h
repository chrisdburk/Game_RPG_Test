#pragma once

#include <string>
#include <vector>

enum class ItemType { Weapon, Armor, Consumable, Material };
enum class ItemEffect { None, Heal, RestoreMana };

// Static description of an item. Players hold item ids (see Inventory);
// the details are looked up here. Add new items in Item.cpp.
struct Item {
    std::string id;
    std::string name;
    std::string description;
    ItemType type = ItemType::Material;
    int price = 0;  // purchase price in gold; items sell for half

    int damageBonus = 0;  // weapons
    int armorBonus = 0;   // armor

    // Consumables restore a random amount in [effectMin, effectMax].
    ItemEffect effect = ItemEffect::None;
    int effectMin = 0;
    int effectMax = 0;

    int sellPrice() const { return price > 1 ? price / 2 : price; }
    // Short stat line, e.g. "+5 damage" or "Heals 1-8 HP".
    std::string summary() const;
};

namespace items {

const Item* find(const std::string& id);  // nullptr if unknown
const Item& get(const std::string& id);   // throws std::out_of_range if unknown
const std::vector<Item>& all();

}  // namespace items
