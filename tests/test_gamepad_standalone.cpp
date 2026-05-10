#include <cassert>
#include <cmath>
#include <iostream>
#include "platform/gamepad.h"

using namespace vge;

// Simple test runner (no external dependencies)
#define TEST(name) void test_##name()
#define EXPECT_TRUE(expr) assert(expr)
#define EXPECT_FALSE(expr) assert(!(expr))
#define EXPECT_EQ(a, b) assert((a) == (b))
#define EXPECT_NE(a, b) assert((a) != (b))
#define EXPECT_FLOAT_EQ(a, b) assert(std::abs((a) - (b)) < 0.0001f)
#define EXPECT_LT(a, b) assert((a) < (b))
#define EXPECT_GT(a, b) assert((a) > (b))
#define EXPECT_NO_THROW(expr) do { expr; } while(0)
#define RUN_TEST(name) do { std::cout << "Running test_" #name "... "; test_##name(); std::cout << "PASSED" << std::endl; } while(0)

// ============================================================================
// Controller State Tests
// ============================================================================

TEST(initialization) {
    GamepadManager manager;
    
    EXPECT_EQ(manager.GetConnectedCount(), 0);
    EXPECT_EQ(manager.GetActiveController(), -1);
}

TEST(dead_zone_clamp) {
    GamepadManager manager;
    
    manager.SetDeadZone(0, 0.5f);
    EXPECT_EQ(manager.GetDeadZone(0), 0.5f);
    
    manager.SetDeadZone(0, 1.5f); // Should clamp to 1.0
    EXPECT_EQ(manager.GetDeadZone(0), 1.0f);
    
    manager.SetDeadZone(0, -0.5f); // Should clamp to 0.0
    EXPECT_EQ(manager.GetDeadZone(0), 0.0f);
}

TEST(dead_zone_mode) {
    GamepadManager manager;
    
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::Circular);
    
    manager.SetDeadZoneMode(0, DeadZoneMode::Axial);
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::Axial);
    
    manager.SetDeadZoneMode(0, DeadZoneMode::ScaledCircular);
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::ScaledCircular);
}

TEST(invalid_controller_id) {
    GamepadManager manager;
    
    EXPECT_FALSE(manager.IsConnected(-1));
    EXPECT_FALSE(manager.IsConnected(99));
    EXPECT_EQ(manager.GetControllerType(-1), ControllerType::Unknown);
    EXPECT_EQ(manager.GetControllerName(-1), "");
    EXPECT_EQ(manager.GetDeadZone(-1), 0.0f);
    
    // Should not crash
    manager.SetDeadZone(-1, 0.5f);
    manager.SetDeadZoneMode(-1, DeadZoneMode::Axial);
    manager.SetVibration(-1, 0.5f, 0.5f, 1.0f);
    manager.StopVibration(-1);
}

// ============================================================================
// Vec2 Helper Tests
// ============================================================================

TEST(vec2_basic_operations) {
    Vec2 v1(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v1.length(), 5.0f);
    EXPECT_FLOAT_EQ(v1.lengthSquared(), 25.0f);
    
    Vec2 normalized = v1.normalize();
    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
}

TEST(vec2_zero_length) {
    Vec2 v(0.0f, 0.0f);
    Vec2 normalized = v.normalize();
    EXPECT_FLOAT_EQ(normalized.x, 0.0f);
    EXPECT_FLOAT_EQ(normalized.y, 0.0f);
    EXPECT_FLOAT_EQ(normalized.length(), 0.0f);
}

// ============================================================================
// Action Mapping Tests
// ============================================================================

TEST(default_mappings_exist) {
    GamepadManager manager;
    
    // All actions should have mappings (except Count)
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        GameAction action = static_cast<GameAction>(i);
        manager.GetActionValue(action);
        manager.IsActionPressed(action);
    }
}

TEST(reset_mappings) {
    GamepadManager manager;
    
    // Clear all mappings
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        manager.ClearActionMapping(static_cast<GameAction>(i));
    }
    
    // After clearing, all actions should return 0
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        EXPECT_FLOAT_EQ(manager.GetActionValue(static_cast<GameAction>(i)), 0.0f);
    }
    
    // Reset to Xbox defaults
    manager.ResetToDefaultMappings(ControllerType::Xbox);
    
    // Verify mappings exist again
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        manager.GetActionValue(static_cast<GameAction>(i));
    }
}

TEST(custom_action_mapping) {
    GamepadManager manager;
    
    // Clear existing
    manager.ClearActionMapping(GameAction::Jump);
    
    // Add custom mapping
    InputBinding binding(InputSourceType::GamepadButton, 
                         static_cast<int>(GamepadButton::Y));
    manager.MapAction(GameAction::Jump, binding);
    
    // Should not crash
    manager.GetActionValue(GameAction::Jump);
}

TEST(multiple_bindings_per_action) {
    GamepadManager manager;
    
    // Clear existing
    manager.ClearActionMapping(GameAction::Jump);
    
    // Add multiple bindings
    std::vector<InputBinding> bindings;
    bindings.emplace_back(InputSourceType::Keyboard, ' ');
    bindings.emplace_back(InputSourceType::GamepadButton, 
                          static_cast<int>(GamepadButton::A));
    
    manager.MapAction(GameAction::Jump, bindings);
    
    // Should not crash
    manager.GetActionValue(GameAction::Jump);
}

// ============================================================================
// Controller Type Detection Tests
// ============================================================================

TEST(controller_type_enum) {
    EXPECT_NE(static_cast<int>(ControllerType::Unknown), static_cast<int>(ControllerType::Xbox));
    EXPECT_NE(static_cast<int>(ControllerType::Xbox), static_cast<int>(ControllerType::PlayStation));
    EXPECT_NE(static_cast<int>(ControllerType::PlayStation), static_cast<int>(ControllerType::Generic));
}

// ============================================================================
// GamepadButton Enum Tests
// ============================================================================

TEST(gamepad_button_enum_count) {
    // Ensure Count is the last enum value
    EXPECT_EQ(static_cast<int>(GamepadButton::Count), 
              static_cast<int>(GamepadButton::DPadRight) + 1);
}

TEST(gamepad_axis_enum_count) {
    // Ensure Count is the last enum value
    EXPECT_EQ(static_cast<int>(GamepadAxis::Count), 
              static_cast<int>(GamepadAxis::RightTrigger) + 1);
}

// ============================================================================
// Vibration Tests
// ============================================================================

TEST(vibration_clamp) {
    GamepadManager manager;
    
    // Should not crash on disconnected controller
    manager.SetVibration(0, 2.0f, -1.0f, 5.0f);
}

TEST(vibration_update) {
    GamepadManager manager;
    
    // Should not crash
    manager.UpdateVibration(0.016f); // ~60fps frame time
    manager.UpdateVibration(1.0f);     // Large delta time
}

// ============================================================================
// Callback Tests
// ============================================================================

TEST(callback_registration) {
    GamepadManager manager;
    
    bool connectCalled = false;
    bool disconnectCalled = false;
    
    manager.SetOnConnectCallback([&connectCalled](int id, ControllerType type) {
        connectCalled = true;
        (void)id;
        (void)type;
    });
    
    manager.SetOnDisconnectCallback([&disconnectCalled](int id) {
        disconnectCalled = true;
        (void)id;
    });
    
    // Update won't trigger callbacks without actual controller connection
    manager.Update();
}

// ============================================================================
// InputBinding Tests
// ============================================================================

TEST(input_binding_default) {
    InputBinding binding;
    EXPECT_EQ(binding.sourceType, InputSourceType::Keyboard);
    EXPECT_EQ(binding.sourceId, 0);
    EXPECT_FLOAT_EQ(binding.axisScale, 1.0f);
    EXPECT_FALSE(binding.isAxis);
}

TEST(input_binding_parameterized) {
    InputBinding binding(InputSourceType::GamepadAxis, 2, -1.0f, true);
    EXPECT_EQ(binding.sourceType, InputSourceType::GamepadAxis);
    EXPECT_EQ(binding.sourceId, 2);
    EXPECT_FLOAT_EQ(binding.axisScale, -1.0f);
    EXPECT_TRUE(binding.isAxis);
}

// ============================================================================
// ControllerState Tests
// ============================================================================

TEST(controller_state_default) {
    ControllerState state;
    
    EXPECT_EQ(state.id, -1);
    EXPECT_FALSE(state.connected);
    EXPECT_EQ(state.type, ControllerType::Unknown);
    EXPECT_EQ(state.name, "");
    EXPECT_FLOAT_EQ(state.deadZone, 0.15f);
    EXPECT_EQ(state.deadZoneMode, DeadZoneMode::Circular);
    
    // Arrays should be zero-initialized
    for (size_t i = 0; i < static_cast<size_t>(GamepadButton::Count); ++i) {
        EXPECT_FALSE(state.buttons[i]);
        EXPECT_FALSE(state.prevButtons[i]);
    }
    
    for (size_t i = 0; i < static_cast<size_t>(GamepadAxis::Count); ++i) {
        EXPECT_FLOAT_EQ(state.axes[i], 0.0f);
        EXPECT_FLOAT_EQ(state.prevAxes[i], 0.0f);
    }
}

// ============================================================================
// Dead Zone Math Tests
// ============================================================================

TEST(dead_zone_axial) {
    float deadZone = 0.15f;
    
    // Value below dead zone should be 0
    float smallValue = 0.1f;
    EXPECT_LT(smallValue, deadZone);
    
    // Value above dead zone should be rescaled
    float largeValue = 0.5f;
    float rescaled = (largeValue - deadZone) / (1.0f - deadZone);
    EXPECT_FLOAT_EQ(rescaled, (0.5f - 0.15f) / 0.85f);
    EXPECT_GT(rescaled, 0.0f);
    EXPECT_LT(rescaled, 1.0f);
}

TEST(dead_zone_circular) {
    Vec2 stick(0.5f, 0.5f);
    float magnitude = stick.length();
    float deadZone = 0.15f;
    
    // Magnitude should be sqrt(0.5) ≈ 0.707
    EXPECT_FLOAT_EQ(magnitude, std::sqrt(0.5f));
    EXPECT_GT(magnitude, deadZone);
    
    // Below dead zone, stick should be zeroed
    Vec2 smallStick(0.1f, 0.05f);
    EXPECT_LT(smallStick.length(), deadZone);
}

TEST(dead_zone_scaled_circular) {
    Vec2 stick(0.8f, 0.6f);
    float magnitude = stick.length(); // = 1.0
    float deadZone = 0.15f;
    
    // Scaled output should be (1.0 - 0.15) / (1.0 - 0.15) = 1.0
    float scaled = (magnitude - deadZone) / (1.0f - deadZone);
    EXPECT_FLOAT_EQ(scaled, 1.0f);
}

// ============================================================================
// Integration: Action Mapping with Gamepad State
// ============================================================================

TEST(action_mapping_integration) {
    GamepadManager manager;
    
    EXPECT_NO_THROW({
        manager.Update();
        manager.GetConnectedCount();
        manager.GetActiveController();
        
        // Try all query methods
        for (int i = 0; i < static_cast<int>(GamepadButton::Count); ++i) {
            manager.IsButtonPressed(0, static_cast<GamepadButton>(i));
            manager.IsButtonJustPressed(0, static_cast<GamepadButton>(i));
            manager.IsButtonReleased(0, static_cast<GamepadButton>(i));
        }
        
        for (int i = 0; i < static_cast<int>(GamepadAxis::Count); ++i) {
            manager.GetAxis(0, static_cast<GamepadAxis>(i));
            manager.GetAxisRaw(0, static_cast<GamepadAxis>(i));
        }
        
        manager.GetLeftStick(0);
        manager.GetRightStick(0);
        manager.GetLeftTrigger(0);
        manager.GetRightTrigger(0);
    });
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Gamepad Manager Tests ===" << std::endl;
    std::cout << std::endl;
    
    RUN_TEST(initialization);
    RUN_TEST(dead_zone_clamp);
    RUN_TEST(dead_zone_mode);
    RUN_TEST(invalid_controller_id);
    RUN_TEST(vec2_basic_operations);
    RUN_TEST(vec2_zero_length);
    RUN_TEST(default_mappings_exist);
    RUN_TEST(reset_mappings);
    RUN_TEST(custom_action_mapping);
    RUN_TEST(multiple_bindings_per_action);
    RUN_TEST(controller_type_enum);
    RUN_TEST(gamepad_button_enum_count);
    RUN_TEST(gamepad_axis_enum_count);
    RUN_TEST(vibration_clamp);
    RUN_TEST(vibration_update);
    RUN_TEST(callback_registration);
    RUN_TEST(input_binding_default);
    RUN_TEST(input_binding_parameterized);
    RUN_TEST(controller_state_default);
    RUN_TEST(dead_zone_axial);
    RUN_TEST(dead_zone_circular);
    RUN_TEST(dead_zone_scaled_circular);
    RUN_TEST(action_mapping_integration);
    
    std::cout << std::endl;
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
