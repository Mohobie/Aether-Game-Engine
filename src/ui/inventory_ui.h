#pragma once
#include "ui/ui_system.h"
#include "core/item_system.h"
#include "core/crafting.h"
#include <memory>
#include <functional>
#include <vector>
#include <string>

namespace vge {

// Forward declarations
class Inventory;
class InventoryManager;
class CraftingSystem;
class RecipeProgression;

// ============================================
// Item Visual Properties
// ============================================
struct ItemVisual {
    Vec3 color;           // Item color (for colored squares)
    std::string name;     // Display name
    std::string description;
    bool hasDurability;   // Whether to show durability bar
    float durabilityPercent; // 0.0 - 1.0
    
    ItemVisual() : color(0.8f, 0.8f, 0.8f), hasDurability(false), durabilityPercent(1.0f) {}
};

// ============================================
// Inventory Slot UI Element
// ============================================
class InventorySlotUI : public UIElement {
private:
    int slotIndex;
    std::string inventoryID;
    bool isHotbarSlot;
    bool isSelected;
    bool isHovered;
    bool isDragTarget;
    
    // Item data
    std::string itemID;
    int itemCount;
    bool hasItem;
    
    // Visuals
    Vec3 slotColor;
    Vec3 itemColor;
    Vec3 selectedColor;
    Vec3 hoverColor;
    Vec3 dragTargetColor;
    float slotSize;
    
public:
    InventorySlotUI(const std::string& name, int index, const std::string& invID, bool hotbar = false);
    
    // Data
    void SetItem(const std::string& id, int count);
    void ClearItem();
    bool HasItem() const { return hasItem; }
    std::string GetItemID() const { return itemID; }
    int GetItemCount() const { return itemCount; }
    int GetSlotIndex() const { return slotIndex; }
    std::string GetInventoryID() const { return inventoryID; }
    
    // State
    void SetSelected(bool selected);
    bool IsSelected() const { return isSelected; }
    void SetHovered(bool hovered) { isHovered = hovered; }
    void SetDragTarget(bool target) { isDragTarget = target; }
    
    // Rendering
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
    
    // Callbacks
    std::function<void(int)> onSlotClick;
    std::function<void(int)> onSlotRightClick;
    std::function<void(int)> onSlotHoverStart;
    std::function<void(int)> onSlotHoverEnd;
    std::function<void(int)> onSlotDragStart;
    std::function<void(int)> onSlotDrop;
};

// ============================================
// Item Tooltip
// ============================================
class ItemTooltip : public UIElement {
private:
    bool visible;
    std::string itemName;
    std::string itemDescription;
    int itemCount;
    float showTimer;
    float showDelay;
    
public:
    ItemTooltip(const std::string& name = "Tooltip");
    
    void Show(const std::string& name, const std::string& description, int count, const Vec2& pos);
    void Hide();
    void Update(float deltaTime) override;
    bool IsVisible() const { return visible; }
    
    void Render() override;
};

// ============================================
// Dragged Item Visual
// ============================================
class DraggedItem : public UIElement {
private:
    bool visible;
    std::string itemID;
    int itemCount;
    Vec3 itemColor;
    Vec2 dragOffset;
    
public:
    DraggedItem(const std::string& name = "DraggedItem");
    
    void SetItem(const std::string& id, int count, const Vec3& color);
    void ClearItem();
    bool IsDragging() const { return visible; }
    
    void UpdatePosition(const Vec2& mousePos);
    void Render() override;
};

// ============================================
// Inventory Grid Panel
// ============================================
class InventoryGridPanel : public UIPanel {
private:
    std::string inventoryID;
    int gridWidth;
    int gridHeight;
    float slotSize;
    float slotSpacing;
    std::vector<std::unique_ptr<InventorySlotUI>> slots;
    
public:
    InventoryGridPanel(const std::string& name, const std::string& invID, 
                       int width, int height, float slotSz = 50.0f);
    
    void SetSlotSize(float size) { slotSize = size; }
    void SetSlotSpacing(float spacing) { slotSpacing = spacing; }
    
    InventorySlotUI* GetSlot(int index);
    InventorySlotUI* GetSlot(int x, int y);
    int GetSlotIndexAtPosition(const Vec2& pos) const;
    Vec2 GetSlotPosition(int index) const;
    
    void UpdateSlot(int index, const std::string& itemID, int count);
    void ClearSlot(int index);
    void SelectSlot(int index);
    void ClearSelection();
    
    void Layout() override;
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
    
    // Callbacks
    std::function<void(int)> onSlotClicked;
    std::function<void(int)> onSlotRightClicked;
    std::function<void(int)> onSlotHovered;
};

// ============================================
// Crafting Grid Panel
// ============================================
class CraftingGridPanel : public UIPanel {
private:
    std::string inventoryID;
    std::vector<std::unique_ptr<InventorySlotUI>> craftingSlots;
    std::unique_ptr<InventorySlotUI> resultSlot;
    float slotSize;
    float slotSpacing;
    
public:
    CraftingGridPanel(const std::string& name, const std::string& invID, float slotSz = 50.0f);
    
    InventorySlotUI* GetCraftingSlot(int index);
    InventorySlotUI* GetResultSlot() { return resultSlot.get(); }
    
    void UpdateCraftingSlot(int index, const std::string& itemID, int count);
    void UpdateResultSlot(const std::string& itemID, int count);
    void ClearCraftingGrid();
    
    void Layout() override;
    void Render() override;
    bool HandleEvent(const UIEvent& event) override;
    
    // Callbacks
    std::function<void(int)> onCraftingSlotChanged;
    std::function<void()> onResultTaken;
};

// ============================================
// Main Inventory UI
// ============================================
class InventoryUI : public UIElement {
private:
    InventoryManager* inventoryManager;
    CraftingSystem* craftingSystem;
    RecipeProgression* recipeProgression;
    
    // UI Components
    std::unique_ptr<InventoryGridPanel> hotbarPanel;
    std::unique_ptr<InventoryGridPanel> inventoryPanel;
    std::unique_ptr<CraftingGridPanel> craftingPanel;
    std::unique_ptr<ItemTooltip> tooltip;
    std::unique_ptr<DraggedItem> draggedItem;
    
    // State
    bool isInventoryOpen;
    bool isDragging;
    int draggedFromSlot;
    std::string draggedFromInventory;
    int selectedHotbarSlot;
    int hoveredSlot;
    std::string hoveredInventory;
    
    // Screen dimensions
    Vec2 screenSize;
    
    // Layout constants
    static constexpr float HOTBAR_SLOT_SIZE = 50.0f;
    static constexpr float HOTBAR_SPACING = 5.0f;
    static constexpr float INV_SLOT_SIZE = 50.0f;
    static constexpr float INV_SPACING = 5.0f;
    
    void LayoutHotbar();
    void LayoutInventory();
    void LayoutCrafting();
    
    void UpdateSlotVisuals();
    void UpdateCraftingResult();
    
    bool StartDrag(const std::string& invID, int slotIndex);
    bool EndDrag(const std::string& invID, int slotIndex);
    void CancelDrag();
    void SplitStack(const std::string& invID, int slotIndex);
    
public:
    InventoryUI(InventoryManager* invManager, CraftingSystem* craftSystem = nullptr,
                RecipeProgression* progression = nullptr);
    
    bool Initialize();
    void Shutdown();
    
    // Inventory state
    void OpenInventory();
    void CloseInventory();
    void ToggleInventory();
    bool IsInventoryOpen() const { return isInventoryOpen; }
    
    // Hotbar
    void SelectHotbarSlot(int index);
    int GetSelectedHotbarSlot() const { return selectedHotbarSlot; }
    std::string GetSelectedItemID() const;
    
    // Input handling
    bool HandleKeyInput(int keyCode, bool pressed);
    bool HandleMouseClick(const Vec2& pos, bool rightClick);
    bool HandleMouseRelease(const Vec2& pos);
    bool HandleMouseMove(const Vec2& pos);
    
    // Updates
    void Update(float deltaTime) override;
    void Render() override;
    
    // Screen size
    void SetScreenSize(const Vec2& size) { screenSize = size; Layout(); }
    
    // Layout
    void Layout() override;
    
    // Recipe discovery
    void OnItemObtained(const std::string& itemID);
};

// ============================================
// Crafting System UI Helper
// ============================================
class CraftingSystemUI {
private:
    CraftingSystem* craftingSystem;
    RecipeProgression* progression;
    
public:
    CraftingSystemUI(CraftingSystem* system, RecipeProgression* prog = nullptr);
    
    // Recipe queries
    bool CanCraftRecipe(const std::string& recipeID, const Inventory& inventory) const;
    std::vector<const CraftingRecipe*> GetAvailableRecipes(const Inventory& inventory) const;
    std::vector<const CraftingRecipe*> GetDiscoveredRecipes() const;
    
    // Crafting operations
    bool TryCraft(const CraftingGrid& grid, CraftingResult2& outResult, const Inventory* playerInv = nullptr) const;
    bool ConsumeIngredients(const CraftingRecipe& recipe, Inventory& inventory);
    
    // Recipe discovery
    void DiscoverRecipe(const std::string& recipeID);
    bool IsRecipeDiscovered(const std::string& recipeID) const;
    void OnItemObtained(const std::string& itemID);
    
    // Default recipes setup
    static void RegisterBasicRecipes(CraftingSystem* system);
    static void RegisterBasicItemVisuals();
};

// ============================================
// Item Visual Registry
// ============================================
class ItemVisualRegistry {
private:
    std::unordered_map<std::string, ItemVisual> visuals;
    static ItemVisualRegistry* instance;
    
    ItemVisualRegistry() = default;
    
public:
    static ItemVisualRegistry& GetInstance();
    
    void RegisterVisual(const std::string& itemID, const ItemVisual& visual);
    const ItemVisual& GetVisual(const std::string& itemID) const;
    bool HasVisual(const std::string& itemID) const;
    
    void RegisterDefaults();
};

} // namespace vge
