#pragma once

#include <map>
#include <string>

// A bag of item ids and counts. Equipped gear is held by the Player, not here.
class Inventory {
public:
    void add(const std::string& itemId, int count = 1);
    // Removes `count` of an item; returns false (and changes nothing) if there aren't enough.
    bool remove(const std::string& itemId, int count = 1);
    int count(const std::string& itemId) const;
    bool empty() const { return items_.empty(); }
    void clear() { items_.clear(); }

    const std::map<std::string, int>& entries() const { return items_; }

private:
    std::map<std::string, int> items_;
};
