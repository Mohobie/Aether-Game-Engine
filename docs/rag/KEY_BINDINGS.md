# Key Bindings

## Quick Reference

```cpp
vge::KeyBindings keys;

// Load from file
keys.LoadFromFile("keybindings.cfg");

// Get key for action
int jumpKey = keys.GetKey("jump"); // Returns GLFW_KEY_SPACE

// Change binding
keys.SetKey("jump", GLFW_KEY_SPACE);

// Save to file
keys.SaveToFile("keybindings.cfg");

// Reset to defaults
keys.ResetToDefaults();
```

## Features

### Default Bindings
| Action | Default Key |
|--------|-------------|
| move_forward | W |
| move_backward | S |
| move_left | A |
| move_right | D |
| jump | Space |
| sneak | Left Shift |
| sprint | Left Control |
| attack | Q |
| use | E |
| drop | G |
| inventory | I |
| crafting | C |
| hotbar_1-9 | 1-9 |
| pause | Escape |
| debug | F3 |
| fullscreen | F11 |
| screenshot | F2 |

### File Format
```
move_forward=87
move_backward=83
jump=32
...
```

## Implementation

```cpp
vge::KeyBindings keyBindings;

// Load custom bindings
if (!keyBindings.LoadFromFile("custom_keys.cfg")) {
    // Use defaults if file doesn't exist
    keyBindings.ResetToDefaults();
}

// Check input
void ProcessInput() {
    if (input.IsKeyPressed(keyBindings.GetKey("jump"))) {
        player.Jump();
    }
    if (input.IsKeyPressed(keyBindings.GetKey("attack"))) {
        player.Attack();
    }
}

// Save on exit
keyBindings.SaveToFile("custom_keys.cfg");
```

## Files
- `src/ui/key_bindings.h`
- `src/ui/key_bindings.cpp`
