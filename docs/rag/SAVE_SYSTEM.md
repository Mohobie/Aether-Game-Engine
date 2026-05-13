# Save Game System

## Quick Reference

```cpp
vge::SaveGameManager saveManager;
saveManager.Initialize("saves");

// Save game
saveManager.SaveGame("world1", world, player, dayTime, dayCount);

// Load game
saveManager.LoadGame("world1", world, player, dayTime, dayCount);

// Quick save/load
saveManager.QuickSave(world, player, dayTime, dayCount);
saveManager.QuickLoad(world, player, dayTime, dayCount);

// Auto-save (call every frame)
saveManager.UpdateAutoSave(deltaTime, world, player, dayTime, dayCount);
```

## Features

- **Multiple Save Slots:** Named saves with metadata
- **Quick Save/Load:** F5/F9 style instant saves
- **Auto-Save:** Configurable interval (default 5 minutes)
- **Save Info:** Date, file size, player position, world seed
- **Save Deletion:** Remove unwanted saves

## Save Format

Uses binary format with:
- World seed
- Player position and rotation
- Day time and day count
- All chunk data (compressed with RLE)
- Checksums for integrity

## Configuration

```cpp
saveManager.EnableAutoSave(true);       // Enable/disable
saveManager.SetAutoSaveInterval(5.0f);  // Minutes between saves

// Get save list
auto saves = saveManager.GetSaveList();
for (const auto& save : saves) {
    std::cout << save.name << " - " << save.lastPlayedDate << std::endl;
}

// Delete save
saveManager.DeleteSave("old_world");
```

## Files
- `src/core/save_game.h`
- `src/core/save_game.cpp`
