#pragma once

// Keys for Player::flag()/setFlag(). Keep them stable: they are written to save files.
namespace flags {

inline constexpr const char* kForestExplorations = "forest_explorations";
inline constexpr const char* kTowerFound = "tower_found";
inline constexpr const char* kTowerFloorsCleared = "tower_floors_cleared";
inline constexpr const char* kGameWon = "game_won";

}  // namespace flags
