#include "item_system.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace vge {

// ==================== ItemRegistry ====================

ItemRegistry* ItemRegistry::instance = nullptr;

ItemRegistry& ItemRegistry::GetInstance() {
    if (!instance) {
        instance = new ItemRegistry();
    }
    return *instance;
}

void ItemRegistry::RegisterItem(const ItemDef& def) {
    items[def.itemID] = def;
    Logger::Info("[ItemRegistry] Registered item: " + def.itemID);
}

const ItemDef& ItemRegistry::GetItem(const std::string& itemID) const {
    static ItemDef empty;
    auto it = items.find(itemID);
    if (it != items.end()) {
        return it->second;
    }
    return empty;
}

bool ItemRegistry::HasItem(const std::string& itemID) const {
    return items.find(itemID) != items.end();
}

std::vector<std::string> ItemRegistry::GetItemsByCategory(ItemCategory category) const {
    std::vector<std::string> result;
    for (const auto& [id, def] : items) {
        if (def.category == category) {
            result.push_back(id);
        }
    }
    return result;
}

bool ItemRegistry::LoadFromFile(const std::string& path) {
    // TODO: Implement JSON file loading
    Logger::Info("[ItemRegistry] LoadFromFile not yet implemented: " + path);
    return false;
}

bool ItemRegistry::LoadFromJson(const std::string& json) {
    // TODO: Implement JSON parsing
    Logger::Info("[ItemRegistry] LoadFromJson not yet implemented");
    return false;
}

void ItemRegistry::Clear() {
    items.clear();
}

// ==================== Inventory ====================

Inventory::Inventory(const std::string& id, int w, int h) 
    : width(w), height(h), inventoryID(id), filterCategory(ItemCategory::Misc), filterEnabled(false) {
    slots.resize(width * height);
    for (int i = 0; i < (int)slots.size(); ++i) {
        slots[i].slotIndex = i;
    }
}

InventorySlot& Inventory::GetSlot(int index) {
    return slots[index];
}

const InventorySlot& Inventory::GetSlot(int index) const {
    return slots[index];
}

InventorySlot& Inventory::GetSlot(int x, int y) {
    return slots[GetSlotIndex(x, y)];
}

const InventorySlot& Inventory::GetSlot(int x, int y) const {
    return slots[GetSlotIndex(x, y)];
}

bool Inventory::AddItem(const std::string& itemID, int amount) {
    if (amount <= 0) return false;
    
    const ItemDef& def = ItemRegistry::GetInstance().GetItem(itemID);
    if (def.itemID.empty() && itemID != "air") {
        Logger::Warning("[Inventory] Unknown item: " + itemID);
        return false;
    }
    
    // First, try to stack with existing items
    for (auto& slot : slots) {
        if (!slot.locked && !slot.IsEmpty() && slot.item.itemID == itemID) {
            int canAdd = def.maxStackSize - slot.item.count;
            if (canAdd > 0) {
                int toAdd = (amount < canAdd) ? amount : canAdd;
                slot.item.count += toAdd;
                amount -= toAdd;
                
                if (amount <= 0) {
                    return true;
                }
            }
        }
    }
    
    // Then, try to find empty slots
    for (auto& slot : slots) {
        if (!slot.locked && slot.IsEmpty()) {
            slot.item.itemID = itemID;
            int toAdd = (amount < def.maxStackSize) ? amount : def.maxStackSize;
            slot.item.count = toAdd;
            amount -= toAdd;
            
            if (amount <= 0) {
                return true;
            }
        }
    }
    
    Logger::Warning("[Inventory] Full! Could not add " + std::to_string(amount) + " items of " + itemID);
    return false;
}

bool Inventory::AddItemToSlot(int slotIndex, const std::string& itemID, int amount) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    if (amount <= 0) return false;
    
    InventorySlot& slot = slots[slotIndex];
    if (slot.locked) return false;
    
    const ItemDef& def = ItemRegistry::GetInstance().GetItem(itemID);
    
    if (slot.IsEmpty()) {
        slot.item.itemID = itemID;
        slot.item.count = std::min(amount, def.maxStackSize);
        return true;
    } else if (slot.item.itemID == itemID) {
        int canAdd = def.maxStackSize - slot.item.count;
        if (canAdd >= amount) {
            slot.item.count += amount;
            return true;
        }
    }
    
    return false;
}

bool Inventory::RemoveItem(int slotIndex, int amount) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    if (amount <= 0) return false;
    
    InventorySlot& slot = slots[slotIndex];
    if (slot.IsEmpty()) return false;
    if (slot.item.count < amount) return false;
    
    slot.item.count -= amount;
    if (slot.item.count <= 0) {
        slot.item = ItemInstance();
    }
    
    return true;
}

bool Inventory::RemoveItem(const std::string& itemID, int amount) {
    if (amount <= 0) return false;
    if (GetItemCount(itemID) < amount) return false;
    
    int remaining = amount;
    for (auto& slot : slots) {
        if (!slot.IsEmpty() && slot.item.itemID == itemID) {
            int toRemove = std::min(remaining, slot.item.count);
            slot.item.count -= toRemove;
            remaining -= toRemove;
            
            if (slot.item.count <= 0) {
                slot.item = ItemInstance();
            }
            
            if (remaining <= 0) {
                return true;
            }
        }
    }
    
    return remaining <= 0;
}

bool Inventory::HasItem(const std::string& itemID, int amount) const {
    return GetItemCount(itemID) >= amount;
}

int Inventory::GetItemCount(const std::string& itemID) const {
    int total = 0;
    for (const auto& slot : slots) {
        if (!slot.IsEmpty() && slot.item.itemID == itemID) {
            total += slot.item.count;
        }
    }
    return total;
}

bool Inventory::CanStack(int slotIndex, const std::string& itemID) const {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    
    const InventorySlot& slot = slots[slotIndex];
    if (slot.IsEmpty()) return true;
    if (slot.item.itemID != itemID) return false;
    
    const ItemDef& def = ItemRegistry::GetInstance().GetItem(itemID);
    return slot.item.count < def.maxStackSize;
}

bool Inventory::MergeStacks(int fromSlot, int toSlot) {
    if (fromSlot < 0 || fromSlot >= (int)slots.size()) return false;
    if (toSlot < 0 || toSlot >= (int)slots.size()) return false;
    if (fromSlot == toSlot) return false;
    
    InventorySlot& from = slots[fromSlot];
    InventorySlot& to = slots[toSlot];
    
    if (from.IsEmpty()) return false;
    if (!to.IsEmpty() && to.item.itemID != from.item.itemID) return false;
    if (to.locked) return false;
    
    const ItemDef& def = ItemRegistry::GetInstance().GetItem(from.item.itemID);
    
    if (to.IsEmpty()) {
        to.item = from.item;
        from.item = ItemInstance();
        return true;
    }
    
    int canAdd = def.maxStackSize - to.item.count;
    if (canAdd <= 0) return false;
    
    int toAdd = std::min(canAdd, from.item.count);
    to.item.count += toAdd;
    from.item.count -= toAdd;
    
    if (from.item.count <= 0) {
        from.item = ItemInstance();
    }
    
    return true;
}

bool Inventory::SplitStack(int slotIndex, int amount) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    if (amount <= 0) return false;
    
    InventorySlot& slot = slots[slotIndex];
    if (slot.IsEmpty()) return false;
    if (slot.item.count <= amount) return false;
    
    // Find empty slot
    for (auto& targetSlot : slots) {
        if (targetSlot.slotIndex != slotIndex && targetSlot.IsEmpty() && !targetSlot.locked) {
            targetSlot.item.itemID = slot.item.itemID;
            targetSlot.item.count = amount;
            slot.item.count -= amount;
            return true;
        }
    }
    
    return false;
}

bool Inventory::SwapSlots(int slotA, int slotB) {
    if (slotA < 0 || slotA >= (int)slots.size()) return false;
    if (slotB < 0 || slotB >= (int)slots.size()) return false;
    if (slotA == slotB) return true;
    
    if (slots[slotA].locked || slots[slotB].locked) return false;
    
    std::swap(slots[slotA].item, slots[slotB].item);
    return true;
}

bool Inventory::MoveItem(int fromSlot, int toSlot) {
    if (fromSlot < 0 || fromSlot >= (int)slots.size()) return false;
    if (toSlot < 0 || toSlot >= (int)slots.size()) return false;
    if (fromSlot == toSlot) return true;
    
    InventorySlot& from = slots[fromSlot];
    InventorySlot& to = slots[toSlot];
    
    if (from.IsEmpty()) return false;
    if (to.locked) return false;
    
    // Try to merge if same item
    if (!to.IsEmpty() && to.item.itemID == from.item.itemID) {
        return MergeStacks(fromSlot, toSlot);
    }
    
    // Otherwise swap
    return SwapSlots(fromSlot, toSlot);
}

void Inventory::SetFilter(ItemCategory category) {
    filterCategory = category;
    filterEnabled = true;
}

void Inventory::ClearFilter() {
    filterEnabled = false;
    filterCategory = ItemCategory::Misc;
}

std::vector<int> Inventory::GetFilteredSlots() const {
    if (!filterEnabled) {
        std::vector<int> all;
        for (int i = 0; i < (int)slots.size(); ++i) {
            all.push_back(i);
        }
        return all;
    }
    
    std::vector<int> filtered;
    for (const auto& slot : slots) {
        if (slot.IsEmpty()) {
            filtered.push_back(slot.slotIndex);
        } else {
            const ItemDef& def = ItemRegistry::GetInstance().GetItem(slot.item.itemID);
            if (def.category == filterCategory) {
                filtered.push_back(slot.slotIndex);
            }
        }
    }
    return filtered;
}

void Inventory::LockSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < (int)slots.size()) {
        slots[slotIndex].locked = true;
    }
}

void Inventory::UnlockSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < (int)slots.size()) {
        slots[slotIndex].locked = false;
    }
}

bool Inventory::IsSlotLocked(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    return slots[slotIndex].locked;
}

void Inventory::Clear() {
    for (auto& slot : slots) {
        slot.item = ItemInstance();
    }
}

void Inventory::ClearSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < (int)slots.size()) {
        slots[slotIndex].item = ItemInstance();
    }
}

std::string Inventory::Serialize() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\":\"" << inventoryID << "\",";
    ss << "\"width\":" << width << ",";
    ss << "\"height\":" << height << ",";
    ss << "\"slots\":[";
    
    bool first = true;
    for (const auto& slot : slots) {
        if (!slot.IsEmpty()) {
            if (!first) ss << ",";
            first = false;
            ss << "{\"index\":" << slot.slotIndex << ",";
            ss << "\"itemID\":\"" << slot.item.itemID << "\",";
            ss << "\"count\":" << slot.item.count << "}";
        }
    }
    
    ss << "]}";
    return ss.str();
}

bool Inventory::Deserialize(const std::string& data) {
    // TODO: Implement JSON deserialization
    Logger::Info("[Inventory] Deserialize not yet implemented");
    return false;
}

void Inventory::Print() const {
    Logger::Info("=== Inventory: " + inventoryID + " ===");
    for (int y = 0; y < height; ++y) {
        std::string row;
        for (int x = 0; x < width; ++x) {
            const InventorySlot& slot = GetSlot(x, y);
            if (slot.IsEmpty()) {
                row += "[    ] ";
            } else {
                row += "[" + slot.item.itemID + ":" + std::to_string(slot.item.count) + "] ";
            }
        }
        Logger::Info(row);
    }
}

// ==================== InventoryManager ====================

InventoryManager::InventoryManager() {}

Inventory* InventoryManager::CreateInventory(const std::string& id, int width, int height) {
    auto inventory = std::make_unique<Inventory>(id, width, height);
    Inventory* ptr = inventory.get();
    inventories[id] = std::move(inventory);
    Logger::Info("[InventoryManager] Created inventory: " + id + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    return ptr;
}

Inventory* InventoryManager::GetInventory(const std::string& id) {
    auto it = inventories.find(id);
    if (it != inventories.end()) {
        return it->second.get();
    }
    return nullptr;
}

void InventoryManager::DestroyInventory(const std::string& id) {
    inventories.erase(id);
    Logger::Info("[InventoryManager] Destroyed inventory: " + id);
}

bool InventoryManager::HasInventory(const std::string& id) const {
    return inventories.find(id) != inventories.end();
}

void InventoryManager::SetActiveInventory(const std::string& id) {
    activeInventoryID = id;
}

Inventory* InventoryManager::GetActiveInventory() {
    if (activeInventoryID.empty()) return nullptr;
    return GetInventory(activeInventoryID);
}

void InventoryManager::ClearActiveInventory() {
    activeInventoryID.clear();
}

bool InventoryManager::StartDrag(const std::string& inventoryID, int slotIndex) {
    Inventory* inv = GetInventory(inventoryID);
    if (!inv) return false;
    
    InventorySlot& slot = inv->GetSlot(slotIndex);
    if (slot.IsEmpty() || slot.locked) return false;
    
    currentDrag.state = DragState::Dragging;
    currentDrag.item = slot.item;
    currentDrag.sourceSlot = slotIndex;
    currentDrag.sourceInventory = 0; // TODO: Use proper inventory index
    
    // Clear source slot temporarily (will restore on cancel)
    slot.item = ItemInstance();
    
    Logger::Info("[InventoryManager] Started drag: " + currentDrag.item.itemID + " x" + std::to_string(currentDrag.item.count));
    return true;
}

void InventoryManager::UpdateDragPosition(const Vec2& position) {
    currentDrag.dragPosition = position;
}

bool InventoryManager::EndDrag(const std::string& inventoryID, int slotIndex) {
    if (!currentDrag.IsDragging()) return false;
    
    Inventory* inv = GetInventory(inventoryID);
    if (!inv) {
        CancelDrag();
        return false;
    }
    
    InventorySlot& slot = inv->GetSlot(slotIndex);
    
    // If same inventory and slot, just cancel (restore)
    if (slotIndex == currentDrag.sourceSlot && inventoryID == activeInventoryID) {
        inv->GetSlot(slotIndex).item = currentDrag.item;
        currentDrag.Clear();
        return true;
    }
    
    // Try to place item
    if (slot.IsEmpty()) {
        slot.item = currentDrag.item;
        currentDrag.Clear();
        
        if (onItemMoved) {
            onItemMoved(inventoryID, currentDrag.sourceSlot, slotIndex);
        }
        
        Logger::Info("[InventoryManager] Dropped item into slot " + std::to_string(slotIndex));
        return true;
    }
    
    // Try to stack
    if (slot.item.itemID == currentDrag.item.itemID) {
        const ItemDef& def = ItemRegistry::GetInstance().GetItem(currentDrag.item.itemID);
        int canAdd = def.maxStackSize - slot.item.count;
        
        if (canAdd >= currentDrag.item.count) {
            slot.item.count += currentDrag.item.count;
            currentDrag.Clear();
            return true;
        } else if (canAdd > 0) {
            slot.item.count += canAdd;
            currentDrag.item.count -= canAdd;
            // Return remaining to source
            inv->GetSlot(currentDrag.sourceSlot).item = currentDrag.item;
            currentDrag.Clear();
            return true;
        }
    }
    
    // Can't place here, return to source
    CancelDrag();
    return false;
}

bool InventoryManager::EndDragToWorld() {
    if (!currentDrag.IsDragging()) return false;
    
    Logger::Info("[InventoryManager] Dropped item into world: " + currentDrag.item.itemID);
    currentDrag.Clear();
    return true;
}

void InventoryManager::CancelDrag() {
    if (!currentDrag.IsDragging()) return;
    
    // Return item to source
    Inventory* inv = GetInventory(activeInventoryID);
    if (inv && currentDrag.sourceSlot >= 0) {
        inv->GetSlot(currentDrag.sourceSlot).item = currentDrag.item;
    }
    
    Logger::Info("[InventoryManager] Cancelled drag, returned item to source");
    currentDrag.Clear();
}

bool InventoryManager::TransferItem(const std::string& fromInv, int fromSlot, 
                                     const std::string& toInv, int toSlot) {
    Inventory* source = GetInventory(fromInv);
    Inventory* target = GetInventory(toInv);
    
    if (!source || !target) return false;
    if (fromSlot < 0 || fromSlot >= source->GetTotalSlots()) return false;
    if (toSlot < 0 || toSlot >= target->GetTotalSlots()) return false;
    
    InventorySlot& srcSlot = source->GetSlot(fromSlot);
    InventorySlot& dstSlot = target->GetSlot(toSlot);
    
    if (srcSlot.IsEmpty()) return false;
    if (dstSlot.locked) return false;
    
    // Try to merge
    if (!dstSlot.IsEmpty() && dstSlot.item.itemID == srcSlot.item.itemID) {
        const ItemDef& def = ItemRegistry::GetInstance().GetItem(srcSlot.item.itemID);
        int canAdd = def.maxStackSize - dstSlot.item.count;
        
        if (canAdd >= srcSlot.item.count) {
            dstSlot.item.count += srcSlot.item.count;
            srcSlot.item = ItemInstance();
            return true;
        } else if (canAdd > 0) {
            dstSlot.item.count += canAdd;
            srcSlot.item.count -= canAdd;
            return true;
        }
    }
    
    // Swap
    if (dstSlot.IsEmpty() || !dstSlot.locked) {
        std::swap(srcSlot.item, dstSlot.item);
        return true;
    }
    
    return false;
}

void InventoryManager::SetOnItemMoved(std::function<void(const std::string&, int, int)> callback) {
    onItemMoved = callback;
}

void InventoryManager::SetOnItemAdded(std::function<void(const std::string&, const std::string&, int)> callback) {
    onItemAdded = callback;
}

void InventoryManager::SetOnItemRemoved(std::function<void(const std::string&, const std::string&, int)> callback) {
    onItemRemoved = callback;
}

bool InventoryManager::ConsumeItemsForCrafting(const std::string& inventoryID, 
                                                const std::vector<std::pair<std::string, int>>& ingredients) {
    Inventory* inv = GetInventory(inventoryID);
    if (!inv) return false;
    
    // Verify all ingredients are available
    for (const auto& [itemID, amount] : ingredients) {
        if (!inv->HasItem(itemID, amount)) {
            Logger::Warning("[InventoryManager] Missing ingredient: " + itemID + " x" + std::to_string(amount));
            return false;
        }
    }
    
    // Consume ingredients
    for (const auto& [itemID, amount] : ingredients) {
        inv->RemoveItem(itemID, amount);
    }
    
    Logger::Info("[InventoryManager] Consumed crafting ingredients");
    return true;
}

bool InventoryManager::AddCraftingResult(const std::string& inventoryID, 
                                            const std::string& itemID, int count) {
    Inventory* inv = GetInventory(inventoryID);
    if (!inv) return false;
    
    bool success = inv->AddItem(itemID, count);
    if (success && onItemAdded) {
        onItemAdded(inventoryID, itemID, count);
    }
    
    return success;
}

std::string InventoryManager::SerializeAll() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"inventories\":[";
    
    bool first = true;
    for (const auto& [id, inv] : inventories) {
        if (!first) ss << ",";
        first = false;
        ss << inv->Serialize();
    }
    
    ss << "]}";
    return ss.str();
}

bool InventoryManager::DeserializeAll(const std::string& data) {
    // TODO: Implement JSON deserialization
    Logger::Info("[InventoryManager] DeserializeAll not yet implemented");
    return false;
}

// ==================== InventoryTooltip ====================

InventoryTooltip::InventoryTooltip() : visible(false), delay(0.5f), timer(0.0f) {}

void InventoryTooltip::Show(const std::string& item, const Vec2& pos) {
    itemID = item;
    position = pos;
    timer = 0.0f;
    // Don't show immediately - wait for delay
}

void InventoryTooltip::Hide() {
    visible = false;
    itemID.clear();
    timer = 0.0f;
}

void InventoryTooltip::Update(float deltaTime) {
    if (!itemID.empty() && !visible) {
        timer += deltaTime;
        if (timer >= delay) {
            visible = true;
        }
    }
}

std::string InventoryTooltip::GenerateTooltipText(const std::string& itemID) {
    const ItemDef& def = ItemRegistry::GetInstance().GetItem(itemID);
    if (def.itemID.empty()) {
        return "Unknown Item";
    }
    
    std::string text = def.displayName + "\n";
    text += "Category: ";
    switch (def.category) {
        case ItemCategory::Block: text += "Block"; break;
        case ItemCategory::Tool: text += "Tool"; break;
        case ItemCategory::Weapon: text += "Weapon"; break;
        case ItemCategory::Armor: text += "Armor"; break;
        case ItemCategory::Consumable: text += "Consumable"; break;
        case ItemCategory::Material: text += "Material"; break;
        case ItemCategory::Misc: text += "Misc"; break;
    }
    text += "\n";
    
    if (!def.description.empty()) {
        text += def.description + "\n";
    }
    
    text += "Max Stack: " + std::to_string(def.maxStackSize);
    
    return text;
}

} // namespace vge
