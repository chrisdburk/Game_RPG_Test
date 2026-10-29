#pragma once

// Dice and random-number helpers shared by the whole game.
namespace rng {

// Random integer in the inclusive range [lo, hi].
int range(int lo, int hi);

// Rolls `count` dice with `sides` sides each, e.g. roll(2, 6) for 2d6.
int roll(int count, int sides);

// Returns true with the given percent chance (0-100).
bool chance(int percent);

}  // namespace rng
