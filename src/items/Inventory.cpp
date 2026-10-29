#include "items/Inventory.h"

void Inventory::add(const std::string& itemId, int count) {
    if (count <= 0 || itemId.empty()) return;
    items_[itemId] += count;
}

bool Inventory::remove(const std::string& itemId, int count) {
    auto it = items_.find(itemId);
    if (count <= 0 || it == items_.end() || it->second < count) return false;
    it->second -= count;
    if (it->second == 0) items_.erase(it);
    return true;
}

int Inventory::count(const std::string& itemId) const {
    auto it = items_.find(itemId);
    return it == items_.end() ? 0 : it->second;
}
