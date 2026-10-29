#include "characters/Enemy.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "core/Random.h"

namespace {

EnemyTemplate make(std::string id, std::string name, std::string description, int hp, int ac,
                   int attack, int diceCount, int diceSides, int damageBonus, int xp,
                   int goldMin, int goldMax, int minLevel) {
    EnemyTemplate e;
    e.id = std::move(id);
    e.name = std::move(name);
    e.description = std::move(description);
    e.maxHp = hp;
    e.armorClass = ac;
    e.attackBonus = attack;
    e.diceCount = diceCount;
    e.diceSides = diceSides;
    e.damageBonus = damageBonus;
    e.xp = xp;
    e.goldMin = goldMin;
    e.goldMax = goldMax;
    e.minLevel = minLevel;
    return e;
}

// Bosses never appear as random encounters.
constexpr int kBossOnly = 99;

std::vector<EnemyTemplate> buildEnemies() {
    std::vector<EnemyTemplate> list;

    // ---- Forest creatures -------------------------------------------------
    {
        auto e = make("gray_wolf", "Gray Wolf", "A lean wolf with hungry yellow eyes.", 12, 12,
                      3, 1, 6, 1, 30, 1, 5, 1);
        e.attackVerb = "lunges with its fangs";
        e.loot = {{"wolf_pelt", 60}};
        list.push_back(e);
    }
    {
        auto e = make("goblin", "Goblin", "A wiry green creature clutching a rusty knife.", 10,
                      13, 3, 1, 6, 0, 25, 3, 10, 1);
        e.attackVerb = "slashes with its rusty knife";
        e.loot = {{"goblin_trinket", 40}};
        list.push_back(e);
    }
    {
        auto e = make("giant_spider", "Giant Spider",
                      "A spider the size of a pony, fangs dripping venom.", 16, 12, 4, 1, 8, 0,
                      40, 0, 4, 2);
        e.attackVerb = "bites with dripping fangs";
        e.loot = {{"spider_silk", 60}};
        list.push_back(e);
    }
    {
        auto e = make("bandit", "Bandit", "A scarred outlaw who wants your coin purse.", 18, 13,
                      4, 1, 8, 1, 50, 8, 20, 2);
        e.attackVerb = "swings a notched sabre";
        e.loot = {{"health_potion", 20}};
        list.push_back(e);
    }
    {
        auto e = make("orc", "Orc", "A hulking, tusked raider in mismatched armor.", 26, 13, 5,
                      1, 10, 2, 75, 6, 18, 3);
        e.attackVerb = "cleaves with a crude axe";
        e.loot = {{"health_potion", 15}};
        list.push_back(e);
    }
    {
        auto e = make("dire_wolf", "Dire Wolf", "A wolf as large as a horse, with a terrible howl.",
                      30, 13, 5, 2, 6, 1, 90, 0, 6, 4);
        e.attackVerb = "tears at you with massive jaws";
        e.loot = {{"wolf_pelt", 80}};
        list.push_back(e);
    }
    {
        auto e = make("forest_troll", "Forest Troll",
                      "A moss-covered brute with gnarled limbs and a foul stench.", 45, 14, 6, 2,
                      6, 3, 140, 10, 30, 5);
        e.attackVerb = "pounds you with gnarled fists";
        e.loot = {{"greater_health_potion", 25}};
        list.push_back(e);
    }

    // ---- Wizard Tower bosses ---------------------------------------------
    {
        auto e = make("gargoyle_sentinel", "Gargoyle Sentinel",
                      "A winged statue of black stone that grinds to life as you enter.", 40, 14,
                      5, 1, 10, 3, 150, 30, 50, kBossOnly);
        e.attackVerb = "rakes you with stone claws";
        e.special = {"Stone Dive", "It launches from the rafters and slams into you!", 20, 2, 8,
                     0};
        e.loot = {{"health_potion", 100}};
        e.art =
            "        (\\_/)    \n"
            "     __/ o o\\__  \n"
            "    /  \\  ^  /  \\\n"
            "   /___/|   |\\___\\\n"
            "        |___|     \n";
        list.push_back(e);
    }
    {
        auto e = make("skeleton_knight", "Skeleton Knight",
                      "An ancient knight, bound in death to serve the Wizard.", 50, 15, 6, 1, 10,
                      4, 200, 40, 60, kBossOnly);
        e.attackVerb = "hacks at you with a rusted greatsword";
        e.loot = {{"greater_health_potion", 100}};
        e.art =
            "        .-.      \n"
            "       (o.o)     \n"
            "        |=|      \n"
            "       __|__     \n"
            "     //.=|=.\\\\   \n"
            "    // .=|=. \\\\  \n"
            "    \\\\ .=|=. //  \n";
        list.push_back(e);
    }
    {
        auto e = make("flame_elemental", "Flame Elemental",
                      "A roaring pillar of living fire, born from burning spellbooks.", 55, 13,
                      6, 2, 6, 3, 250, 40, 70, kBossOnly);
        e.attackVerb = "lashes you with a fiery tendril";
        e.special = {"Engulf", "It surges forward and engulfs you in flame!", 25, 3, 6, 0};
        e.loot = {{"mana_potion", 100}};
        e.art =
            "        (  )     \n"
            "       ) (  (    \n"
            "      (  )\\  )   \n"
            "       \\(  )/    \n"
            "      (  /\\  )   \n"
            "       \\/  \\/    \n";
        list.push_back(e);
    }
    {
        auto e = make("shadow_stalker", "Shadow Stalker",
                      "A shape of living darkness that flickers between the lamps.", 60, 16, 7,
                      2, 6, 4, 300, 50, 80, kBossOnly);
        e.attackVerb = "slashes at you from the darkness";
        e.special = {"Shadow Strike", "It vanishes, then strikes from behind!", 20, 3, 8, 0};
        e.loot = {{"greater_health_potion", 100}};
        list.push_back(e);
    }
    {
        auto e = make("ogre_warlord", "Ogre Warlord",
                      "A towering ogre in spiked iron, the Wizard's chief enforcer.", 85, 14, 7,
                      2, 8, 4, 400, 60, 100, kBossOnly);
        e.attackVerb = "swings a massive spiked club";
        e.special = {"Earthshaker Slam", "It brings its club down and the whole floor shakes!",
                     20, 3, 10, 0};
        e.loot = {{"greater_health_potion", 100}};
        e.art =
            "         ____      \n"
            "        /o  o\\     \n"
            "       |  <>  |    \n"
            "        \\ -- /     \n"
            "      __/|  |\\__   \n"
            "     /   |  |   \\  \n";
        list.push_back(e);
    }
    {
        auto e = make("the_wizard", "Malgrath the Wizard",
                      "The Wizard himself, robes crackling with stolen power. Behind him the "
                      "great dragon Seraphine strains against glowing chains.",
                      110, 15, 7, 1, 8, 3, 600, 400, 500, kBossOnly);
        e.properName = true;
        e.attackVerb = "strikes with his crackling staff";
        e.special = {"Arcane Fireball", "He hurls a roaring ball of violet fire!", 35, 4, 6, 0};
        e.art =
            "             /\\        \n"
            "            /  \\       \n"
            "           /_**_\\      \n"
            "           (o  o)   *  \n"
            "            \\~~/   /   \n"
            "         ___/  \\__/    \n"
            "        /  |    |      \n"
            "           |____|      \n";
        list.push_back(e);
    }

    return list;
}

}  // namespace

namespace enemies {

const std::vector<EnemyTemplate>& all() {
    static const std::vector<EnemyTemplate> database = buildEnemies();
    return database;
}

const EnemyTemplate* find(const std::string& id) {
    for (const EnemyTemplate& e : all()) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

const EnemyTemplate& get(const std::string& id) {
    const EnemyTemplate* e = find(id);
    if (!e) throw std::out_of_range("Unknown enemy id: " + id);
    return *e;
}

const EnemyTemplate& randomEncounter(const std::vector<std::string>& pool, int playerLevel) {
    std::vector<std::pair<const EnemyTemplate*, int>> weighted;
    int totalWeight = 0;
    for (const std::string& id : pool) {
        const EnemyTemplate& e = get(id);
        if (e.minLevel > playerLevel) continue;
        // Enemies near the player's level show up more often.
        int weight = std::max(1, 4 - (playerLevel - e.minLevel));
        weighted.emplace_back(&e, weight);
        totalWeight += weight;
    }
    if (weighted.empty()) return get(pool.front());

    int pick = rng::range(1, totalWeight);
    for (const auto& [enemy, weight] : weighted) {
        pick -= weight;
        if (pick <= 0) return *enemy;
    }
    return *weighted.back().first;
}

}  // namespace enemies
