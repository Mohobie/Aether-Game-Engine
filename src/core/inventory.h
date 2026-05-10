#pragma once
#include <vector>
#include "voxel/block_types.h"

namespace vge {

constexpr int MAX_STACK_SIZE = 64;
constexpr int DEFAULT_INVENTORY_SIZE = 36; // 9 hotbar + 27 inventory

struct InventorySlot {
    BlockTypeID type;
    int count;
    
    InventorySlot() : type(BLOCK_AIR), count(0) {}
};

class Inventory {
private:
    std::vector<InventorySlot> slots;
    int selectedSlot;
    
public:
    Inventory(int size = DEFAULT_INVENTORY_SIZE);
    ~Inventory();
    
    // Add items to inventory (returns false if full)
    bool AddItem(BlockTypeID type, int amount = 1);
    
    // Remove items from a specific slot
    bool RemoveItem(int slotIndex, int amount = 1);
    
    // Check if inventory has enough items
    bool HasItem(BlockTypeID type, int amount = 1) const;
    
    // Get total count of a specific item
    int GetItemCount(BlockTypeID type) const;
    
    // Hotbar selection
    void SelectSlot(int index);
    int GetSelectedSlot() const { return selectedSlot; }
    BlockTypeID GetSelectedBlockType() const;
    
    // Get slot info
    const InventorySlot& GetSlot(int index) const { return slots[index]; }
    int GetSize() const { return (int)slots.size(); }
    
    // Debug
    void Print() const;
};

} // namespace vge
