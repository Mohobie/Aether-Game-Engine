# Input Module API

**Files:** `input/input.h`, `input/keyboard.h`, `input/mouse.h`, `input/gamepad.h`, `input/input_action.h`

---

## `input/input.h`

```cpp
namespace vge {
```

### `class Input`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `Input&` | `void` | Singleton |
| `Update` | `void` | `void` | Update input state |
| `IsKeyDown` | `bool` | `KeyCode key` | Check key held |
| `IsKeyPressed` | `bool` | `KeyCode key` | Check key just pressed |
| `IsKeyReleased` | `bool` | `KeyCode key` | Check key just released |
| `GetMousePosition` | `Vec2` | `void` | Get mouse position |
| `GetMouseDelta` | `Vec2` | `void` | Get mouse movement |
| `IsMouseButtonDown` | `bool` | `MouseButton button` | Check mouse held |
| `IsMouseButtonPressed` | `bool` | `MouseButton button` | Check mouse pressed |
| `IsMouseButtonReleased` | `bool` | `MouseButton button` | Check mouse released |
| `GetMouseWheel` | `float` | `void` | Get scroll delta |
| `IsGamepadConnected` | `bool` | `int gamepad = 0` | Check gamepad |
| `GetGamepadAxis` | `float` | `int gamepad, GamepadAxis axis` | Get axis value |
| `IsGamepadButtonDown` | `bool` | `int gamepad, GamepadButton button` | Check button |
| `GetGamepadName` | `std::string` | `int gamepad` | Get gamepad name |
| `SetCursorMode` | `void` | `CursorMode mode` | Set cursor mode |
| `GetCursorMode` | `CursorMode` | `void` | Get cursor mode |
| `SetCursorPosition` | `void` | `float x, float y` | Set cursor pos |
| `IsAnyKeyDown` | `bool` | `void` | Any key held |
| `IsAnyMouseDown` | `bool` | `void` | Any mouse button held |
| `GetLastKeyPressed` | `KeyCode` | `void` | Get last key |
| `GetLastMouseButton` | `MouseButton` | `void` | Get last mouse button |

---

## `input/keyboard.h`

### `enum class KeyCode`
| Value | Description |
|-------|-------------|
| `A` - `Z` | Letter keys |
| `Num0` - `Num9` | Number keys |
| `Escape` | Escape key |
| `Space` | Spacebar |
| `Enter` | Enter/Return |
| `Tab` | Tab key |
| `Backspace` | Backspace |
| `Delete` | Delete key |
| `Insert` | Insert key |
| `Home` | Home key |
| `End` | End key |
| `PageUp` | Page up |
| `PageDown` | Page down |
| `Left` | Left arrow |
| `Right` | Right arrow |
| `Up` | Up arrow |
| `Down` | Down arrow |
| `Shift` | Shift |
| `Control` | Ctrl |
| `Alt` | Alt |
| `F1` - `F12` | Function keys |
| `LeftShift` | Left shift |
| `RightShift` | Right shift |
| `LeftControl` | Left ctrl |
| `RightControl` | Right ctrl |
| `LeftAlt` | Left alt |
| `RightAlt` | Right alt |
| `LeftSuper` | Left OS key |
| `RightSuper` | Right OS key |
| `Menu` | Menu key |
| `CapsLock` | Caps lock |
| `NumLock` | Num lock |
| `ScrollLock` | Scroll lock |
| `PrintScreen` | Print screen |
| `Pause` | Pause/Break |
| `Numpad0` - `Numpad9` | Numpad numbers |
| `NumpadDecimal` | Numpad decimal |
| `NumpadDivide` | Numpad / |
| `NumpadMultiply` | Numpad * |
| `NumpadSubtract` | Numpad - |
| `NumpadAdd` | Numpad + |
| `NumpadEnter` | Numpad enter |
| `NumpadEqual` | Numpad = |
| `Semicolon` | ; |
| `Equal` | = |
| `Comma` | , |
| `Minus` | - |
| `Period` | . |
| `Slash` | / |
| `GraveAccent` | ` |
| `LeftBracket` | [ |
| `Backslash` | \ |
| `RightBracket` | ] |
| `Apostrophe` | ' |
| `World1` | Non-US #1 |
| `World2` | Non-US #2 |
| `Unknown` | Unknown key |

---

## `input/mouse.h`

### `enum class MouseButton`
| Value | Description |
|-------|-------------|
| `Left` | Left button |
| `Right` | Right button |
| `Middle` | Middle button |
| `Button4` | Extra button 4 |
| `Button5` | Extra button 5 |
| `Button6` | Extra button 6 |
| `Button7` | Extra button 7 |
| `Button8` | Extra button 8 |

### `enum class CursorMode`
| Value | Description |
|-------|-------------|
| `Normal` | Normal cursor |
| `Hidden` | Hidden cursor |
| `Disabled` | Disabled/captured (FPS style) |
| `Locked` | Locked to center |

---

## `input/gamepad.h`

### `enum class GamepadAxis`
| Value | Description |
|-------|-------------|
| `LeftX` | Left stick X |
| `LeftY` | Left stick Y |
| `RightX` | Right stick X |
| `RightY` | Right stick Y |
| `LeftTrigger` | Left trigger |
| `RightTrigger` | Right trigger |

### `enum class GamepadButton`
| Value | Description |
|-------|-------------|
| `A` | A/Cross button |
| `B` | B/Circle button |
| `X` | X/Square button |
| `Y` | Y/Triangle button |
| `LeftBumper` | Left bumper |
| `RightBumper` | Right bumper |
| `Back` | Back/Select |
| `Start` | Start/Menu |
| `Guide` | Guide/PS/Xbox button |
| `LeftThumb` | Left stick click |
| `RightThumb` | Right stick click |
| `DPadUp` | D-pad up |
| `DPadRight` | D-pad right |
| `DPadDown` | D-pad down |
| `DPadLeft` | D-pad left |

---

## `input/input_action.h`

### `struct InputAction`
| Member | Type | Description |
|--------|------|-------------|
| `name` | `std::string` | Action name |
| `keys` | `std::vector<KeyCode>` | Bound keys |
| `mouseButtons` | `std::vector<MouseButton>` | Bound mouse buttons |
| `gamepadButtons` | `std::vector<GamepadButton>` | Bound gamepad buttons |
| `axis` | `GamepadAxis` | Bound axis |
| `axisDeadzone` | `float` | Axis deadzone |
| `axisSensitivity` | `float` | Axis sensitivity |
| `isContinuous` | `bool` | Continuous action |

### `class InputActionMap`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `InputActionMap` | (ctor) | `const std::string& name` | Constructor |
| `AddAction` | `void` | `const InputAction& action` | Add action |
| `RemoveAction` | `void` | `const std::string& name` | Remove action |
| `GetAction` | `InputAction*` | `const std::string& name` | Get action |
| `IsActionPressed` | `bool` | `const std::string& name` | Check pressed |
| `IsActionHeld` | `bool` | `const std::string& name` | Check held |
| `IsActionReleased` | `bool` | `const std::string& name` | Check released |
| `GetActionAxis` | `float` | `const std::string& name` | Get axis value |
| `GetActionVector2` | `Vec2` | `const std::string& nameX, const std::string& nameY` | Get 2D input |
| `Enable` | `void` | `void` | Enable map |
| `Disable` | `void` | `void` | Disable map |
| `IsEnabled` | `bool` | `void` | Check enabled |
| `GetName` | `std::string` | `void` | Get map name |
| `GetActionNames` | `std::vector<std::string>` | `void` | All action names |
| `LoadFromFile` | `bool` | `const std::string& path` | Load from file |
| `SaveToFile` | `bool` | `const std::string& path` | Save to file |
