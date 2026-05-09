#include "game/inventory.hpp"

namespace game {

bool Inventory::add(const std::string& itemID, uint32_t amount) {
    for (auto& slot : items) {
        if (slot.itemID == itemID && !slot.isFull()) {
            uint32_t take = std::min(amount, slot.space());
            slot.count += take;
            amount -= take;
            if (amount == 0) return true;
        }
    }
    for (auto& slot : items) {
        if (slot.count == 0) {
            slot.itemID = itemID;
            uint32_t take = std::min(amount, slot.maxStack);
            slot.count = take;
            amount -= take;
            if (amount == 0) return true;
        }
    }
    return amount == 0;
}

bool Inventory::remove(const std::string& itemID, uint32_t amount) {
    if (count(itemID) < amount) return false;
    for (auto& slot : items) {
        if (slot.itemID == itemID) {
            uint32_t take = std::min(amount, slot.count);
            slot.count -= take;
            if (slot.count == 0) slot.itemID.clear();
            amount -= take;
            if (amount == 0) return true;
        }
    }
    return true;
}

uint32_t Inventory::count(const std::string& itemID) const {
    uint32_t total = 0;
    for (const auto& slot : items) if (slot.itemID == itemID) total += slot.count;
    return total;
}

} // namespace game
