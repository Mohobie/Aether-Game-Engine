#include "inventory_persistence.h"
#include "core/logger.h"
#include "core/item_system.h"
#include <fstream>
#include <sstream>

namespace vge {

bool InventoryPersistence::SaveInventory(const Inventory& inventory, const std::string& filepath) {
    std::string json = SerializeInventory(inventory);
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Logger::Error("[InventoryPersistence] Failed to open file for writing: " + filepath);
        return false;
    }
    
    file << json;
    file.close();
    
    Logger::Info("[InventoryPersistence] Inventory saved to " + filepath);
    return true;
}

bool InventoryPersistence::LoadInventory(Inventory& inventory, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::Info("[InventoryPersistence] No inventory file found: " + filepath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return DeserializeInventory(inventory, buffer.str());
}

std::string InventoryPersistence::SerializeInventory(const Inventory& inventory) {
    return inventory.Serialize();
}

bool InventoryPersistence::DeserializeInventory(Inventory& inventory, const std::string& json) {
    return inventory.Deserialize(json);
}

} // namespace vge
