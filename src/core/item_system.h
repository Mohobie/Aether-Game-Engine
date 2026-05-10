#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "core/math.h"
#include "voxel/block_types.h"

using aether::Vec2;

namespace vge {

// Item categories for organization
enum class ItemCategory {
    Block,
    Tool,
    Weapon,
    Armor,
    Consumable,
    Material,
    Misc
};

// Extended item definition with category and stack info
struct ItemDef {
    BlockTypeID blockType;      // Block type ID (BLOCK_AIR for non-block items)
    std::string itemID;         // Unique item string ID
    std::string displayName;    // Human-readable name
    std::string description;    // Tooltip description
    ItemCategory category;      // Item category
    int maxStackSize;           // Maximum stack size (1 for tools/weapons, 64 for blocks/materials)
    bool isBlock;               // Whether this item places a block
    std::string iconPath;       // Icon texture path
    
    ItemDef() : blockType(BLOCK_AIR), category(ItemCategory::Misc), maxStackSize(64), isBlock(false) {}
};

// Individual item instance in a slot
struct ItemInstance {
    std::string itemID;         // Item type ID
    int count;                  // Stack count
    int durability;             // For tools/weapons (-1 if not applicable)
    std::unordered_map<std::string, std::string> metadata; // Custom metadata
    
    ItemInstance() : count(0), durability(-1) {}
    explicit ItemInstance(const std::string& id, int cnt = 1) : itemID(id), count(cnt), durability(-1) {}
    
    bool IsEmpty() const { return itemID.empty() || count <= 0; }
    bool IsFull(const ItemDef& def) const { return count >= def.maxStackSize; }
    int SpaceRemaining(const ItemDef& def) const { return def.maxStackSize - count; }
};

// Item registry - manages item definitions
class ItemRegistry {
private:
    std::unordered_map<std::string, ItemDef> items;
    static ItemRegistry* instance;
    
    ItemRegistry() = default;
    
public:
    static ItemRegistry& GetInstance();
    
    // Register an item definition
    void RegisterItem(const ItemDef& def);
    
    // Get item definition
    const ItemDef& GetItem(const std::string& itemID) const;
    bool HasItem(const std::string& itemID) const;
    
    // Get all items in a category
    std::vector<std::string> GetItemsByCategory(ItemCategory category) const;
    
    // Load from JSON
    bool LoadFromFile(const std::string& path);
    bool LoadFromJson(const std::string& json);
    
    // Clear all items
    void Clear();
};

// Grid-based inventory slot
struct InventorySlot {
    ItemInstance item;
    bool locked;        // Slot cannot be modified (e.g., armor slots)
    int slotIndex;      // Position in grid
    
    InventorySlot() : locked(false), slotIndex(-1) {}
    bool IsEmpty() const { return item.IsEmpty(); }
};

// Drag-and-drop state
enum class DragState {
    None,
    Dragging,
    Dropped
};

struct DragOperation {
    DragState state;
    ItemInstance item;
    int sourceSlot;
    int sourceInventory;
    Vec2 dragPosition;  // Current mouse position during drag
    
    DragOperation() : state(DragState::None), sourceSlot(-1), sourceInventory(-1) {}
    bool IsDragging() const { return state == DragState::Dragging; }
    void Clear() { state = DragState::None; sourceSlot = -1; sourceInventory = -1; item = ItemInstance(); }
};

// Grid-based inventory with categories and filtering
class Inventory {
private:
    std::vector<InventorySlot> slots;
    int width;
    int height;
    std::string inventoryID;
    ItemCategory filterCategory;    // Current filter (Misc = no filter)
    bool filterEnabled;
    
public:
    Inventory(const std::string& id, int w, int h);
    ~Inventory() = default;
    
    // Grid dimensions
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    int GetTotalSlots() const { return width * height; }
    const std::string& GetID() const { return inventoryID; }
    
    // Slot access
    InventorySlot& GetSlot(int index);
    const InventorySlot& GetSlot(int index) const;
    InventorySlot& GetSlot(int x, int y);
    const InventorySlot& GetSlot(int x, int y) const;
    int GetSlotIndex(int x, int y) const { return y * width + x; }
    
    // Item operations
    bool AddItem(const std::string& itemID, int amount = 1);
    bool AddItemToSlot(int slotIndex, const std::string& itemID, int amount = 1);
    bool RemoveItem(int slotIndex, int amount = 1);
    bool RemoveItem(const std::string& itemID, int amount = 1);
    bool HasItem(const std::string& itemID, int amount = 1) const;
    int GetItemCount(const std::string& itemID) const;
    
    // Stack operations
    bool CanStack(int slotIndex, const std::string& itemID) const;
    bool MergeStacks(int fromSlot, int toSlot);
    bool SplitStack(int slotIndex, int amount);
    
    // Move/swap items between slots
    bool SwapSlots(int slotA, int slotB);
    bool MoveItem(int fromSlot, int toSlot);
    
    // Filtering
    void SetFilter(ItemCategory category);
    void ClearFilter();
    bool IsFilterEnabled() const { return filterEnabled; }
    ItemCategory GetFilter() const { return filterCategory; }
    std::vector<int> GetFilteredSlots() const;
    
    // Lock/unlock slots
    void LockSlot(int slotIndex);
    void UnlockSlot(int slotIndex);
    bool IsSlotLocked(int slotIndex) const;
    
    // Clear inventory
    void Clear();
    void ClearSlot(int slotIndex);
    
    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);
    
    // Debug
    void Print() const;
};

// Inventory manager - handles multiple inventories and drag-and-drop
class InventoryManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Inventory>> inventories;
    DragOperation currentDrag;
    std::string activeInventoryID;
    
    // Callbacks
    std::function<void(const std::string&, int, int)> onItemMoved;
    std::function<void(const std::string&, const std::string&, int)> onItemAdded;
    std::function<void(const std::string&, const std::string&, int)> onItemRemoved;
    
public:
    InventoryManager();
    ~InventoryManager() = default;
    
    // Inventory management
    Inventory* CreateInventory(const std::string& id, int width, int height);
    Inventory* GetInventory(const std::string& id);
    void DestroyInventory(const std::string& id);
    bool HasInventory(const std::string& id) const;
    
    // Active inventory (for UI)
    void SetActiveInventory(const std::string& id);
    Inventory* GetActiveInventory();
    void ClearActiveInventory();
    
    // Drag and drop
    bool StartDrag(const std::string& inventoryID, int slotIndex);
    void UpdateDragPosition(const Vec2& position);
    bool EndDrag(const std::string& inventoryID, int slotIndex);
    bool EndDragToWorld();  // Drop item into world
    void CancelDrag();
    bool IsDragging() const { return currentDrag.IsDragging(); }
    const DragOperation& GetDragOperation() const { return currentDrag; }
    const ItemInstance& GetDraggedItem() const { return currentDrag.item; }
    
    // Move between inventories
    bool TransferItem(const std::string& fromInv, int fromSlot, 
                      const std::string& toInv, int toSlot);
    
    // Callbacks
    void SetOnItemMoved(std::function<void(const std::string&, int, int)> callback);
    void SetOnItemAdded(std::function<void(const std::string&, const std::string&, int)> callback);
    void SetOnItemRemoved(std::function<void(const std::string&, const std::string&, int)> callback);
    
    // Crafting integration
    bool ConsumeItemsForCrafting(const std::string& inventoryID, 
                                  const std::vector<std::pair<std::string, int>>& ingredients);
    bool AddCraftingResult(const std::string& inventoryID, 
                           const std::string& itemID, int count);
    
    // Serialization
    std::string SerializeAll() const;
    bool DeserializeAll(const std::string& data);
};

// Tooltip system for inventory items
class InventoryTooltip {
private:
    bool visible;
    Vec2 position;
    std::string itemID;
    float delay;
    float timer;
    
public:
    InventoryTooltip();
    
    void Show(const std::string& itemID, const Vec2& pos);
    void Hide();
    void update(float deltaTime);
    bool IsVisible() const { return visible; }
    
    Vec2 GetPosition() const { return position; }
    std::string GetItemID() const { return itemID; }
    
    // Generate tooltip text from item definition
    static std::string GenerateTooltipText(const std::string& itemID);
};

} // namespace vge
