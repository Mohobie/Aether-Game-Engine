# Settings Menu

## Quick Reference

```cpp
vge::SettingsMenu settings(&ui);
settings.Initialize();

// Show/hide
settings.Show();
settings.Hide();
settings.Toggle();

// Callbacks
settings.SetOnVolumeChanged([](float vol) { audio.SetMasterVolume(vol); });
settings.SetOnSensitivityChanged([](float sens) { camera.SetSensitivity(sens); });
settings.SetOnRenderDistanceChanged([](int dist) { renderer.SetRenderDistance(dist); });
settings.SetOnFullscreenChanged([](bool fs) { window.SetFullscreen(fs); });
settings.SetOnVSyncChanged([](bool vsync) { window.SetVSync(vsync); });
```

## Features

### Settings
- **Volume:** Master volume slider (0-1)
- **Mouse Sensitivity:** Look speed (0.1-2.0)
- **Render Distance:** Chunk render distance (2-16)
- **Fullscreen:** Toggle fullscreen mode
- **VSync:** Enable/disable vertical sync

### UI Elements
- Sliders for numeric values
- Checkboxes for boolean options
- Back button to close menu

## Implementation

```cpp
// Create and initialize
vge::SettingsMenu settings(&ui);
settings.Initialize();

// Set callbacks
settings.SetOnVolumeChanged([](float vol) {
    audio.SetMasterVolume(vol);
});

settings.SetOnSensitivityChanged([](float sens) {
    flyCamera.SetSensitivity(sens * 0.1f);
});

// Update current values
settings.UpdateSettings(0.5f, 1.0f, 8, false, true);

// Toggle with ESC or menu button
if (input.IsKeyJustPressed(GLFW_KEY_ESCAPE)) {
    settings.Toggle();
}
```

## Files
- `src/ui/settings_menu.h`
- `src/ui/settings_menu.cpp`
