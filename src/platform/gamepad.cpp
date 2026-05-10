#include "platform/gamepad.h"
#include <cmath>
#include <algorithm>
#include <cstring>

// GLFW gamepad constants (embedded for standalone use)
#define GLFW_JOYSTICK_1             0
#define GLFW_JOYSTICK_2             1
#define GLFW_JOYSTICK_3             2
#define GLFW_JOYSTICK_4             3
#define GLFW_JOYSTICK_5             4
#define GLFW_JOYSTICK_6             5
#define GLFW_JOYSTICK_7             6
#define GLFW_JOYSTICK_8             7
#define GLFW_JOYSTICK_9             8
#define GLFW_JOYSTICK_10            9
#define GLFW_JOYSTICK_11            10
#define GLFW_JOYSTICK_12            11
#define GLFW_JOYSTICK_13            12
#define GLFW_JOYSTICK_14            13
#define GLFW_JOYSTICK_15            14
#define GLFW_JOYSTICK_16            15
#define GLFW_JOYSTICK_LAST          GLFW_JOYSTICK_16

#define GLFW_GAMEPAD_BUTTON_A               0
#define GLFW_GAMEPAD_BUTTON_B               1
#define GLFW_GAMEPAD_BUTTON_X               2
#define GLFW_GAMEPAD_BUTTON_Y               3
#define GLFW_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define GLFW_GAMEPAD_BUTTON_BACK            6
#define GLFW_GAMEPAD_BUTTON_START           7
#define GLFW_GAMEPAD_BUTTON_GUIDE           8
#define GLFW_GAMEPAD_BUTTON_LEFT_THUMB      9
#define GLFW_GAMEPAD_BUTTON_RIGHT_THUMB    10
#define GLFW_GAMEPAD_BUTTON_DPAD_UP        11
#define GLFW_GAMEPAD_BUTTON_DPAD_RIGHT     12
#define GLFW_GAMEPAD_BUTTON_DPAD_DOWN      13
#define GLFW_GAMEPAD_BUTTON_DPAD_LEFT      14
#define GLFW_GAMEPAD_BUTTON_LAST           GLFW_GAMEPAD_BUTTON_DPAD_LEFT
#define GLFW_GAMEPAD_BUTTON_CROSS       GLFW_GAMEPAD_BUTTON_A
#define GLFW_GAMEPAD_BUTTON_CIRCLE      GLFW_GAMEPAD_BUTTON_B
#define GLFW_GAMEPAD_BUTTON_SQUARE      GLFW_GAMEPAD_BUTTON_X
#define GLFW_GAMEPAD_BUTTON_TRIANGLE    GLFW_GAMEPAD_BUTTON_Y

#define GLFW_GAMEPAD_AXIS_LEFT_X        0
#define GLFW_GAMEPAD_AXIS_LEFT_Y        1
#define GLFW_GAMEPAD_AXIS_RIGHT_X       2
#define GLFW_GAMEPAD_AXIS_RIGHT_Y       3
#define GLFW_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define GLFW_GAMEPAD_AXIS_LAST          GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER

namespace vge {

// ============================================================================
// Controller Type Detection
// ============================================================================

ControllerType GamepadManager::DetectControllerType(const std::string& name) const {
    std::string lower;
    lower.reserve(name.size());
    for (char c : name) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    
    // Xbox controllers
    if (lower.find("xbox") != std::string::npos ||
        lower.find("xinput") != std::string::npos ||
        lower.find("microsoft") != std::string::npos) {
        return ControllerType::Xbox;
    }
    
    // PlayStation controllers
    if (lower.find("playstation") != std::string::npos ||
        lower.find("dualshock") != std::string::npos ||
        lower.find("dualsense") != std::string::npos ||
        lower.find("sony") != std::string::npos ||
        lower.find("ps4") != std::string::npos ||
        lower.find("ps5") != std::string::npos ||
        lower.find("wireless controller") != std::string::npos) {
        return ControllerType::PlayStation;
    }
    
    return ControllerType::Generic;
}

// ============================================================================
// Dead Zone Handling
// ============================================================================

float GamepadManager::ApplyDeadZone(float value, float deadZone, DeadZoneMode mode) const {
    if (mode == DeadZoneMode::Axial) {
        if (std::abs(value) < deadZone) {
            return 0.0f;
        }
        // Rescale to use full range after dead zone
        float sign = (value > 0) ? 1.0f : -1.0f;
        return sign * (std::abs(value) - deadZone) / (1.0f - deadZone);
    }
    // For circular modes, individual axis dead zone is handled in GetAxis
    return value;
}

void GamepadManager::ApplyDeadZone(Vec2& stick, float deadZone, DeadZoneMode mode) const {
    if (mode == DeadZoneMode::Circular || mode == DeadZoneMode::ScaledCircular) {
        float magnitude = stick.length();
        if (magnitude < deadZone) {
            stick.x = 0.0f;
            stick.y = 0.0f;
            return;
        }
        
        if (mode == DeadZoneMode::ScaledCircular) {
            // Rescale so output starts at 0 after dead zone and reaches 1 at max
            float scaled = (magnitude - deadZone) / (1.0f - deadZone);
            if (scaled > 1.0f) scaled = 1.0f;
            float scale = scaled / magnitude;
            stick.x *= scale;
            stick.y *= scale;
        }
    }
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

GamepadManager::GamepadManager() 
    : onConnectCallback(nullptr), onDisconnectCallback(nullptr) {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        controllers[i].id = i;
    }
    ResetToDefaultMappings(ControllerType::Xbox);
}

GamepadManager::~GamepadManager() {
    // Stop all vibration on shutdown
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        StopVibration(i);
    }
}

// ============================================================================
// Update
// ============================================================================

void GamepadManager::Update() {
    // In a real GLFW implementation, this would call:
    // - glfwJoystickPresent() for each controller
    // - glfwGetGamepadState() for connected controllers
    // For this standalone implementation, we simulate the update pattern
    
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        ControllerState& state = controllers[i];
        
        // Store previous state
        state.prevButtons = state.buttons;
        state.prevAxes = state.axes;
        
        // In a real implementation with GLFW:
        // GLFWgamepadstate gpState;
        // if (glfwGetGamepadState(i, &gpState)) {
        //     state.buttons[static_cast<size_t>(GamepadButton::A)] = gpState.buttons[GLFW_GAMEPAD_BUTTON_A];
        //     ... etc
        //     state.axes[static_cast<size_t>(GamepadAxis::LeftStickX)] = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        //     ... etc
        // }
        
        // Connection detection (simulated - would use glfwJoystickPresent in real impl)
        bool wasConnected = state.connected;
        // state.connected = glfwJoystickPresent(i) == GLFW_TRUE;
        
        // On connect
        if (!wasConnected && state.connected) {
            // const char* name = glfwGetJoystickName(i);
            // state.name = name ? name : "Unknown Controller";
            // state.type = DetectControllerType(state.name);
            
            if (onConnectCallback) {
                onConnectCallback(i, state.type);
            }
        }
        
        // On disconnect
        if (wasConnected && !state.connected) {
            if (onDisconnectCallback) {
                onDisconnectCallback(i);
            }
            // Reset state
            state = ControllerState();
            state.id = i;
        }
    }
}

// ============================================================================
// Controller Queries
// ============================================================================

bool GamepadManager::IsConnected(int controllerId) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return false;
    return controllers[controllerId].connected;
}

ControllerType GamepadManager::GetControllerType(int controllerId) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return ControllerType::Unknown;
    return controllers[controllerId].type;
}

std::string GamepadManager::GetControllerName(int controllerId) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return "";
    return controllers[controllerId].name;
}

int GamepadManager::GetConnectedCount() const {
    int count = 0;
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (controllers[i].connected) count++;
    }
    return count;
}

int GamepadManager::GetActiveController() const {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (controllers[i].connected) return i;
    }
    return -1;
}

// ============================================================================
// Button Queries
// ============================================================================

bool GamepadManager::IsButtonPressed(int controllerId, GamepadButton button) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return false;
    size_t idx = static_cast<size_t>(button);
    if (idx >= static_cast<size_t>(GamepadButton::Count)) return false;
    return controllers[controllerId].buttons[idx];
}

bool GamepadManager::IsButtonJustPressed(int controllerId, GamepadButton button) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return false;
    size_t idx = static_cast<size_t>(button);
    if (idx >= static_cast<size_t>(GamepadButton::Count)) return false;
    return controllers[controllerId].buttons[idx] && 
           !controllers[controllerId].prevButtons[idx];
}

bool GamepadManager::IsButtonReleased(int controllerId, GamepadButton button) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return false;
    size_t idx = static_cast<size_t>(button);
    if (idx >= static_cast<size_t>(GamepadButton::Count)) return false;
    return !controllers[controllerId].buttons[idx] && 
           controllers[controllerId].prevButtons[idx];
}

// ============================================================================
// Axis Queries
// ============================================================================

float GamepadManager::GetAxisRaw(int controllerId, GamepadAxis axis) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return 0.0f;
    size_t idx = static_cast<size_t>(axis);
    if (idx >= static_cast<size_t>(GamepadAxis::Count)) return 0.0f;
    return controllers[controllerId].axes[idx];
}

float GamepadManager::GetAxis(int controllerId, GamepadAxis axis) const {
    float raw = GetAxisRaw(controllerId, axis);
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return 0.0f;
    
    const ControllerState& state = controllers[controllerId];
    
    // Triggers use axial dead zone (they're 0-1, not -1 to 1)
    if (axis == GamepadAxis::LeftTrigger || axis == GamepadAxis::RightTrigger) {
        if (raw < state.deadZone) return 0.0f;
        return (raw - state.deadZone) / (1.0f - state.deadZone);
    }
    
    // Sticks use per-axis dead zone (circular is handled in GetLeftStick/GetRightStick)
    return ApplyDeadZone(raw, state.deadZone, DeadZoneMode::Axial);
}

Vec2 GamepadManager::GetLeftStick(int controllerId) const {
    Vec2 stick;
    stick.x = GetAxisRaw(controllerId, GamepadAxis::LeftStickX);
    stick.y = GetAxisRaw(controllerId, GamepadAxis::LeftStickY);
    
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return stick;
    
    const ControllerState& state = controllers[controllerId];
    ApplyDeadZone(stick, state.deadZone, state.deadZoneMode);
    return stick;
}

Vec2 GamepadManager::GetRightStick(int controllerId) const {
    Vec2 stick;
    stick.x = GetAxisRaw(controllerId, GamepadAxis::RightStickX);
    stick.y = GetAxisRaw(controllerId, GamepadAxis::RightStickY);
    
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return stick;
    
    const ControllerState& state = controllers[controllerId];
    ApplyDeadZone(stick, state.deadZone, state.deadZoneMode);
    return stick;
}

float GamepadManager::GetLeftTrigger(int controllerId) const {
    return GetAxis(controllerId, GamepadAxis::LeftTrigger);
}

float GamepadManager::GetRightTrigger(int controllerId) const {
    return GetAxis(controllerId, GamepadAxis::RightTrigger);
}

// ============================================================================
// Dead Zone Configuration
// ============================================================================

void GamepadManager::SetDeadZone(int controllerId, float deadZone) {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return;
    controllers[controllerId].deadZone = std::clamp(deadZone, 0.0f, 1.0f);
}

void GamepadManager::SetDeadZoneMode(int controllerId, DeadZoneMode mode) {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return;
    controllers[controllerId].deadZoneMode = mode;
}

float GamepadManager::GetDeadZone(int controllerId) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return 0.0f;
    return controllers[controllerId].deadZone;
}

DeadZoneMode GamepadManager::GetDeadZoneMode(int controllerId) const {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return DeadZoneMode::Circular;
    return controllers[controllerId].deadZoneMode;
}

// ============================================================================
// Vibration / Haptic Feedback
// ============================================================================

void GamepadManager::SetVibration(int controllerId, float leftMotor, float rightMotor, float duration) {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return;
    if (!controllers[controllerId].connected) return;
    
    controllers[controllerId].vibration.leftMotor = std::clamp(leftMotor, 0.0f, 1.0f);
    controllers[controllerId].vibration.rightMotor = std::clamp(rightMotor, 0.0f, 1.0f);
    controllers[controllerId].vibration.duration = duration;
    
    #ifdef PLATFORM_WINDOWS
    ApplyVibrationWindows(controllerId);
    #endif
    #ifdef PLATFORM_LINUX
    ApplyVibrationLinux(controllerId);
    #endif
}

void GamepadManager::StopVibration(int controllerId) {
    if (controllerId < 0 || controllerId >= MAX_CONTROLLERS) return;
    controllers[controllerId].vibration.leftMotor = 0.0f;
    controllers[controllerId].vibration.rightMotor = 0.0f;
    controllers[controllerId].vibration.duration = 0.0f;
    
    #ifdef PLATFORM_WINDOWS
    ApplyVibrationWindows(controllerId);
    #endif
    #ifdef PLATFORM_LINUX
    ApplyVibrationLinux(controllerId);
    #endif
}

void GamepadManager::UpdateVibration(float deltaTime) {
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        VibrationState& vib = controllers[i].vibration;
        if (vib.duration > 0) {
            vib.duration -= deltaTime;
            if (vib.duration <= 0) {
                vib.duration = 0;
                vib.leftMotor = 0.0f;
                vib.rightMotor = 0.0f;
                
                #ifdef PLATFORM_WINDOWS
                ApplyVibrationWindows(i);
                #endif
                #ifdef PLATFORM_LINUX
                ApplyVibrationLinux(i);
                #endif
            }
        }
    }
}

// ============================================================================
// Platform-Specific Vibration
// ============================================================================

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <xinput.h>

void GamepadManager::ApplyVibrationWindows(int controllerId) {
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = static_cast<WORD>(controllers[controllerId].vibration.leftMotor * 65535.0f);
    vibration.wRightMotorSpeed = static_cast<WORD>(controllers[controllerId].vibration.rightMotor * 65535.0f);
    XInputSetState(controllerId, &vibration);
}
#endif

#ifdef PLATFORM_LINUX
// Linux vibration via evdev (simplified - would need actual device path)
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

void GamepadManager::ApplyVibrationLinux(int controllerId) {
    // This is a placeholder for Linux haptic feedback
    // In a real implementation, you would:
    // 1. Find the evdev device for the controller
    // 2. Open it with O_RDWR
    // 3. Use EV_FF to upload and play force feedback effects
    // 4. Close the device when done
    
    // Example (simplified):
    // int fd = open("/dev/input/eventXX", O_RDWR);
    // struct ff_effect effect;
    // effect.type = FF_RUMBLE;
    // effect.u.rumble.strong_magnitude = leftMotor * 0xFFFF;
    // effect.u.rumble.weak_magnitude = rightMotor * 0xFFFF;
    // ioctl(fd, EVIOCSFF, &effect);
    // struct input_event play;
    // play.type = EV_FF;
    // play.code = effect.id;
    // play.value = 1;
    // write(fd, &play, sizeof(play));
    // close(fd);
    
    (void)controllerId; // Suppress unused warning for placeholder
}
#endif

// ============================================================================
// Connection Callbacks
// ============================================================================

void GamepadManager::SetOnConnectCallback(std::function<void(int, ControllerType)> callback) {
    onConnectCallback = callback;
}

void GamepadManager::SetOnDisconnectCallback(std::function<void(int)> callback) {
    onDisconnectCallback = callback;
}

// ============================================================================
// Action Mapping System
// ============================================================================

void GamepadManager::MapAction(GameAction action, const InputBinding& binding) {
    auto it = actionMappings.find(action);
    if (it == actionMappings.end()) {
        ActionMapping mapping(action);
        mapping.bindings.push_back(binding);
        actionMappings[action] = mapping;
    } else {
        it->second.bindings.push_back(binding);
    }
}

void GamepadManager::MapAction(GameAction action, const std::vector<InputBinding>& bindings) {
    auto it = actionMappings.find(action);
    if (it == actionMappings.end()) {
        ActionMapping mapping(action);
        mapping.bindings = bindings;
        actionMappings[action] = mapping;
    } else {
        it->second.bindings.insert(it->second.bindings.end(), bindings.begin(), bindings.end());
    }
}

void GamepadManager::ClearActionMapping(GameAction action) {
    actionMappings.erase(action);
}

void GamepadManager::ResetToDefaultMappings(ControllerType type) {
    actionMappings.clear();
    
    // Movement
    MapAction(GameAction::MoveForward, {InputSourceType::Keyboard, 'W'});
    MapAction(GameAction::MoveBackward, {InputSourceType::Keyboard, 'S'});
    MapAction(GameAction::MoveLeft, {InputSourceType::Keyboard, 'A'});
    MapAction(GameAction::MoveRight, {InputSourceType::Keyboard, 'D'});
    
    // Jump
    MapAction(GameAction::Jump, {InputSourceType::Keyboard, 32}); // Space
    
    // Sprint / Crouch
    MapAction(GameAction::Sprint, {InputSourceType::Keyboard, 340}); // Left Shift
    MapAction(GameAction::Crouch, {InputSourceType::Keyboard, 341}); // Left Control
    
    // Interaction
    MapAction(GameAction::Interact, {InputSourceType::Keyboard, 'E'});
    MapAction(GameAction::Attack, {InputSourceType::MouseButton, 0});
    MapAction(GameAction::Block, {InputSourceType::MouseButton, 1});
    
    // UI
    MapAction(GameAction::Pause, {InputSourceType::Keyboard, 256}); // Escape
    MapAction(GameAction::Inventory, {InputSourceType::Keyboard, 'I'});
    
    // Hotbar
    MapAction(GameAction::Hotbar1, {InputSourceType::Keyboard, '1'});
    MapAction(GameAction::Hotbar2, {InputSourceType::Keyboard, '2'});
    MapAction(GameAction::Hotbar3, {InputSourceType::Keyboard, '3'});
    MapAction(GameAction::Hotbar4, {InputSourceType::Keyboard, '4'});
    MapAction(GameAction::Hotbar5, {InputSourceType::Keyboard, '5'});
    MapAction(GameAction::Hotbar6, {InputSourceType::Keyboard, '6'});
    MapAction(GameAction::Hotbar7, {InputSourceType::Keyboard, '7'});
    MapAction(GameAction::Hotbar8, {InputSourceType::Keyboard, '8'});
    MapAction(GameAction::Hotbar9, {InputSourceType::Keyboard, '9'});
    
    // Gamepad mappings
    int btnA = static_cast<int>(GamepadButton::A);
    int btnB = static_cast<int>(GamepadButton::B);
    int btnX = static_cast<int>(GamepadButton::X);
    int btnY = static_cast<int>(GamepadButton::Y);
    int btnLB = static_cast<int>(GamepadButton::LeftBumper);
    int btnRB = static_cast<int>(GamepadButton::RightBumper);
    int btnSelect = static_cast<int>(GamepadButton::Select);
    int btnStart = static_cast<int>(GamepadButton::Start);
    int btnL3 = static_cast<int>(GamepadButton::LeftStick);
    int btnR3 = static_cast<int>(GamepadButton::RightStick);
    
    int axisLX = static_cast<int>(GamepadAxis::LeftStickX);
    int axisLY = static_cast<int>(GamepadAxis::LeftStickY);
    int axisRX = static_cast<int>(GamepadAxis::RightStickX);
    int axisRY = static_cast<int>(GamepadAxis::RightStickY);
    int axisLT = static_cast<int>(GamepadAxis::LeftTrigger);
    int axisRT = static_cast<int>(GamepadAxis::RightTrigger);
    
    // Gamepad movement (left stick)
    MapAction(GameAction::MoveForward, {InputSourceType::GamepadAxis, axisLY, -1.0f, true});
    MapAction(GameAction::MoveBackward, {InputSourceType::GamepadAxis, axisLY, 1.0f, true});
    MapAction(GameAction::MoveLeft, {InputSourceType::GamepadAxis, axisLX, -1.0f, true});
    MapAction(GameAction::MoveRight, {InputSourceType::GamepadAxis, axisLX, 1.0f, true});
    
    // Gamepad camera (right stick)
    MapAction(GameAction::LookUp, {InputSourceType::GamepadAxis, axisRY, -1.0f, true});
    MapAction(GameAction::LookDown, {InputSourceType::GamepadAxis, axisRY, 1.0f, true});
    MapAction(GameAction::LookLeft, {InputSourceType::GamepadAxis, axisRX, -1.0f, true});
    MapAction(GameAction::LookRight, {InputSourceType::GamepadAxis, axisRX, 1.0f, true});
    
    // Gamepad actions
    MapAction(GameAction::Jump, {InputSourceType::GamepadButton, btnA});
    MapAction(GameAction::Sprint, {InputSourceType::GamepadButton, btnL3});
    MapAction(GameAction::Crouch, {InputSourceType::GamepadButton, btnB});
    MapAction(GameAction::Interact, {InputSourceType::GamepadButton, btnX});
    MapAction(GameAction::Attack, {InputSourceType::GamepadButton, btnRB});
    MapAction(GameAction::Block, {InputSourceType::GamepadButton, btnLB});
    MapAction(GameAction::UseItem, {InputSourceType::GamepadButton, btnY});
    MapAction(GameAction::Pause, {InputSourceType::GamepadButton, btnStart});
    MapAction(GameAction::Inventory, {InputSourceType::GamepadButton, btnSelect});
    
    // PlayStation-specific remappings (if needed)
    if (type == ControllerType::PlayStation) {
        // PlayStation uses same layout as Xbox for most things
        // but could remap here if needed (e.g., different culture button conventions)
    }
}

float GamepadManager::GetActionValue(GameAction action) const {
    auto it = actionMappings.find(action);
    if (it == actionMappings.end()) return 0.0f;
    
    float maxValue = 0.0f;
    
    for (const auto& binding : it->second.bindings) {
        float value = 0.0f;
        
        switch (binding.sourceType) {
            case InputSourceType::Keyboard:
                // Would need keyboard state - placeholder
                break;
            case InputSourceType::MouseButton:
                // Would need mouse state - placeholder
                break;
            case InputSourceType::MouseAxisX:
            case InputSourceType::MouseAxisY:
                // Would need mouse state - placeholder
                break;
            case InputSourceType::GamepadButton: {
                int controllerId = GetActiveController();
                if (controllerId >= 0) {
                    GamepadButton btn = static_cast<GamepadButton>(binding.sourceId);
                    value = IsButtonPressed(controllerId, btn) ? 1.0f : 0.0f;
                }
                break;
            }
            case InputSourceType::GamepadAxis: {
                int controllerId = GetActiveController();
                if (controllerId >= 0) {
                    GamepadAxis axis = static_cast<GamepadAxis>(binding.sourceId);
                    value = GetAxis(controllerId, axis) * binding.axisScale;
                }
                break;
            }
        }
        
        // Use the largest absolute value from all bindings
        if (std::abs(value) > std::abs(maxValue)) {
            maxValue = value;
        }
    }
    
    return maxValue;
}

bool GamepadManager::IsActionPressed(GameAction action) const {
    return std::abs(GetActionValue(action)) > 0.5f;
}

bool GamepadManager::IsActionJustPressed(GameAction action) const {
    // Would need previous frame state tracking for proper implementation
    // This is a simplified version
    return IsActionPressed(action);
}

bool GamepadManager::IsActionReleased(GameAction action) const {
    // Would need previous frame state tracking for proper implementation
    return !IsActionPressed(action);
}

} // namespace vge
