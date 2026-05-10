#pragma once
#include "core/item_system.h"
#include "ui/ui_element.h"
#include <memory>
#include <functional>

namespace vge {

// Forward declarations
class Inventory;
class InventoryManager;
class InventoryTooltip;

// UI representation of a single inventory slot
class UIInventorySlot : public UIElement {
private:
    int slotIndex;
    std::string inventoryID;
    bool isHovered;
    bool isPressed;
    bool isDragTarget;
    uint32_t backgroundColor;
    uint32_t hoverColor;
    uint32_t pressedColor;
    uint32_t borderColor;
    uint32_t dragTargetColor;
    
public:
    UIInventorySlot(int index, const std::string& invID);
    
    void setColors(uint32_t bg, uint32_t hover, uint32_t pressed, uint32_t border);
    
    int getSlotIndex() const { return slotIndex; }
    std::string getInventoryID() const { return inventoryID; }
    
    void setHovered(bool hovered) { isHovered = hovered; }
    void setPressed(bool pressed) { isPressed = pressed; }
    void setDragTarget(bool target) { isDragTarget = target; }
    
    bool isSlotHovered() const { return isHovered; }
    bool isSlotPressed() const { return isPressed; }
    bool isSlotDragTarget() const { return isDragTarget; }
    
    void render() override;
    bool onClick(const Vec2& pos) override;
    
    // Callbacks
    std::function<void(int)> onSlotClick;
    std::function<void(int)> onSlotRightClick;
    std::function<void(int)> onSlotHoverStart;
    std::function<void(int)> onSlotHoverEnd;
    std::function<void(int)> onSlotDragStart;
    std::function<void(int)> onSlotDrop;
};

// Grid-based inventory UI panel
class UIInventoryGrid : public UIElement {
private:
    std::string inventoryID;
    int gridWidth;
    int gridHeight;
    float slotSize;
    float slotSpacing;
    std::vector<std::shared_ptr<UIInventorySlot>> slotElements;
    
    uint32_t backgroundColor;
    uint32_t slotBackgroundColor;
    uint32_t slotHoverColor;
    uint32_t slotPressedColor;
    uint32_t slotBorderColor;
    
public:
    UIInventoryGrid(const std::string& invID, int width, int height);
    
    void setSlotSize(float size) { slotSize = size; }
    void setSlotSpacing(float spacing) { slotSpacing = spacing; }
    float getSlotSize() const { return slotSize; }
    float getSlotSpacing() const { return slotSpacing; }
    
    void setBackgroundColor(uint32_t color) { backgroundColor = color; }
    void setSlotColors(uint32_t bg, uint32_t hover, uint32_t pressed, uint32_t border);
    
    void refreshSlots();
    std::shared_ptr<UIInventorySlot> getSlotElement(int index);
    std::shared_ptr<UIInventorySlot> getSlotElement(int x, int y);
    
    int getSlotAtPosition(const Vec2& pos) const;
    Vec2 getSlotPosition(int index) const;
    
    void render() override;
    bool onClick(const Vec2& pos) override;
    
    // Callbacks
    std::function<void(int)> onSlotClicked;
    std::function<void(int)> onSlotRightClicked;
    std::function<void(int)> onSlotHovered;
};

// Dragged item visual
class UIDraggedItem : public UIElement {
private:
    ItemInstance item;
    Vec2 offset;
    bool visible;
    
public:
    UIDraggedItem();
    
    void setItem(const ItemInstance& itemInstance);
    void clearItem();
    const ItemInstance& getItem() const { return item; }
    
    void setDragOffset(const Vec2& dragOffset) { offset = dragOffset; }
    Vec2 getDragOffset() const { return offset; }
    
    void setDragVisible(bool vis) { visible = vis; }
    bool isDragVisible() const { return visible; }
    
    void updatePosition(const Vec2& mousePos);
    
    void render() override;
};

// Inventory tooltip UI
class UIInventoryTooltip : public UIElement {
private:
    bool visible;
    std::string itemID;
    std::string tooltipText;
    uint32_t backgroundColor;
    uint32_t textColor;
    float maxWidth;
    
public:
    UIInventoryTooltip();
    
    void show(const std::string& item, const Vec2& pos);
    void hide();
    bool isTooltipVisible() const { return visible; }
    
    void setTooltipText(const std::string& text) { tooltipText = text; }
    std::string getTooltipText() const { return tooltipText; }
    
    void setBackgroundColor(uint32_t color) { backgroundColor = color; }
    void setTextColor(uint32_t color) { textColor = color; }
    void setMaxWidth(float width) { maxWidth = width; }
    
    void render() override;
};

// Main inventory UI controller
class UIInventoryController : public UIElement {
private:
    InventoryManager* inventoryManager;
    std::shared_ptr<UIInventoryGrid> playerInventoryGrid;
    std::shared_ptr<UIInventoryGrid> hotbarGrid;
    std::shared_ptr<UIDraggedItem> draggedItem;
    std::shared_ptr<UIInventoryTooltip> tooltip;
    
    bool isInventoryOpen;
    bool isDragging;
    int draggedFromSlot;
    std::string draggedFromInventory;
    
    // Category filter buttons
    std::vector<std::shared_ptr<UIElement>> filterButtons;
    ItemCategory currentFilter;
    
public:
    UIInventoryController(InventoryManager* manager);
    
    bool initialize();
    void shutdown();
    
    void openInventory();
    void closeInventory();
    void toggleInventory();
    bool isOpen() const { return isInventoryOpen; }
    
    // Update
    void update(float deltaTime);
    void updateDrag(const Vec2& mousePos);
    void updateTooltip(const Vec2& mousePos, int hoveredSlot);
    
    // Input handling
    bool handleMouseClick(const Vec2& pos, bool isRightClick);
    bool handleMouseRelease(const Vec2& pos);
    bool handleMouseMove(const Vec2& pos);
    bool handleKeyPress(int key);
    
    // Filter
    void setCategoryFilter(ItemCategory category);
    void clearCategoryFilter();
    
    void render() override;
};

// Crafting UI integration
class UICraftingPanel : public UIElement {
private:
    InventoryManager* inventoryManager;
    std::shared_ptr<UIInventoryGrid> craftingGrid;    // 3x3 crafting grid
    std::shared_ptr<UIInventorySlot> resultSlot;        // Result slot
    std::vector<CraftingRecipe> availableRecipes;
    
public:
    UICraftingPanel(InventoryManager* manager);
    
    bool initialize();
    void refreshRecipes();
    void tryCraft();
    
    void onCraftingSlotChanged(int slot);
    void onCraftingResultTaken();
    
    void render() override;
};

} // namespace vge
