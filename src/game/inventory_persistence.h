#pragma once
#include "core/item_system.h"
#include <string>

namespace vge {

// ============================================
// Inventory Persistence
// ============================================
class InventoryPersistence {
public:
    // Save inventory to file
    static bool SaveInventory(const Inventory& inventory, const std::string& filepath);
    
    // Load inventory from file
    static bool LoadInventory(Inventory& inventory, const std::string& filepath);
    
    // Save to JSON string
    static std::string SerializeInventory(const Inventory& inventory);
    
    // Load from JSON string
    static bool DeserializeInventory(Inventory& inventory, const std::string& json);

private:

};

} // namespace vge
