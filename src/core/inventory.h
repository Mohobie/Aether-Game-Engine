#pragma once
#include <vector>
#include "voxel/block.h"

namespace vge {

constexpr int MAX_STACK_SIZE = 64;
constexpr int DEFAULT_INVENTORY_SIZE = 36; // 9 hotbar + 27 inventory

struct InventorySlot {
    BlockType type;
    int count;
    
    InventorySlot();
};

class Inventory {
private:
    std::vector<InventorySlot> slots;
    int selectedSlot;
    
public:
    Inventory(int size = DEFAULT_INVENTORY_SIZE);
    ~Inventory();
    
    // Add items to inventory (returns false if full)
    bool AddItem(BlockType type, int amount = 1);
    
    // Remove items from a specific slot
    bool RemoveItem(int slotIndex, int amount = 1);
    
    // Check if inventory has enough items
    bool HasItem(BlockType type, int amount = 1) const;
    
    // Get total count of a specific item
    int GetItemCount(BlockType type) const;
    
    // Hotbar selection
    void SelectSlot(int index);
    int GetSelectedSlot() const { return selectedSlot; }
    BlockType GetSelectedBlockType() const;
    
    // Get slot info
    const InventorySlot& GetSlot(int index) const { return slots[index]; }
    int GetSize() const { return (int)slots.size(); }
    
    // Debug
    void Print() const;
};

} // namespace vge
