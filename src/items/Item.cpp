#include "items/Item.h"

#include <stdexcept>
#include <utility>

namespace {

Item weapon(std::string id, std::string name, std::string description, int price, int damage) {
    Item item;
    item.id = std::move(id);
    item.name = std::move(name);
    item.description = std::move(description);
    item.type = ItemType::Weapon;
    item.price = price;
    item.damageBonus = damage;
    return item;
}

Item armor(std::string id, std::string name, std::string description, int price, int bonus) {
    Item item;
    item.id = std::move(id);
    item.name = std::move(name);
    item.description = std::move(description);
    item.type = ItemType::Armor;
    item.price = price;
    item.armorBonus = bonus;
    return item;
}

Item consumable(std::string id, std::string name, std::string description, int price,
                ItemEffect effect, int min, int max) {
    Item item;
    item.id = std::move(id);
    item.name = std::move(name);
    item.description = std::move(description);
    item.type = ItemType::Consumable;
    item.price = price;
    item.effect = effect;
    item.effectMin = min;
    item.effectMax = max;
    return item;
}

Item material(std::string id, std::string name, std::string description, int price) {
    Item item;
    item.id = std::move(id);
    item.name = std::move(name);
    item.description = std::move(description);
    item.type = ItemType::Material;
    item.price = price;
    return item;
}

std::vector<Item> buildItems() {
    return {
        // Weapons
        weapon("shortsword", "Shortsword", "A plain but reliable blade.", 20, 5),
        weapon("longsword", "Longsword", "A well-balanced knight's sword.", 75, 7),
        weapon("battleaxe", "Battleaxe", "A heavy, bearded axe that bites deep.", 180, 9),
        weapon("greatsword", "Greatsword", "A massive two-handed blade of dwarven steel.", 450,
               12),

        // Armor
        armor("leather_armor", "Leather Armor", "Boiled leather, light and quiet.", 40, 2),
        armor("chain_shirt", "Chain Shirt", "Interlocking steel rings worn under a tabard.",
              140, 4),
        armor("plate_armor", "Plate Armor", "Full plate, polished to a mirror shine.", 400, 6),

        // Consumables
        consumable("health_potion", "Health Potion", "A small vial of red liquid.", 15,
                   ItemEffect::Heal, 1, 8),
        consumable("greater_health_potion", "Greater Health Potion",
                   "A large flask that glows a deep crimson.", 45, ItemEffect::Heal, 10, 20),
        consumable("mana_potion", "Mana Potion", "A small vial of shimmering blue liquid.", 15,
                   ItemEffect::RestoreMana, 1, 10),

        // Materials (sold to shops or delivered for quests)
        material("moonpetal_herb", "Moonpetal Herb",
                 "A pale flower that only blooms in sunlit forest clearings.", 8),
        material("wolf_pelt", "Wolf Pelt", "A thick gray pelt, prized by tanners.", 10),
        material("spider_silk", "Spider Silk", "A sticky bundle of strong, fine silk.", 12),
        material("goblin_trinket", "Goblin Trinket", "A shiny bauble of dubious origin.", 6),
    };
}

}  // namespace

std::string Item::summary() const {
    switch (type) {
        case ItemType::Weapon: return "+" + std::to_string(damageBonus) + " damage";
        case ItemType::Armor: return "+" + std::to_string(armorBonus) + " armor";
        case ItemType::Consumable: {
            std::string amount = std::to_string(effectMin) + "-" + std::to_string(effectMax);
            if (effect == ItemEffect::Heal) return "Heals " + amount + " HP";
            if (effect == ItemEffect::RestoreMana) return "Restores " + amount + " MP";
            return "Consumable";
        }
        case ItemType::Material: return "Material";
    }
    return "";
}

namespace items {

const std::vector<Item>& all() {
    static const std::vector<Item> database = buildItems();
    return database;
}

const Item* find(const std::string& id) {
    for (const Item& item : all()) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

const Item& get(const std::string& id) {
    const Item* item = find(id);
    if (!item) throw std::out_of_range("Unknown item id: " + id);
    return *item;
}

}  // namespace items
