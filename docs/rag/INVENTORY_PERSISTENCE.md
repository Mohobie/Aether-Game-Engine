# Inventory Persistence

## Quick Reference

```cpp
vge::InventoryPersistence persistence;

// Save inventory
persistence.SaveInventory(inventory, "save1_inventory.json");

// Load inventory
persistence.LoadInventory(inventory, "save1_inventory.json");

// Serialize to string
std::string json = persistence.SerializeInventory(inventory);

// Deserialize from string
persistence.DeserializeInventory(inventory, json);
```

## Features

### Save/Load
- **JSON format:** Human-readable save files
- **File-based:** Save to disk
- **Slot-based:** Multiple save slots support

### Data Stored
- Item types and counts
- Slot positions
- Selected hotbar slot
- Inventory metadata

## Implementation

```cpp
// Create inventory
vge::Inventory inventory("player", 9, 4); // 9x4 grid

// Add items
inventory.AddItem("dirt", 32);
inventory.AddItem("stone_pickaxe", 1);

// Save
vge::InventoryPersistence::SaveInventory(inventory, "player_inv.json");

// Load
vge::Inventory loadedInv("player", 9, 4);
vge::InventoryPersistence::LoadInventory(loadedInv, "player_inv.json");
```

## Files
- `src/game/inventory_persistence.h`
- `src/game/inventory_persistence.cpp`
