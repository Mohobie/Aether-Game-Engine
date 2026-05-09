#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace game {

struct ItemStack {
    std::string itemID;
    uint32_t count = 0;
    uint32_t maxStack = 64;
    bool isFull() const { return count >= maxStack; }
    uint32_t space() const { return maxStack > count ? maxStack - count : 0; }
};

class Inventory {
public:
    explicit Inventory(size_t slots = 36) : items(slots) {}
    bool add(const std::string& itemID, uint32_t amount);
    bool remove(const std::string& itemID, uint32_t amount);
    uint32_t count(const std::string& itemID) const;
    const ItemStack& slot(size_t idx) const { return items[idx]; }
    size_t size() const { return items.size(); }
private:
    std::vector<ItemStack> items;
};

} // namespace game
