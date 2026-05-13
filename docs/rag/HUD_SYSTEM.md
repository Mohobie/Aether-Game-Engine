# HUD System

## Quick Reference

```cpp
vge::HUDSystem hud(&ui);
hud.Initialize();

// Update values
hud.SetHealth(15, 20);
hud.SetHunger(18, 20);
hud.SetStamina(80, 100);
hud.SetExperience(500, 1000);

// Hotbar
hud.SetHotbarSlot(0); // Select slot 0
hud.SetHotbarItem(0, "dirt", 64);
```

## Features

### Health Bar
- **Position:** Top-left
- **Size:** 200x20
- **Color:** Red (changes to orange/yellow when low)
- **Text:** Current / Max

### Hunger Bar
- **Position:** Below health
- **Size:** 200x20
- **Color:** Brown (changes to red when starving)
- **Text:** Current / Max

### Stamina Bar
- **Position:** Below hunger
- **Size:** 200x10
- **Color:** Green
- **Shows:** Sprint stamina

### Experience Bar
- **Position:** Bottom center
- **Size:** 600x10
- **Color:** Green
- **Shows:** XP progress

### Hotbar
- **Position:** Bottom center
- **Slots:** 9 slots
- **Selector:** White border highlights selected
- **Labels:** Item name and count

## Implementation

```cpp
// Create and initialize
vge::HUDSystem hud(&ui);
hud.Initialize();

// Update in game loop
hud.SetHealth(playerStats.GetHealth(), playerStats.GetMaxHealth());
hud.SetHunger(playerStats.GetHunger(), playerStats.GetMaxHunger());
hud.SetStamina(playerStats.GetStamina(), playerStats.GetMaxStamina());

// Hotbar
for (int i = 0; i < 9; i++) {
    auto item = inventory.GetHotbarSlot(i);
    hud.SetHotbarItem(i, item.id, item.count);
}

// Show/hide
hud.Show();
hud.Hide();
```

## Files
- `src/ui/hud_system.h`
- `src/ui/hud_system.cpp`
