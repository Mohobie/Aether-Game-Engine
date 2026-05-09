#include "inventory.h"
#include <iostream>

namespace vge {

InventorySlot::InventorySlot() : type(BlockType::Air), count(0) {}

Inventory::Inventory(int size) : selectedSlot(0) {
    slots.resize(size);
}

Inventory::~Inventory() {}

bool Inventory::AddItem(BlockType type, int amount) {
    // First, try to stack with existing items
    for (auto& slot : slots) {
        if (slot.type == type && slot.count < MAX_STACK_SIZE) {
            int canAdd = MAX_STACK_SIZE - slot.count;
            int toAdd = (amount < canAdd) ? amount : canAdd;
            slot.count += toAdd;
            amount -= toAdd;
            
            if (amount <= 0) {
                return true;
            }
        }
    }
    
    // Then, try to find empty slots
    for (auto& slot : slots) {
        if (slot.type == BlockType::Air || slot.count == 0) {
            slot.type = type;
            int toAdd = (amount < MAX_STACK_SIZE) ? amount : MAX_STACK_SIZE;
            slot.count = toAdd;
            amount -= toAdd;
            
            if (amount <= 0) {
                return true;
            }
        }
    }
    
    // Inventory full
    std::cout << "[Inventory] Full! Could not add " << amount << " items" << std::endl;
    return false;
}

bool Inventory::RemoveItem(int slotIndex, int amount) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) {
        return false;
    }
    
    InventorySlot& slot = slots[slotIndex];
    if (slot.count < amount) {
        return false;
    }
    
    slot.count -= amount;
    if (slot.count <= 0) {
        slot.type = BlockType::Air;
        slot.count = 0;
    }
    
    return true;
}

bool Inventory::HasItem(BlockType type, int amount) const {
    int total = 0;
    for (const auto& slot : slots) {
        if (slot.type == type) {
            total += slot.count;
            if (total >= amount) {
                return true;
            }
        }
    }
    return false;
}

int Inventory::GetItemCount(BlockType type) const {
    int total = 0;
    for (const auto& slot : slots) {
        if (slot.type == type) {
            total += slot.count;
        }
    }
    return total;
}

void Inventory::SelectSlot(int index) {
    if (index >= 0 && index < (int)slots.size()) {
        selectedSlot = index;
    }
}

BlockType Inventory::GetSelectedBlockType() const {
    if (selectedSlot >= 0 && selectedSlot < (int)slots.size()) {
        return slots[selectedSlot].type;
    }
    return BlockType::Air;
}

void Inventory::Print() const {
    std::cout << "=== Inventory ===" << std::endl;
    for (int i = 0; i < (int)slots.size(); ++i) {
        const InventorySlot& slot = slots[i];
        std::cout << "[" << i << "] ";
        if (i == selectedSlot) std::cout << "* ";
        else std::cout << "  ";
        
        if (slot.type == BlockType::Air) {
            std::cout << "Empty" << std::endl;
        } else {
            std::cout << BlockRegistry::GetInstance().GetBlock(slot.type).GetName() 
                      << " x" << slot.count << std::endl;
        }
    }
}

} // namespace vge