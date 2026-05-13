# Main Menu

## Quick Reference

```cpp
vge::MainMenu mainMenu(&ui);
mainMenu.Initialize();

// Set callbacks
mainMenu.SetOnSingleplayer([]() {
    // Start singleplayer game
});

mainMenu.SetOnMultiplayer([]() {
    // Show multiplayer menu
});

mainMenu.SetOnSettings([]() {
    // Show settings menu
});

mainMenu.SetOnQuit([]() {
    // Quit game
});

// Show/hide
mainMenu.Show();
mainMenu.Hide();
```

## Features

### Menu Options
- **Singleplayer:** Start a singleplayer game
- **Multiplayer:** Join or host multiplayer
- **Settings:** Open settings menu
- **Quit:** Exit the game

### UI Elements
- Title with game name
- Subtitle with description
- Version info at bottom
- Dark background

## Implementation

```cpp
vge::MainMenu menu(&ui);
menu.Initialize();

menu.SetOnSingleplayer([]() {
    menu.Hide();
    game.StartSingleplayer();
});

menu.SetOnQuit([]() {
    window.Close();
});
```

## Files
- `src/ui/main_menu.h`
- `src/ui/main_menu.cpp`
