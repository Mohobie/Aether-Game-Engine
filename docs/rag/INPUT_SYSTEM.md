# Input System

## Quick Reference

```cpp
vge::Input input;

// In game loop
input.Update();

// Check keys
if (input.IsKeyPressed(vge::KeyCode::W)) {
    // Move forward
}

if (input.IsKeyJustPressed(vge::KeyCode::Space)) {
    // Jump (only on first press)
}

// Mouse
float dx, dy;
input.GetMouseDelta(dx, dy);
```

## Key Codes

| Key | Code |
|-----|------|
| W, A, S, D | `KeyCode::W`, `A`, `S`, `D` |
| Space | `KeyCode::Space` |
| Escape | `KeyCode::Escape` |
| Enter | `KeyCode::Enter` |
| E, Q | `KeyCode::E`, `KeyCode::Q` |
| Shift | `KeyCode::Shift` |
| Ctrl | `KeyCode::Ctrl` |
| 1-9 | `KeyCode::Key1` - `KeyCode::Key9` |
| Arrow Keys | `KeyCode::Up`, `Down`, `Left`, `Right` |

## Mouse Input

```cpp
// Set up mouse callback (in window setup)
window.SetCursorCallback([&input](double x, double y) {
    static double lastX = x, lastY = y;
    double dx = x - lastX;
    double dy = y - lastY;
    lastX = x;
    lastY = y;
    input.SetMouseDelta(static_cast<float>(dx), static_cast<float>(dy));
});

// Lock cursor for first-person
window.SetCursorMode(true);  // Lock
window.SetCursorMode(false); // Unlock
```

## Methods

```cpp
// Key states
bool IsKeyPressed(KeyCode key);     // Currently held
bool IsKeyJustPressed(KeyCode key);  // Pressed this frame
bool IsKeyReleased(KeyCode key);     // Released this frame

// Mouse
void SetMouseDelta(float dx, float dy);
void GetMouseDelta(float& dx, float& dy);
void ResetMouseDelta();

// Scroll
void SetScrollDelta(float delta);
float GetScrollDelta();
```

## Files
- `src/platform/input_manager.h`
- `src/platform/input_manager.cpp`
