#include "core/Stats.h"

#include <algorithm>

std::string statShortName(Stat stat) {
    switch (stat) {
        case Stat::Str: return "Str";
        case Stat::Dex: return "Dex";
        case Stat::Con: return "Con";
        case Stat::Int: return "Int";
        case Stat::Wis: return "Wis";
        case Stat::Cha: return "Cha";
    }
    return "?";
}

std::string statLongName(Stat stat) {
    switch (stat) {
        case Stat::Str: return "Strength";
        case Stat::Dex: return "Dexterity";
        case Stat::Con: return "Constitution";
        case Stat::Int: return "Intelligence";
        case Stat::Wis: return "Wisdom";
        case Stat::Cha: return "Charisma";
    }
    return "?";
}

Stats::Stats(int str, int dex, int con, int intel, int wis, int cha)
    : values_{str, dex, con, intel, wis, cha} {}

void Stats::increase(Stat stat, int amount) {
    set(stat, std::min(kMaxScore, get(stat) + amount));
}

int Stats::modifierFor(int score) {
    // Rounds toward negative infinity: 9 -> -1, 8 -> -1, 7 -> -2.
    int diff = score - 10;
    return diff >= 0 ? diff / 2 : -((-diff + 1) / 2);
}
