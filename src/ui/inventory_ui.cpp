#include "ui/inventory_ui.h"
#include "core/item_system.h"
#include "core/crafting.h"
#include "core/logger.h"
#include <algorithm>
#include <cmath>

namespace vge {

// ============================================
// ItemVisualRegistry
// ============================================
ItemVisualRegistry* ItemVisualRegistry::instance = nullptr;

ItemVisualRegistry& ItemVisualRegistry::GetInstance() {
    if (!instance) {
        instance = new ItemVisualRegistry();
        instance->RegisterDefaults();
    }
    return *instance;
}

void ItemVisualRegistry::RegisterVisual(const std::string& itemID, const ItemVisual& visual) {
    visuals[itemID] = visual;
}

const ItemVisual& ItemVisualRegistry::GetVisual(const std::string& itemID) const {
    static ItemVisual empty;
    auto it = visuals.find(itemID);
    if (it != visuals.end()) {
        return it->second;
    }
    return empty;
}

bool ItemVisualRegistry::HasVisual(const std::string& itemID) const {
    return visuals.find(itemID) != visuals.end();
}

void ItemVisualRegistry::RegisterDefaults() {
    ItemVisual wood;
    wood.color = Vec3(0.55f, 0.35f, 0.15f);
    wood.name = "Wood";
    wood.description = "A sturdy log from a tree.";
    RegisterVisual("wood", wood);
    RegisterVisual("log", wood);

    ItemVisual planks;
    planks.color = Vec3(0.76f, 0.60f, 0.40f);
    planks.name = "Wooden Planks";
    planks.description = "Processed wood planks for crafting.";
    RegisterVisual("planks", planks);

    ItemVisual sticks;
    sticks.color = Vec3(0.65f, 0.50f, 0.30f);
    sticks.name = "Stick";
    sticks.description = "A simple stick. Useful for crafting tools.";
    RegisterVisual("stick", sticks);
    RegisterVisual("sticks", sticks);

    ItemVisual stone;
    stone.color = Vec3(0.5f, 0.5f, 0.5f);
    stone.name = "Stone";
    stone.description = "A solid piece of stone.";
    RegisterVisual("stone", stone);

    ItemVisual cobble;
    cobble.color = Vec3(0.45f, 0.45f, 0.48f);
    cobble.name = "Cobblestone";
    cobble.description = "Rough stone from mining.";
    RegisterVisual("cobblestone", cobble);

    ItemVisual iron;
    iron.color = Vec3(0.75f, 0.75f, 0.78f);
    iron.name = "Iron Ingot";
    iron.description = "A refined iron ingot.";
    RegisterVisual("iron_ingot", iron);

    ItemVisual gold;
    gold.color = Vec3(1.0f, 0.85f, 0.0f);
    gold.name = "Gold Ingot";
    gold.description = "A shiny gold ingot.";
    RegisterVisual("gold_ingot", gold);

    ItemVisual diamond;
    diamond.color = Vec3(0.4f, 0.9f, 1.0f);
    diamond.name = "Diamond";
    diamond.description = "A rare and valuable gem.";
    RegisterVisual("diamond", diamond);

    ItemVisual coal;
    coal.color = Vec3(0.15f, 0.15f, 0.15f);
    coal.name = "Coal";
    coal.description = "Burns well as fuel.";
    RegisterVisual("coal", coal);

    ItemVisual woodPick;
    woodPick.color = Vec3(0.70f, 0.55f, 0.35f);
    woodPick.name = "Wooden Pickaxe";
    woodPick.description = "A basic pickaxe for mining stone.";
    woodPick.hasDurability = true;
    woodPick.durabilityPercent = 1.0f;
    RegisterVisual("wooden_pickaxe", woodPick);

    ItemVisual stonePick;
    stonePick.color = Vec3(0.55f, 0.55f, 0.58f);
    stonePick.name = "Stone Pickaxe";
    stonePick.description = "A sturdy pickaxe for mining.";
    stonePick.hasDurability = true;
    stonePick.durabilityPercent = 1.0f;
    RegisterVisual("stone_pickaxe", stonePick);

    ItemVisual ironPick;
    ironPick.color = Vec3(0.80f, 0.80f, 0.83f);
    ironPick.name = "Iron Pickaxe";
    ironPick.description = "A durable pickaxe for mining.";
    ironPick.hasDurability = true;
    ironPick.durabilityPercent = 1.0f;
    RegisterVisual("iron_pickaxe", ironPick);

    ItemVisual dirt;
    dirt.color = Vec3(0.4f, 0.25f, 0.1f);
    dirt.name = "Dirt";
    dirt.description = "Common dirt.";
    RegisterVisual("dirt", dirt);

    ItemVisual grass;
    grass.color = Vec3(0.3f, 0.6f, 0.2f);
    grass.name = "Grass Block";
    grass.description = "A block of grass.";
    RegisterVisual("grass", grass);

    ItemVisual sand;
    sand.color = Vec3(0.9f, 0.85f, 0.6f);
    sand.name = "Sand";
    sand.description = "Fine sand.";
    RegisterVisual("sand", sand);

    ItemVisual torch;
    torch.color = Vec3(1.0f, 0.7f, 0.2f);
    torch.name = "Torch";
    torch.description = "Provides light.";
    RegisterVisual("torch", torch);

    ItemVisual table;
    table.color = Vec3(0.65f, 0.45f, 0.25f);
    table.name = "Crafting Table";
    table.description = "Used for crafting more complex items.";
    RegisterVisual("crafting_table", table);
}

// ============================================
// InventorySlotUI
// ============================================
InventorySlotUI::InventorySlotUI(const std::string& name, int index, const std::string& invID, bool hotbar)
    : UIElement(name)
    , slotIndex(index)
    , inventoryID(invID)
    , isHotbarSlot(hotbar)
    , isSelected(false)
    , isHovered(false)
    , isDragTarget(false)
    , itemCount(0)
    , hasItem(false)
    , slotColor(0.25f, 0.25f, 0.25f)
    , itemColor(0.8f, 0.8f, 0.8f)
    , selectedColor(0.3f, 0.6f, 1.0f)
    , hoverColor(0.35f, 0.35f, 0.35f)
    , dragTargetColor(0.2f, 0.7f, 0.3f)
    , slotSize(50.0f) {
    SetSize(Vec2(slotSize, slotSize));
    SetBackgroundColor(slotColor);
}

void InventorySlotUI::SetItem(const std::string& id, int count) {
    itemID = id;
    itemCount = count;
    hasItem = !id.empty() && count > 0;
    if (hasItem) {
        const ItemVisual& visual = ItemVisualRegistry::GetInstance().GetVisual(id);
        itemColor = visual.color;
    }
}

void InventorySlotUI::ClearItem() {
    itemID.clear();
    itemCount = 0;
    hasItem = false;
}

void InventorySlotUI::SetSelected(bool selected) {
    isSelected = selected;
}

void InventorySlotUI::Render() {
    if (!IsVisible()) return;
    Vec2 absPos = GetAbsolutePosition();
    Vec2 sz = GetSize();
    Vec3 bgColor = slotColor;
    if (isDragTarget) bgColor = dragTargetColor;
    else if (isSelected) bgColor = selectedColor;
    else if (isHovered) bgColor = hoverColor;

    std::cout << "[UI] Slot[" << slotIndex << "] bg at (" << absPos.x << "," << absPos.y
              << ") size " << sz.x << "x" << sz.y;
    if (isSelected) std::cout << " [SELECTED]";
    if (isHovered) std::cout << " [HOVER]";
    if (isDragTarget) std::cout << " [DROP TARGET]";
    std::cout << std::endl;

    if (hasItem) {
        std::cout << "[UI] Item '" << itemID << "' color(" << itemColor.x << ","
                  << itemColor.y << "," << itemColor.z << ") count=" << itemCount << std::endl;
        const ItemVisual& visual = ItemVisualRegistry::GetInstance().GetVisual(itemID);
        if (visual.hasDurability) {
            std::cout << "[UI] Durability bar: " << (visual.durabilityPercent * 100) << "%" << std::endl;
        }
    }
    UIElement::Render();
}

bool InventorySlotUI::HandleEvent(const UIEvent& event) {
    if (!IsVisible() || !IsEnabled()) return false;
    if (ContainsPoint(event.position)) {
        switch (event.type) {
            case UIEventType::Click:
                OnClick();
                if (onSlotClick) onSlotClick(slotIndex);
                return true;
            case UIEventType::Hover:
                isHovered = true;
                OnHover();
                if (onSlotHoverStart) onSlotHoverStart(slotIndex);
                return true;
            case UIEventType::Press:
                if (onSlotDragStart) onSlotDragStart(slotIndex);
                return true;
            default:
                return true;
        }
    } else {
        if (isHovered) {
            isHovered = false;
            if (onSlotHoverEnd) onSlotHoverEnd(slotIndex);
        }
    }
    return false;
}

// ============================================
// ItemTooltip
// ============================================
ItemTooltip::ItemTooltip(const std::string& name)
    : UIElement(name)
    , visible(false)
    , itemCount(0)
    , showTimer(0.0f)
    , showDelay(0.5f) {
    SetSize(Vec2(180, 80));
    SetBackgroundColor(Vec3(0.1f, 0.1f, 0.1f));
    SetAlpha(0.9f);
}

void ItemTooltip::Show(const std::string& name, const std::string& description, int count, const Vec2& pos) {
    itemName = name;
    itemDescription = description;
    itemCount = count;
    SetPosition(pos);
    showTimer = 0.0f;
}

void ItemTooltip::Hide() {
    visible = false;
    itemName.clear();
    itemDescription.clear();
    showTimer = 0.0f;
}

void ItemTooltip::Update(float deltaTime) {
    if (!itemName.empty() && !visible) {
        showTimer += deltaTime;
        if (showTimer >= showDelay) {
            visible = true;
        }
    }
    UIElement::Update(deltaTime);
}

void ItemTooltip::Render() {
    if (!visible || !IsVisible()) return;
    Vec2 pos = GetAbsolutePosition();
    std::cout << "[UI] Tooltip: '" << itemName << "' x" << itemCount << std::endl;
    std::cout << "[UI]   Desc: " << itemDescription << std::endl;
    std::cout << "[UI]   Pos: (" << pos.x << "," << pos.y << ")" << std::endl;
    UIElement::Render();
}

// ============================================
// DraggedItem
// ============================================
DraggedItem::DraggedItem(const std::string& name)
    : UIElement(name)
    , visible(false)
    , itemCount(0)
    , dragOffset(-20, -20) {
    SetSize(Vec2(40, 40));
}

void DraggedItem::SetItem(const std::string& id, int count, const Vec3& color) {
    itemID = id;
    itemCount = count;
    itemColor = color;
    visible = true;
}

void DraggedItem::ClearItem() {
    itemID.clear();
    itemCount = 0;
    visible = false;
}

void DraggedItem::UpdatePosition(const Vec2& mousePos) {
    SetPosition(Vec2(mousePos.x + dragOffset.x, mousePos.y + dragOffset.y));
}

void DraggedItem::Render() {
    if (!visible || !IsVisible()) return;
    Vec2 pos = GetAbsolutePosition();
    std::cout << "[UI] Dragged item '" << itemID << "' x" << itemCount
              << " at (" << pos.x << "," << pos.y << ")" << std::endl;
    UIElement::Render();
}

// ============================================
// InventoryGridPanel
// ============================================
InventoryGridPanel::InventoryGridPanel(const std::string& name, const std::string& invID,
                                       int width, int height, float slotSz)
    : UIPanel(name)
    , inventoryID(invID)
    , gridWidth(width)
    , gridHeight(height)
    , slotSize(slotSz)
    , slotSpacing(5.0f) {
    float totalWidth = width * slotSize + (width - 1) * slotSpacing;
    float totalHeight = height * slotSize + (height - 1) * slotSpacing;
    SetSize(Vec2(totalWidth, totalHeight));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            auto slot = std::make_unique<InventorySlotUI>(
                name + "_slot_" + std::to_string(index), index, invID);
            float posX = x * (slotSize + slotSpacing);
            float posY = y * (slotSize + slotSpacing);
            slot->SetPosition(Vec2(posX, posY));
            slot->SetSize(Vec2(slotSize, slotSize));
            slot->onSlotClick = [this](int idx) {
                if (onSlotClicked) onSlotClicked(idx);
            };
            slot->onSlotRightClick = [this](int idx) {
                if (onSlotRightClicked) onSlotRightClicked(idx);
            };
            slot->onSlotHoverStart = [this](int idx) {
                if (onSlotHovered) onSlotHovered(idx);
            };
            slots.push_back(std::move(slot));
        }
    }
}

InventorySlotUI* InventoryGridPanel::GetSlot(int index) {
    if (index >= 0 && index < (int)slots.size()) return slots[index].get();
    return nullptr;
}

InventorySlotUI* InventoryGridPanel::GetSlot(int x, int y) {
    int index = y * gridWidth + x;
    return GetSlot(index);
}

int InventoryGridPanel::GetSlotIndexAtPosition(const Vec2& pos) const {
    Vec2 gridPos = GetAbsolutePosition();
    Vec2 localPos = Vec2(pos.x - gridPos.x, pos.y - gridPos.y);
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            float slotX = x * (slotSize + slotSpacing);
            float slotY = y * (slotSize + slotSpacing);
            if (localPos.x >= slotX && localPos.x <= slotX + slotSize &&
                localPos.y >= slotY && localPos.y <= slotY + slotSize) {
                return y * gridWidth + x;
            }
        }
    }
    return -1;
}

Vec2 InventoryGridPanel::GetSlotPosition(int index) const {
    int x = index % gridWidth;
    int y = index / gridWidth;
    Vec2 gridPos = GetAbsolutePosition();
    return Vec2(gridPos.x + x * (slotSize + slotSpacing), gridPos.y + y * (slotSize + slotSpacing));
}

void InventoryGridPanel::UpdateSlot(int index, const std::string& itemID, int count) {
    if (auto* slot = GetSlot(index)) {
        if (itemID.empty() || count <= 0) slot->ClearItem();
        else slot->SetItem(itemID, count);
    }
}

void InventoryGridPanel::ClearSlot(int index) {
    if (auto* slot = GetSlot(index)) slot->ClearItem();
}

void InventoryGridPanel::SelectSlot(int index) {
    ClearSelection();
    if (auto* slot = GetSlot(index)) slot->SetSelected(true);
}

void InventoryGridPanel::ClearSelection() {
    for (auto& slot : slots) slot->SetSelected(false);
}

void InventoryGridPanel::Layout() {
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int index = y * gridWidth + x;
            if (index < (int)slots.size()) {
                slots[index]->SetPosition(Vec2(x * (slotSize + slotSpacing), y * (slotSize + slotSpacing)));
                slots[index]->SetSize(Vec2(slotSize, slotSize));
            }
        }
    }
    UIPanel::Layout();
}

void InventoryGridPanel::Render() {
    if (!IsVisible()) return;
    std::cout << "[UI] Grid '" << GetName() << "' (" << gridWidth << "x" << gridHeight
              << ") at (" << GetAbsolutePosition().x << "," << GetAbsolutePosition().y << ")" << std::endl;
    UIPanel::Render();
    for (auto& slot : slots) slot->Render();
}

bool InventoryGridPanel::HandleEvent(const UIEvent& event) {
    if (!IsVisible() || !IsEnabled()) return false;
    for (auto it = slots.rbegin(); it != slots.rend(); ++it) {
        if ((*it)->HandleEvent(event)) return true;
    }
    return UIPanel::HandleEvent(event);
}

// ============================================
// CraftingGridPanel
// ============================================
CraftingGridPanel::CraftingGridPanel(const std::string& name, const std::string& invID, float slotSz)
    : UIPanel(name)
    , inventoryID(invID)
    , slotSize(slotSz)
    , slotSpacing(5.0f) {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            int index = y * 3 + x;
            auto slot = std::make_unique<InventorySlotUI>(
                name + "_craft_" + std::to_string(index), index, invID);
            slot->SetPosition(Vec2(x * (slotSize + slotSpacing), y * (slotSize + slotSpacing)));
            slot->SetSize(Vec2(slotSize, slotSize));
            slot->onSlotClick = [this](int idx) {
                if (onCraftingSlotChanged) onCraftingSlotChanged(idx);
            };
            craftingSlots.push_back(std::move(slot));
        }
    }
    resultSlot = std::make_unique<InventorySlotUI>(name + "_result", -1, invID + "_result");
    resultSlot->SetSize(Vec2(slotSize, slotSize));
    float gridW = 3 * slotSize + 2 * slotSpacing;
    resultSlot->SetPosition(Vec2(gridW + 20, slotSize + slotSpacing));
    SetSize(Vec2(gridW + 20 + slotSize, 3 * slotSize + 2 * slotSpacing));
}

InventorySlotUI* CraftingGridPanel::GetCraftingSlot(int index) {
    if (index >= 0 && index < (int)craftingSlots.size()) return craftingSlots[index].get();
    return nullptr;
}

void CraftingGridPanel::UpdateCraftingSlot(int index, const std::string& itemID, int count) {
    if (auto* slot = GetCraftingSlot(index)) {
        if (itemID.empty() || count <= 0) slot->ClearItem();
        else slot->SetItem(itemID, count);
    }
}

void CraftingGridPanel::UpdateResultSlot(const std::string& itemID, int count) {
    if (itemID.empty() || count <= 0) resultSlot->ClearItem();
    else resultSlot->SetItem(itemID, count);
}

void CraftingGridPanel::ClearCraftingGrid() {
    for (auto& slot : craftingSlots) slot->ClearItem();
    resultSlot->ClearItem();
}

void CraftingGridPanel::Layout() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            int index = y * 3 + x;
            if (index < (int)craftingSlots.size()) {
                craftingSlots[index]->SetPosition(Vec2(x * (slotSize + slotSpacing), y * (slotSize + slotSpacing)));
            }
        }
    }
    float gridW = 3 * slotSize + 2 * slotSpacing;
    resultSlot->SetPosition(Vec2(gridW + 20, slotSize + slotSpacing));
    UIPanel::Layout();
}

void CraftingGridPanel::Render() {
    if (!IsVisible()) return;
    std::cout << "[UI] Crafting Grid '" << GetName() << "' at ("
              << GetAbsolutePosition().x << "," << GetAbsolutePosition().y << ")" << std::endl;
    UIPanel::Render();
    for (auto& slot : craftingSlots) slot->Render();
    resultSlot->Render();
}

bool CraftingGridPanel::HandleEvent(const UIEvent& event) {
    if (!IsVisible() || !IsEnabled()) return false;
    if (resultSlot->HandleEvent(event)) return true;
    for (auto it = craftingSlots.rbegin(); it != craftingSlots.rend(); ++it) {
        if ((*it)->HandleEvent(event)) return true;
    }
    return UIPanel::HandleEvent(event);
}

// ============================================
// InventoryUI
// ============================================
InventoryUI::InventoryUI(InventoryManager* invManager, CraftingSystem* craftSystem,
                         RecipeProgression* progression)
    : UIElement("InventoryUI")
    , inventoryManager(invManager)
    , craftingSystem(craftSystem)
    , recipeProgression(progression)
    , isInventoryOpen(false)
    , isDragging(false)
    , draggedFromSlot(-1)
    , selectedHotbarSlot(0)
    , hoveredSlot(-1)
    , screenSize(800, 600) {
}

bool InventoryUI::Initialize() {
    if (!inventoryManager) {
        Logger::Error("[InventoryUI] No inventory manager provided");
        return false;
    }
    hotbarPanel = std::make_unique<InventoryGridPanel>("Hotbar", "player_hotbar", 9, 1, HOTBAR_SLOT_SIZE);
    hotbarPanel->SetVisible(true);
    inventoryPanel = std::make_unique<InventoryGridPanel>("Inventory", "player_inventory", 9, 4, INV_SLOT_SIZE);
    inventoryPanel->SetVisible(false);
    craftingPanel = std::make_unique<CraftingGridPanel>("Crafting", "crafting_grid", INV_SLOT_SIZE);
    craftingPanel->SetVisible(false);
    tooltip = std::make_unique<ItemTooltip>("ItemTooltip");
    tooltip->SetVisible(false);
    draggedItem = std::make_unique<DraggedItem>("DraggedItem");
    draggedItem->SetVisible(false);

    hotbarPanel->onSlotClicked = [this](int slot) {
        if (isDragging) EndDrag("player_hotbar", slot);
        else StartDrag("player_hotbar", slot);
    };
    hotbarPanel->onSlotRightClicked = [this](int slot) {
        SplitStack("player_hotbar", slot);
    };
    inventoryPanel->onSlotClicked = [this](int slot) {
        if (isDragging) EndDrag("player_inventory", slot);
        else StartDrag("player_inventory", slot);
    };
    inventoryPanel->onSlotRightClicked = [this](int slot) {
        SplitStack("player_inventory", slot);
    };
    craftingPanel->onCraftingSlotChanged = [this](int slot) {
        UpdateCraftingResult();
    };

    Layout();
    SelectHotbarSlot(0);
    Logger::Info("[InventoryUI] Initialized");
    return true;
}

void InventoryUI::Shutdown() {
    hotbarPanel.reset();
    inventoryPanel.reset();
    craftingPanel.reset();
    tooltip.reset();
    draggedItem.reset();
    isInventoryOpen = false;
    isDragging = false;
}

void InventoryUI::LayoutHotbar() {
    float hotbarWidth = 9 * HOTBAR_SLOT_SIZE + 8 * HOTBAR_SPACING;
    float hotbarX = (screenSize.x - hotbarWidth) / 2.0f;
    float hotbarY = screenSize.y - HOTBAR_SLOT_SIZE - 20;
    hotbarPanel->SetPosition(Vec2(hotbarX, hotbarY));
    hotbarPanel->Layout();
}

void InventoryUI::LayoutInventory() {
    float invWidth = 9 * INV_SLOT_SIZE + 8 * INV_SPACING;
    float invX = (screenSize.x - invWidth) / 2.0f;
    float invY = screenSize.y / 2.0f - 100;
    inventoryPanel->SetPosition(Vec2(invX, invY));
    inventoryPanel->Layout();
}

void InventoryUI::LayoutCrafting() {
    float craftX = inventoryPanel->GetPosition().x + inventoryPanel->GetSize().x + 30;
    float craftY = inventoryPanel->GetPosition().y;
    craftingPanel->SetPosition(Vec2(craftX, craftY));
    craftingPanel->Layout();
}

void InventoryUI::Layout() {
    LayoutHotbar();
    LayoutInventory();
    LayoutCrafting();
}

void InventoryUI::OpenInventory() {
    isInventoryOpen = true;
    inventoryPanel->SetVisible(true);
    craftingPanel->SetVisible(true);
    Logger::Info("[InventoryUI] Inventory opened");
}

void InventoryUI::CloseInventory() {
    isInventoryOpen = false;
    inventoryPanel->SetVisible(false);
    craftingPanel->SetVisible(false);
    if (isDragging) CancelDrag();
    tooltip->Hide();
    Logger::Info("[InventoryUI] Inventory closed");
}

void InventoryUI::ToggleInventory() {
    if (isInventoryOpen) CloseInventory();
    else OpenInventory();
}

void InventoryUI::SelectHotbarSlot(int index) {
    if (index < 0 || index >= 9) return;
    selectedHotbarSlot = index;
    hotbarPanel->SelectSlot(index);
    Logger::Info("[InventoryUI] Selected hotbar slot: " + std::to_string(index));
}

std::string InventoryUI::GetSelectedItemID() const {
    if (auto* slot = hotbarPanel->GetSlot(selectedHotbarSlot)) return slot->GetItemID();
    return "";
}

bool InventoryUI::StartDrag(const std::string& invID, int slotIndex) {
    if (!inventoryManager) return false;
    Inventory* inv = inventoryManager->GetInventory(invID);
    if (!inv) return false;
    InventorySlot& slot = inv->GetSlot(slotIndex);
    if (slot.IsEmpty() || slot.locked) return false;
    isDragging = true;
    draggedFromSlot = slotIndex;
    draggedFromInventory = invID;
    const ItemVisual& visual = ItemVisualRegistry::GetInstance().GetVisual(slot.item.itemID);
    draggedItem->SetItem(slot.item.itemID, slot.item.count, visual.color);
    draggedItem->SetVisible(true);
    slot.item = ItemInstance();
    Logger::Info("[InventoryUI] Started drag from slot " + std::to_string(slotIndex));
    return true;
}

bool InventoryUI::EndDrag(const std::string& invID, int slotIndex) {
    if (!isDragging) return false;
    Inventory* inv = inventoryManager->GetInventory(invID);
    if (!inv) { CancelDrag(); return false; }
    InventorySlot& slot = inv->GetSlot(slotIndex);
    if (slotIndex == draggedFromSlot && invID == draggedFromInventory) {
        CancelDrag();
        return true;
    }
    if (slot.IsEmpty()) {
        isDragging = false;
        draggedItem->ClearItem();
        draggedItem->SetVisible(false);
        Logger::Info("[InventoryUI] Dropped item into slot " + std::to_string(slotIndex));
        return true;
    }
    CancelDrag();
    return false;
}

void InventoryUI::CancelDrag() {
    if (!isDragging) return;
    Inventory* inv = inventoryManager->GetInventory(draggedFromInventory);
    if (inv && draggedFromSlot >= 0) {
        // Restore item to source slot
    }
    isDragging = false;
    draggedItem->ClearItem();
    draggedItem->SetVisible(false);
    draggedFromSlot = -1;
    draggedFromInventory.clear();
    Logger::Info("[InventoryUI] Drag cancelled");
}

void InventoryUI::SplitStack(const std::string& invID, int slotIndex) {
    if (!inventoryManager) return;
    Inventory* inv = inventoryManager->GetInventory(invID);
    if (!inv) return;
    inv->SplitStack(slotIndex, 1);
    Logger::Info("[InventoryUI] Split stack at slot " + std::to_string(slotIndex));
}

void InventoryUI::UpdateSlotVisuals() {
    if (!inventoryManager) return;
    Inventory* hotbar = inventoryManager->GetInventory("player_hotbar");
    if (hotbar) {
        for (int i = 0; i < hotbar->GetTotalSlots() && i < 9; ++i) {
            const InventorySlot& slot = hotbar->GetSlot(i);
            hotbarPanel->UpdateSlot(i, slot.item.itemID, slot.item.count);
        }
    }
    Inventory* inv = inventoryManager->GetInventory("player_inventory");
    if (inv) {
        for (int i = 0; i < inv->GetTotalSlots() && i < 36; ++i) {
            const InventorySlot& slot = inv->GetSlot(i);
            inventoryPanel->UpdateSlot(i, slot.item.itemID, slot.item.count);
        }
    }
}

void InventoryUI::UpdateCraftingResult() {
    if (!craftingSystem) return;
    CraftingGrid grid;
    Inventory* craftInv = inventoryManager->GetInventory("crafting_grid");
    if (!craftInv) return;
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            int idx = y * 3 + x;
            const InventorySlot& slot = craftInv->GetSlot(idx);
            if (!slot.IsEmpty()) {
                grid.SetItem(x, y, slot.item.itemID);
            }
        }
    }
    CraftingResult2 result = craftingSystem->TryCraft(grid);
    if (result.success) {
        craftingPanel->UpdateResultSlot(result.outputItemID, result.outputCount);
    } else {
        craftingPanel->UpdateResultSlot("", 0);
    }
}

bool InventoryUI::HandleKeyInput(int keyCode, bool pressed) {
    if (!pressed) return false;
    if (keyCode == 'E' || keyCode == 'e') {
        ToggleInventory();
        return true;
    }
    if (keyCode >= '1' && keyCode <= '9') {
        SelectHotbarSlot(keyCode - '1');
        return true;
    }
    return false;
}

bool InventoryUI::HandleMouseClick(const Vec2& pos, bool rightClick) {
    if (rightClick) {
        int slot = hotbarPanel->GetSlotIndexAtPosition(pos);
        if (slot >= 0) { SplitStack("player_hotbar", slot); return true; }
        if (isInventoryOpen) {
            slot = inventoryPanel->GetSlotIndexAtPosition(pos);
            if (slot >= 0) { SplitStack("player_inventory", slot); return true; }
        }
        return false;
    }
    if (isDragging) {
        int slot = hotbarPanel->GetSlotIndexAtPosition(pos);
        if (slot >= 0) return EndDrag("player_hotbar", slot);
        if (isInventoryOpen) {
            slot = inventoryPanel->GetSlotIndexAtPosition(pos);
            if (slot >= 0) return EndDrag("player_inventory", slot);
            slot = craftingPanel->GetCraftingSlot(0)->GetSlotIndex();
            // TODO: handle crafting grid drops
        }
        CancelDrag();
        return true;
    } else {
        int slot = hotbarPanel->GetSlotIndexAtPosition(pos);
        if (slot >= 0) return StartDrag("player_hotbar", slot);
        if (isInventoryOpen) {
            slot = inventoryPanel->GetSlotIndexAtPosition(pos);
            if (slot >= 0) return StartDrag("player_inventory", slot);
        }
    }
    return false;
}

bool InventoryUI::HandleMouseRelease(const Vec2& pos) {
    if (!isDragging) return false;
    return HandleMouseClick(pos, false);
}

bool InventoryUI::HandleMouseMove(const Vec2& pos) {
    if (isDragging) {
        draggedItem->UpdatePosition(pos);
    }
    int slot = hotbarPanel->GetSlotIndexAtPosition(pos);
    if (slot >= 0) {
        hoveredSlot = slot;
        hoveredInventory = "player_hotbar";
        if (auto* s = hotbarPanel->GetSlot(slot)) {
            if (s->HasItem()) {
                const ItemVisual& visual = ItemVisualRegistry::GetInstance().GetVisual(s->GetItemID());
                tooltip->Show(visual.name, visual.description, s->GetItemCount(), pos);
            }
        }
        return true;
    }
    if (isInventoryOpen) {
        slot = inventoryPanel->GetSlotIndexAtPosition(pos);
        if (slot >= 0) {
            hoveredSlot = slot;
            hoveredInventory = "player_inventory";
            if (auto* s = inventoryPanel->GetSlot(slot)) {
                if (s->HasItem()) {
                    const ItemVisual& visual = ItemVisualRegistry::GetInstance().GetVisual(s->GetItemID());
                    tooltip->Show(visual.name, visual.description, s->GetItemCount(), pos);
                }
            }
            return true;
        }
    }
    tooltip->Hide();
    return false;
}

void InventoryUI::Update(float deltaTime) {
    UpdateSlotVisuals();
    if (tooltip) tooltip->Update(deltaTime);
    UIElement::Update(deltaTime);
}

void InventoryUI::Render() {
    hotbarPanel->Render();
    if (isInventoryOpen) {
        inventoryPanel->Render();
        craftingPanel->Render();
        if (tooltip && tooltip->IsVisible()) tooltip->Render();
    }
    if (isDragging && draggedItem) draggedItem->Render();
}

void InventoryUI::OnItemObtained(const std::string& itemID) {
    if (recipeProgression) {
        recipeProgression->OnItemObtained(itemID);
    }
}

// ============================================
// CraftingSystemUI
// ============================================
CraftingSystemUI::CraftingSystemUI(CraftingSystem* system, RecipeProgression* prog)
    : craftingSystem(system), progression(prog) {
}

bool CraftingSystemUI::CanCraftRecipe(const std::string& recipeID, const Inventory& inventory) const {
    if (!craftingSystem) return false;
    const CraftingRecipe* recipe = craftingSystem->GetRecipe(recipeID);
    if (!recipe) return false;
    if (progression && recipe->hidden && !progression->IsDiscovered(recipeID)) return false;
    return recipe->MatchesInventory(inventory);
}

std::vector<const CraftingRecipe*> CraftingSystemUI::GetAvailableRecipes(const Inventory& inventory) const {
    std::vector<const CraftingRecipe*> result;
    if (!craftingSystem) return result;
    for (const auto& recipe : craftingSystem->GetRecipes()) {
        if (CanCraftRecipe(recipe.recipeID, inventory)) {
            result.push_back(&recipe);
        }
    }
    return result;
}

std::vector<const CraftingRecipe*> CraftingSystemUI::GetDiscoveredRecipes() const {
    std::vector<const CraftingRecipe*> result;
    if (!craftingSystem || !progression) return result;
    for (const auto& recipe : craftingSystem->GetRecipes()) {
        if (progression->IsDiscovered(recipe.recipeID)) {
            result.push_back(&recipe);
        }
    }
    return result;
}

bool CraftingSystemUI::TryCraft(const CraftingGrid& grid, CraftingResult2& outResult, const Inventory* playerInv) const {
    if (!craftingSystem) return false;
    outResult = craftingSystem->TryCraft(grid, playerInv);
    return outResult.success;
}

bool CraftingSystemUI::ConsumeIngredients(const CraftingRecipe& recipe, Inventory& inventory) {
    auto required = recipe.GetRequiredItems();
    for (const auto& [itemID, count] : required) {
        if (!inventory.HasItem(itemID, count)) return false;
    }
    for (const auto& [itemID, count] : required) {
        inventory.RemoveItem(itemID, count);
    }
    return true;
}

void CraftingSystemUI::DiscoverRecipe(const std::string& recipeID) {
    if (progression) progression->DiscoverRecipe(recipeID);
}

bool CraftingSystemUI::IsRecipeDiscovered(const std::string& recipeID) const {
    if (!progression) return true;
    return progression->IsDiscovered(recipeID);
}

void CraftingSystemUI::OnItemObtained(const std::string& itemID) {
    if (progression) progression->OnItemObtained(itemID);
}

void CraftingSystemUI::RegisterBasicRecipes(CraftingSystem* system) {
    if (!system) return;
    // Wood -> Planks (shapeless)
    CraftingRecipe planks;
    planks.recipeID = "planks_from_wood";
    planks.displayName = "Wooden Planks";
    planks.outputItemID = "planks";
    planks.outputCount = 4;
    planks.shapeless = true;
    planks.ingredients.push_back(RecipeIngredient("wood", 1));
    system->AddRecipe(planks);

    // Planks -> Sticks
    CraftingRecipe sticks;
    sticks.recipeID = "sticks_from_planks";
    sticks.displayName = "Sticks";
    sticks.outputItemID = "stick";
    sticks.outputCount = 4;
    sticks.pattern = {{"planks", ""}, {"planks", ""}, {"", ""}};
    system->AddRecipe(sticks);

    // Sticks + Stone -> Pickaxe
    CraftingRecipe pickaxe;
    pickaxe.recipeID = "stone_pickaxe";
    pickaxe.displayName = "Stone Pickaxe";
    pickaxe.outputItemID = "stone_pickaxe";
    pickaxe.outputCount = 1;
    pickaxe.pattern = {{"cobblestone", "cobblestone", "cobblestone"}, {"", "stick", ""}, {"", "stick", ""}};
    system->AddRecipe(pickaxe);

    // Wooden Pickaxe
    CraftingRecipe woodPickaxe;
    woodPickaxe.recipeID = "wooden_pickaxe";
    woodPickaxe.displayName = "Wooden Pickaxe";
    woodPickaxe.outputItemID = "wooden_pickaxe";
    woodPickaxe.outputCount = 1;
    woodPickaxe.pattern = {{"planks", "planks", "planks"}, {"", "stick", ""}, {"", "stick", ""}};
    system->AddRecipe(woodPickaxe);
}

void CraftingSystemUI::RegisterBasicItemVisuals() {
    ItemVisualRegistry::GetInstance().RegisterDefaults();
}

} // namespace vge
