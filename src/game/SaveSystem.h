#pragma once

#include <memory>
#include <string>

class Player;

// Reads and writes a single save slot as a plain "key=value" text file.
class SaveSystem {
public:
    static bool exists(const std::string& path);
    static bool save(const Player& player, const std::string& areaId, const std::string& path);
    // Returns nullptr and fills `error` on failure.
    static std::unique_ptr<Player> load(const std::string& path, std::string& areaId,
                                        std::string& error);
};
