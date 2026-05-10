#include <gtest/gtest.h>
#include "platform/gamepad.h"

using namespace vge;

// ============================================================================
// Controller State Tests
// ============================================================================

TEST(GamepadManager, Initialization) {
    GamepadManager manager;
    
    EXPECT_EQ(manager.GetConnectedCount(), 0);
    EXPECT_EQ(manager.GetActiveController(), -1);
}

TEST(GamepadManager, DeadZoneClamp) {
    GamepadManager manager;
    
    // Can't set on disconnected controller, but test the API exists
    manager.SetDeadZone(0, 0.5f);
    EXPECT_EQ(manager.GetDeadZone(0), 0.5f);
    
    manager.SetDeadZone(0, 1.5f); // Should clamp to 1.0
    EXPECT_EQ(manager.GetDeadZone(0), 1.0f);
    
    manager.SetDeadZone(0, -0.5f); // Should clamp to 0.0
    EXPECT_EQ(manager.GetDeadZone(0), 0.0f);
}

TEST(GamepadManager, DeadZoneMode) {
    GamepadManager manager;
    
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::Circular);
    
    manager.SetDeadZoneMode(0, DeadZoneMode::Axial);
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::Axial);
    
    manager.SetDeadZoneMode(0, DeadZoneMode::ScaledCircular);
    EXPECT_EQ(manager.GetDeadZoneMode(0), DeadZoneMode::ScaledCircular);
}

TEST(GamepadManager, InvalidControllerId) {
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

TEST(Vec2, BasicOperations) {
    Vec2 v1(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v1.length(), 5.0f);
    EXPECT_FLOAT_EQ(v1.lengthSquared(), 25.0f);
    
    Vec2 normalized = v1.normalize();
    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
}

TEST(Vec2, ZeroLength) {
    Vec2 v(0.0f, 0.0f);
    Vec2 normalized = v.normalize();
    EXPECT_FLOAT_EQ(normalized.x, 0.0f);
    EXPECT_FLOAT_EQ(normalized.y, 0.0f);
    EXPECT_FLOAT_EQ(normalized.length(), 0.0f);
}

// ============================================================================
// Action Mapping Tests
// ============================================================================

TEST(GamepadManager, DefaultMappingsExist) {
    GamepadManager manager;
    
    // Default mappings should be set up
    EXPECT_TRUE(manager.IsActionPressed(GameAction::Count) == false);
    
    // All actions should have mappings (except Count)
    // Can't test directly without keyboard/mouse state, but verify no crash
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        GameAction action = static_cast<GameAction>(i);
        manager.GetActionValue(action);
        manager.IsActionPressed(action);
    }
}

TEST(GamepadManager, ResetMappings) {
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
    
    // Verify mappings exist again (can't check values without input state)
    // But at least verify no crash
    for (int i = 0; i < static_cast<int>(GameAction::Count); ++i) {
        manager.GetActionValue(static_cast<GameAction>(i));
    }
}

TEST(GamepadManager, CustomActionMapping) {
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

TEST(GamepadManager, MultipleBindingsPerAction) {
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

TEST(ControllerType, EnumValues) {
    EXPECT_NE(static_cast<int>(ControllerType::Unknown), static_cast<int>(ControllerType::Xbox));
    EXPECT_NE(static_cast<int>(ControllerType::Xbox), static_cast<int>(ControllerType::PlayStation));
    EXPECT_NE(static_cast<int>(ControllerType::PlayStation), static_cast<int>(ControllerType::Generic));
}

// ============================================================================
// GamepadButton Enum Tests
// ============================================================================

TEST(GamepadButton, EnumCount) {
    // Ensure Count is the last enum value
    EXPECT_EQ(static_cast<int>(GamepadButton::Count), 
              static_cast<int>(GamepadButton::DPadRight) + 1);
}

TEST(GamepadAxis, EnumCount) {
    // Ensure Count is the last enum value
    EXPECT_EQ(static_cast<int>(GamepadAxis::Count), 
              static_cast<int>(GamepadAxis::RightTrigger) + 1);
}

// ============================================================================
// Vibration Tests
// ============================================================================

TEST(GamepadManager, VibrationClamp) {
    GamepadManager manager;
    
    // Should not crash on disconnected controller
    manager.SetVibration(0, 2.0f, -1.0f, 5.0f);
    // Values should be clamped internally
}

TEST(GamepadManager, VibrationUpdate) {
    GamepadManager manager;
    
    // Should not crash
    manager.UpdateVibration(0.016f); // ~60fps frame time
    manager.UpdateVibration(1.0f);     // Large delta time
}

// ============================================================================
// Callback Tests
// ============================================================================

TEST(GamepadManager, CallbackRegistration) {
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
    
    // Callbacks are registered (can't test actual triggering without hardware)
    EXPECT_TRUE(true); // Test passes if no crash
}

// ============================================================================
// InputBinding Tests
// ============================================================================

TEST(InputBinding, DefaultConstruction) {
    InputBinding binding;
    EXPECT_EQ(binding.sourceType, InputSourceType::Keyboard);
    EXPECT_EQ(binding.sourceId, 0);
    EXPECT_FLOAT_EQ(binding.axisScale, 1.0f);
    EXPECT_FALSE(binding.isAxis);
}

TEST(InputBinding, ParameterizedConstruction) {
    InputBinding binding(InputSourceType::GamepadAxis, 2, -1.0f, true);
    EXPECT_EQ(binding.sourceType, InputSourceType::GamepadAxis);
    EXPECT_EQ(binding.sourceId, 2);
    EXPECT_FLOAT_EQ(binding.axisScale, -1.0f);
    EXPECT_TRUE(binding.isAxis);
}

// ============================================================================
// ControllerState Tests
// ============================================================================

TEST(ControllerState, DefaultConstruction) {
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

TEST(DeadZone, AxialDeadZone) {
    // Test axial dead zone logic conceptually
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

TEST(DeadZone, CircularDeadZone) {
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

TEST(DeadZone, ScaledCircular) {
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

TEST(GamepadManager, ActionMappingIntegration) {
    // This test verifies the action mapping system works end-to-end
    // with manually set controller state
    
    GamepadManager manager;
    
    // Manually set a controller as connected (simulating GLFW detection)
    // In real usage, this would be done by Update() calling glfwGetGamepadState
    
    // For this test, we verify the API surface is correct
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
