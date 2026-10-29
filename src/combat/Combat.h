#pragma once

class Player;
struct Enemy;

namespace combat {

enum class Outcome { Victory, Defeat, Fled };

// Runs a turn-based battle until someone falls or the player escapes.
// Rewards and defeat consequences are handled by the caller (see Game::battle).
Outcome fight(Player& player, Enemy& enemy, bool canFlee = true);

// Resolves one weapon attack by the player and prints the result.
// Returns the damage dealt (0 on a miss).
int playerAttack(Player& player, Enemy& enemy, int hitBonus = 0, int damageMultiplier = 1);

}  // namespace combat
