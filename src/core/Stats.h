#pragma once

#include <array>
#include <cstddef>
#include <string>

// The six ability scores every character has.
enum class Stat { Str, Dex, Con, Int, Wis, Cha };

constexpr std::array<Stat, 6> kAllStats = {Stat::Str, Stat::Dex, Stat::Con,
                                           Stat::Int, Stat::Wis, Stat::Cha};

std::string statShortName(Stat stat);  // "Str"
std::string statLongName(Stat stat);   // "Strength"

class Stats {
public:
    static constexpr int kMaxScore = 20;

    Stats() = default;
    Stats(int str, int dex, int con, int intel, int wis, int cha);

    int get(Stat stat) const { return values_[index(stat)]; }
    void set(Stat stat, int value) { values_[index(stat)] = value; }
    void increase(Stat stat, int amount = 1);

    // Bonus derived from a score, e.g. 14 -> +2, 8 -> -1.
    int modifier(Stat stat) const { return modifierFor(get(stat)); }
    static int modifierFor(int score);

private:
    static std::size_t index(Stat stat) { return static_cast<std::size_t>(stat); }

    std::array<int, 6> values_{10, 10, 10, 10, 10, 10};
};
