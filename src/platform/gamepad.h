#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <functional>
#include <map>

namespace vge {

// ============================================================================
// Gamepad / Controller Support
// ============================================================================

// Controller types for automatic mapping
enum class ControllerType {
    Unknown,
    Xbox,           // Xbox 360 / Xbox One / Series X|S
    PlayStation,    // DualShock 4 / DualSense (PS4/PS5)
    Generic         // Generic HID gamepad
};

// Standard gamepad buttons (XInput/GLFW mapping abstraction)
enum class GamepadButton {
    A,              // Bottom face button (Xbox A / PS Cross)
    B,              // Right face button (Xbox B / PS Circle)
    X,              // Left face button (Xbox X / PS Square)
    Y,              // Top face button (Xbox Y / PS Triangle)
    LeftBumper,     // L1 / LB
    RightBumper,    // R1 / RB
    LeftTrigger,    // L2 / LT (also analog)
    RightTrigger,   // R2 / RT (also analog)
    Select,         // Back / Share / View
    Start,          // Start / Options / Menu
    LeftStick,      // L3 (press left stick)
    RightStick,     // R3 (press right stick)
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    Count
};

// Standard axes
enum class GamepadAxis {
    LeftStickX,
    LeftStickY,
    RightStickX,
    RightStickY,
    LeftTrigger,
    RightTrigger,
    Count
};

// Dead zone modes
enum class DeadZoneMode {
    Circular,       // Radial dead zone (best for movement)
    Axial,          // Per-axis dead zone (best for camera)
    ScaledCircular  // Circular with scaled output (smooth transition)
};

// Vibration / haptic feedback
struct VibrationState {
    float leftMotor;    // 0.0 - 1.0 (low frequency / large motor)
    float rightMotor; // 0.0 - 1.0 (high frequency / small motor)
    float duration;     // seconds remaining
    
    VibrationState() : leftMotor(0), rightMotor(0), duration(0) {}
};

// Individual controller state
struct ControllerState {
    int id;                         // GLFW joystick ID (0-15)
    bool connected;
    ControllerType type;
    std::string name;
    
    // Buttons
    std::array<bool, static_cast<size_t>(GamepadButton::Count)> buttons;
    std::array<bool, static_cast<size_t>(GamepadButton::Count)> prevButtons;
    
    // Axes (raw values -1.0 to 1.0, triggers 0.0 to 1.0)
    std::array<float, static_cast<size_t>(GamepadAxis::Count)> axes;
    std::array<float, static_cast<size_t>(GamepadAxis::Count)> prevAxes;
    
    // Dead zone configuration
    float deadZone;
    DeadZoneMode deadZoneMode;
    
    // Vibration
    VibrationState vibration;
    
    ControllerState() 
        : id(-1), connected(false), type(ControllerType::Unknown),
          deadZone(0.15f), deadZoneMode(DeadZoneMode::Circular) {
        buttons.fill(false);
        prevButtons.fill(false);
        axes.fill(0.0f);
        prevAxes.fill(0.0f);
    }
};

// ============================================================================
// Action Mapping System
// ============================================================================

// High-level game actions (independent of input device)
enum class GameAction {
    // Movement
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Sprint,
    Crouch,
    
    // Camera
    LookUp,
    LookDown,
    LookLeft,
    LookRight,
    
    // Interaction
    Interact,
    Attack,
    Block,
    UseItem,
    
    // UI
    Pause,
    Inventory,
    Map,
    
    // Hotbar
    Hotbar1,
    Hotbar2,
    Hotbar3,
    Hotbar4,
    Hotbar5,
    Hotbar6,
    Hotbar7,
    Hotbar8,
    Hotbar9,
    
    Count
};

// Input source type
enum class InputSourceType {
    Keyboard,
    MouseButton,
    MouseAxisX,
    MouseAxisY,
    GamepadButton,
    GamepadAxis
};

// Single input binding
struct InputBinding {
    InputSourceType sourceType;
    int sourceId;           // Key code, mouse button, gamepad button/axis index
    float axisScale;        // For axes: multiplier (e.g., -1 to invert)
    bool isAxis;            // True if this binding represents an analog value
    
    InputBinding() 
        : sourceType(InputSourceType::Keyboard), sourceId(0), axisScale(1.0f), isAxis(false) {}
    
    InputBinding(InputSourceType type, int id, float scale = 1.0f, bool axis = false)
        : sourceType(type), sourceId(id), axisScale(scale), isAxis(axis) {}
};

// Action mapping: one action can have multiple bindings (keyboard + gamepad)
struct ActionMapping {
    GameAction action;
    std::vector<InputBinding> bindings;
    
    ActionMapping() : action(GameAction::Count) {}
    explicit ActionMapping(GameAction act) : action(act) {}
};

// Simple 2D vector helper for stick handling
struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
    Vec2 normalize() const {
        float len = length();
        if (len > 0) return Vec2(x / len, y / len);
        return Vec2(0, 0);
    }
};

// ============================================================================
// Gamepad Manager
// ============================================================================

class GamepadManager {
private:
    static constexpr int MAX_CONTROLLERS = 4;
    std::array<ControllerState, MAX_CONTROLLERS> controllers;
    
    // Action mappings
    std::map<GameAction, ActionMapping> actionMappings;
    
    // Callbacks for connection events
    std::function<void(int, ControllerType)> onConnectCallback;
    std::function<void(int)> onDisconnectCallback;
    
    // Detect controller type from GLFW name
    ControllerType DetectControllerType(const std::string& name) const;
    
    // Apply dead zone to axis values
    float ApplyDeadZone(float value, float deadZone, DeadZoneMode mode) const;
    void ApplyDeadZone(Vec2& stick, float deadZone, DeadZoneMode mode) const;
    
public:
    GamepadManager();
    ~GamepadManager();
    
    // Update all controller states (call once per frame)
    void Update();
    
    // Controller queries
    bool IsConnected(int controllerId) const;
    ControllerType GetControllerType(int controllerId) const;
    std::string GetControllerName(int controllerId) const;
    int GetConnectedCount() const;
    
    // Button queries
    bool IsButtonPressed(int controllerId, GamepadButton button) const;
    bool IsButtonJustPressed(int controllerId, GamepadButton button) const;
    bool IsButtonReleased(int controllerId, GamepadButton button) const;
    
    // Axis queries (with dead zone applied)
    float GetAxis(int controllerId, GamepadAxis axis) const;
    float GetAxisRaw(int controllerId, GamepadAxis axis) const; // No dead zone
    Vec2 GetLeftStick(int controllerId) const;
    Vec2 GetRightStick(int controllerId) const;
    float GetLeftTrigger(int controllerId) const;
    float GetRightTrigger(int controllerId) const;
    
    // Dead zone configuration
    void SetDeadZone(int controllerId, float deadZone);
    void SetDeadZoneMode(int controllerId, DeadZoneMode mode);
    float GetDeadZone(int controllerId) const;
    DeadZoneMode GetDeadZoneMode(int controllerId) const;
    
    // Vibration / haptic feedback
    void SetVibration(int controllerId, float leftMotor, float rightMotor, float duration);
    void StopVibration(int controllerId);
    void UpdateVibration(float deltaTime);
    
    // Connection callbacks
    void SetOnConnectCallback(std::function<void(int, ControllerType)> callback);
    void SetOnDisconnectCallback(std::function<void(int)> callback);
    
    // Action mapping system
    void MapAction(GameAction action, const InputBinding& binding);
    void MapAction(GameAction action, const std::vector<InputBinding>& bindings);
    void ClearActionMapping(GameAction action);
    void ResetToDefaultMappings(ControllerType type);
    
    // Get action value (0.0-1.0 for buttons, -1.0 to 1.0 for axes)
    float GetActionValue(GameAction action) const;
    bool IsActionPressed(GameAction action) const;
    bool IsActionJustPressed(GameAction action) const;
    bool IsActionReleased(GameAction action) const;
    
    // Get active controller (first connected, or -1 if none)
    int GetActiveController() const;
    
    // Platform-specific vibration implementation
    #ifdef PLATFORM_WINDOWS
    void ApplyVibrationWindows(int controllerId);
    #endif
    #ifdef PLATFORM_LINUX
    void ApplyVibrationLinux(int controllerId);
    #endif
};

} // namespace vge
