#include "core/Random.h"

#include <random>
#include <utility>

namespace rng {

namespace {
std::mt19937& engine() {
    static std::mt19937 generator{std::random_device{}()};
    return generator;
}
}  // namespace

int range(int lo, int hi) {
    if (hi < lo) std::swap(lo, hi);
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(engine());
}

int roll(int count, int sides) {
    int total = 0;
    for (int i = 0; i < count; ++i) total += range(1, sides);
    return total;
}

bool chance(int percent) { return range(1, 100) <= percent; }

}  // namespace rng
