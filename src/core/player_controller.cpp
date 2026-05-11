#include "core/player_controller.h"
#include "platform/input_manager.h"
#include "voxel/world.h"
#include <cmath>

namespace vge {

PlayerController::PlayerController()
    : position(0, 0, 0), velocity(0, 0, 0), yaw(0), pitch(0),
      speed(5.0f), jumpForce(8.0f), height(1.8f), radius(0.3f), onGround(false) {}

PlayerController::~PlayerController() {}

void PlayerController::Update(float deltaTime, Input& input, World& world) {
    (void)deltaTime; (void)input; (void)world;
    // Stub implementation
}

bool PlayerController::CheckCollision(const Vec3& pos, World& world) {
    (void)pos; (void)world;
    return false;
}

bool PlayerController::PlaceBlock(World& world, BlockTypeID type) {
    (void)world; (void)type;
    return false;
}

bool PlayerController::BreakBlock(World& world) {
    (void)world;
    return false;
}

Vec3 PlayerController::GetPosition() const { return position; }
void PlayerController::SetPosition(const Vec3& pos) { position = pos; }
float PlayerController::GetYaw() const { return yaw; }
float PlayerController::GetPitch() const { return pitch; }

Vec3 PlayerController::GetLookDirection() const {
    float yawRad = yaw * 3.14159f / 180.0f;
    float pitchRad = pitch * 3.14159f / 180.0f;
    return Vec3(
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)
    );
}

} // namespace vge
