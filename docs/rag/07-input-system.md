# Input System

The engine provides comprehensive input handling for keyboard, mouse, and gamepad.

## Keyboard Input

```cpp
#include "platform/input_manager.h"

// Create input manager
vge::InputManager input;

// Check if key is held
if (input.GetKey(Key::W)) {
    // Move forward
}

// Check if key was pressed this frame
if (input.GetKeyDown(Key::Space)) {
    // Jump
}

// Check if key was released this frame
if (input.GetKeyUp(Key::Escape)) {
    // Open menu
}

// Common keys
Key::W, Key::A, Key::S, Key::D          // Movement
Key::Space                               // Jump
Key::LeftShift, Key::LeftControl         // Sprint/Crouch
Key::Escape                              // Menu
Key::E                                   // Interact
Key::F                                   // Flashlight
Key::Tab                                 // Inventory
Key::Number1 to Key::Number9             // Hotbar slots
```

## Mouse Input

```cpp
// Mouse position
int mouseX = input.GetMouseX();
int mouseY = input.GetMouseY();

// Mouse movement delta (for camera look)
float deltaX = input.GetMouseDeltaX();
float deltaY = input.GetMouseDeltaY();

// Mouse buttons
if (input.GetMouseButton(MouseButton::Left)) {
    // Break block / Attack
}

if (input.GetMouseButtonDown(MouseButton::Right)) {
    // Place block / Aim
}

if (input.GetMouseButtonUp(MouseButton::Middle)) {
    // Release middle mouse
}

// Scroll wheel
float scroll = input.GetMouseScroll();
if (scroll > 0) {
    // Scroll up - previous hotbar slot
}
if (scroll < 0) {
    // Scroll down - next hotbar slot
}

// Lock/unlock cursor (first-person mode)
input.SetCursorLocked(true);   // Hide and lock cursor
input.SetCursorLocked(false);  // Show and free cursor
```

## Gamepad Input

```cpp
#include "platform/gamepad.h"

// Check if gamepad connected
if (input.IsGamepadConnected(0)) {  // Player 1
    
    // Left stick - movement
    float leftX = input.GetGamepadAxis(0, GamepadAxis::LeftStickX);
    float leftY = input.GetGamepadAxis(0, GamepadAxis::LeftStickY);
    
    // Right stick - camera look
    float rightX = input.GetGamepadAxis(0, GamepadAxis::RightStickX);
    float rightY = input.GetGamepadAxis(0, GamepadAxis::RightStickY);
    
    // Apply deadzone
    if (abs(leftX) < 0.15f) leftX = 0;
    if (abs(leftY) < 0.15f) leftY = 0;
    
    // Triggers
    float leftTrigger = input.GetGamepadAxis(0, GamepadAxis::LeftTrigger);
    float rightTrigger = input.GetGamepadAxis(0, GamepadAxis::RightTrigger);
    
    // Buttons
    if (input.GetGamepadButton(0, GamepadButton::A)) {
        // Jump
    }
    
    if (input.GetGamepadButtonDown(0, GamepadButton::B)) {
        // Cancel / Back
    }
    
    if (input.GetGamepadButton(0, GamepadButton::X)) {
        // Interact
    }
    
    if (input.GetGamepadButton(0, GamepadButton::Y)) {
        // Inventory
    }
    
    // D-Pad
    if (input.GetGamepadButton(0, GamepadButton::DpadUp)) {
        // Hotbar up
    }
    
    // Bumpers
    if (input.GetGamepadButtonDown(0, GamepadButton::LeftBumper)) {
        // Previous item
    }
    
    if (input.GetGamepadButtonDown(0, GamepadButton::RightBumper)) {
        // Next item
    }
}
```

## Input Mapping

```cpp
// Create action mappings
class InputMapper {
public:
    void Initialize() {
        // Movement
        MapAxis("MoveForward", Key::W, Key::S);
        MapAxis("MoveRight", Key::D, Key::A);
        MapAxis("MoveForward", GamepadAxis::LeftStickY);
        MapAxis("MoveRight", GamepadAxis::LeftStickX);
        
        // Look
        MapAxis("LookUp", MouseAxis::Y);
        MapAxis("LookRight", MouseAxis::X);
        MapAxis("LookUp", GamepadAxis::RightStickY);
        MapAxis("LookRight", GamepadAxis::RightStickX);
        
        // Actions
        MapAction("Jump", Key::Space);
        MapAction("Jump", GamepadButton::A);
        
        MapAction("Interact", Key::E);
        MapAction("Interact", GamepadButton::X);
        
        MapAction("Attack", MouseButton::Left);
        MapAction("Attack", GamepadAxis::RightTrigger);
        
        MapAction("Place", MouseButton::Right);
        MapAction("Place", GamepadAxis::LeftTrigger);
        
        MapAction("Sprint", Key::LeftShift);
        MapAction("Sprint", GamepadButton::LeftStick);
        
        MapAction("Crouch", Key::LeftControl);
        MapAction("Crouch", GamepadButton::RightStick);
        
        MapAction("Inventory", Key::Tab);
        MapAction("Inventory", GamepadButton::Y);
        
        MapAction("Pause", Key::Escape);
        MapAction("Pause", GamepadButton::Menu);
    }
    
    float GetAxis(const std::string& name) {
        return axisValues[name];
    }
    
    bool GetAction(const std::string& name) {
        return actionStates[name];
    }
    
    bool GetActionDown(const std::string& name) {
        return actionDown[name];
    }
    
    void Update() {
        // Reset one-frame actions
        for (auto& pair : actionDown) {
            pair.second = false;
        }
        
        // Update all mappings
        // ... (implementation)
    }
    
private:
    std::unordered_map<std::string, float> axisValues;
    std::unordered_map<std::string, bool> actionStates;
    std::unordered_map<std::string, bool> actionDown;
};
```

## Input Contexts

```cpp
// Different input modes for different game states
enum class InputContext {
    Gameplay,
    Menu,
    Inventory,
    Editor
};

class InputContextManager {
    InputContext currentContext = InputContext::Gameplay;
    
public:
    void SetContext(InputContext ctx) {
        currentContext = ctx;
        
        // Update cursor state
        switch (ctx) {
            case InputContext::Gameplay:
                input.SetCursorLocked(true);
                break;
            case InputContext::Menu:
            case InputContext::Inventory:
                input.SetCursorLocked(false);
                break;
        }
    }
    
    void Update() {
        switch (currentContext) {
            case InputContext::Gameplay:
                UpdateGameplayInput();
                break;
            case InputContext::Menu:
                UpdateMenuInput();
                break;
            case InputContext::Inventory:
                UpdateInventoryInput();
                break;
        }
    }
    
private:
    void UpdateGameplayInput() {
        // Movement
        float forward = input.GetAxis("MoveForward");
        float right = input.GetAxis("MoveRight");
        
        // Look
        float lookX = input.GetAxis("LookRight") * mouseSensitivity;
        float lookY = input.GetAxis("LookUp") * mouseSensitivity;
        camera.Rotate(lookX, lookY);
        
        // Actions
        if (input.GetActionDown("Jump")) {
            player.Jump();
        }
        
        if (input.GetAction("Attack")) {
            player.Attack();
        }
        
        if (input.GetActionDown("Inventory")) {
            OpenInventory();
        }
    }
    
    void UpdateMenuInput() {
        // Mouse cursor navigation
        if (input.GetMouseButtonDown(MouseButton::Left)) {
            // Click UI element
        }
        
        if (input.GetActionDown("Pause")) {
            CloseMenu();
        }
    }
    
    void UpdateInventoryInput() {
        if (input.GetActionDown("Inventory") || input.GetActionDown("Pause")) {
            CloseInventory();
        }
        
        // Hotbar selection
        if (input.GetMouseScroll() > 0) {
            inventory.SelectPreviousSlot();
        }
        if (input.GetMouseScroll() < 0) {
            inventory.SelectNextSlot();
        }
    }
};
```

## Event-Based Input

```cpp
// Subscribe to input events
input.OnKeyDown([](Key key) {
    switch (key) {
        case Key::F1:
            ToggleDebugUI();
            break;
        case Key::F5:
            SaveGame();
            break;
        case Key::F9:
            LoadGame();
            break;
        case Key::F11:
            ToggleFullscreen();
            break;
        case Key::F12:
            TakeScreenshot();
            break;
    }
});

input.OnMouseButtonDown([](MouseButton button, int x, int y) {
    if (button == MouseButton::Left) {
        // Check UI first
        if (ui.HitTest(x, y)) {
            ui.OnClick(x, y);
            return;
        }
        
        // Then world interaction
        Ray ray = camera.ScreenToRay(x, y);
        world.Interact(ray);
    }
});
```

## Text Input

```cpp
// For chat, console, name entry
input.StartTextInput();  // Enable text input mode

// In update loop
std::string text = input.GetTextInput();
if (!text.empty()) {
    chatBuffer += text;
}

// Handle special keys
if (input.GetKeyDown(Key::Backspace) && !chatBuffer.empty()) {
    chatBuffer.pop_back();
}

if (input.GetKeyDown(Key::Return)) {
    SendChatMessage(chatBuffer);
    chatBuffer.clear();
}

if (input.GetKeyDown(Key::Escape)) {
    input.StopTextInput();
    CloseChat();
}
```

## Input Recording/Playback

```cpp
// Record input for replays or testing
class InputRecorder {
    struct InputFrame {
        float time;
        std::vector<Key> keysPressed;
        std::vector<Key> keysReleased;
        float mouseX, mouseY;
        float mouseDeltaX, mouseDeltaY;
    };
    
    std::vector<InputFrame> recording;
    bool isRecording = false;
    bool isPlaying = false;
    size_t playbackFrame = 0;
    
public:
    void StartRecording() {
        recording.clear();
        isRecording = true;
    }
    
    void StopRecording() {
        isRecording = false;
    }
    
    void RecordFrame(float time) {
        if (!isRecording) return;
        
        InputFrame frame;
        frame.time = time;
        frame.mouseX = input.GetMouseX();
        frame.mouseY = input.GetMouseY();
        frame.mouseDeltaX = input.GetMouseDeltaX();
        frame.mouseDeltaY = input.GetMouseDeltaY();
        
        // Record key state changes
        // ...
        
        recording.push_back(frame);
    }
    
    void StartPlayback() {
        isPlaying = true;
        playbackFrame = 0;
    }
    
    void PlaybackFrame() {
        if (!isPlaying || playbackFrame >= recording.size()) {
            isPlaying = false;
            return;
        }
        
        const auto& frame = recording[playbackFrame++];
        
        // Replay input
        // ...
    }
    
    void SaveRecording(const std::string& filename) {
        // Serialize to file
    }
    
    void LoadRecording(const std::string& filename) {
        // Deserialize from file
    }
};
```
