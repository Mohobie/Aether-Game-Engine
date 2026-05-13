#include "core/player_controller.h"
#include "platform/input_manager.h"
#include "voxel/world.h"
#include <cmath>

namespace vge {

PlayerController::PlayerController()
    : position(0, 10, 0), velocity(0, 0, 0), yaw(180), pitch(-30),
      speed(8.0f), jumpForce(12.0f), height(1.8f), radius(0.3f), onGround(false),
      mouseSensitivity(0.15f), gravity(25.0f) {}

PlayerController::~PlayerController() {}

void PlayerController::Update(float deltaTime, Input& input, World& world) {
    // Mouse look
    float dx, dy;
    input.GetMouseDelta(dx, dy);
    if (dx != 0 || dy != 0) {
        yaw += dx * mouseSensitivity;
        pitch -= dy * mouseSensitivity;
        // Clamp pitch to prevent flipping
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
    input.ResetMouseDelta();
    
    // Get look direction
    Vec3 forward = GetLookDirection();
    Vec3 right = GetRightDirection();
    
    // Movement input
    Vec3 moveDir(0, 0, 0);
    
    if (input.IsKeyPressed(KeyCode::W)) {
        moveDir = moveDir + forward;
    }
    if (input.IsKeyPressed(KeyCode::S)) {
        moveDir = moveDir - forward;
    }
    if (input.IsKeyPressed(KeyCode::A)) {
        moveDir = moveDir - right;
    }
    if (input.IsKeyPressed(KeyCode::D)) {
        moveDir = moveDir + right;
    }
    
    // Normalize horizontal movement (keep y=0 for ground movement)
    moveDir.y = 0;
    float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (len > 0.001f) {
        moveDir.x /= len;
        moveDir.z /= len;
    }
    
    // Apply movement
    float moveSpeed = input.IsKeyPressed(KeyCode::Shift) ? speed * 1.5f : speed;
    velocity.x = moveDir.x * moveSpeed;
    velocity.z = moveDir.z * moveSpeed;
    
    // Jumping
    if (input.IsKeyJustPressed(KeyCode::Space) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }
    
    // Apply gravity
    velocity.y -= gravity * deltaTime;
    
    // Apply velocity with collision detection
    Vec3 newPos = position;
    
    // Try X movement
    newPos.x = position.x + velocity.x * deltaTime;
    if (!CheckCollision(newPos, world)) {
        position.x = newPos.x;
    } else {
        velocity.x = 0;
    }
    
    // Try Z movement
    newPos.x = position.x; // Reset X, keep Z change
    newPos.z = position.z + velocity.z * deltaTime;
    if (!CheckCollision(newPos, world)) {
        position.z = newPos.z;
    } else {
        velocity.z = 0;
    }
    
    // Try Y movement
    newPos.z = position.z;
    newPos.y = position.y + velocity.y * deltaTime;
    if (!CheckCollision(newPos, world)) {
        position.y = newPos.y;
        onGround = false;
    } else {
        if (velocity.y < 0) {
            onGround = true;
        }
        velocity.y = 0;
    }
    
    // Keep player above void
    if (position.y < -50) {
        position.y = 50;
        velocity.y = 0;
    }
}

bool PlayerController::CheckCollision(const Vec3& pos, World& world) {
    // Check collision with blocks around player
    int minX = static_cast<int>(std::floor(pos.x - radius));
    int maxX = static_cast<int>(std::floor(pos.x + radius));
    int minY = static_cast<int>(std::floor(pos.y));
    int maxY = static_cast<int>(std::floor(pos.y + height));
    int minZ = static_cast<int>(std::floor(pos.z - radius));
    int maxZ = static_cast<int>(std::floor(pos.z + radius));
    
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            for (int z = minZ; z <= maxZ; z++) {
                BlockTypeID block = world.GetBlock(x, y, z);
                if (block != BlockRegistry::GetInstance().GetBlockId("air") &&
                    block != BlockRegistry::GetInstance().GetBlockId("water")) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool PlayerController::PlaceBlock(World& world, BlockTypeID type) {
    Vec3 lookDir = GetLookDirection();
    Vec3 eyePos = position + Vec3(0, height * 0.8f, 0);
    
    // Ray cast to find block placement position
    float maxDist = 5.0f;
    float step = 0.1f;
    
    Vec3 prevPos = eyePos;
    for (float t = 0; t < maxDist; t += step) {
        Vec3 checkPos = eyePos + lookDir * t;
        int bx = static_cast<int>(std::floor(checkPos.x));
        int by = static_cast<int>(std::floor(checkPos.y));
        int bz = static_cast<int>(std::floor(checkPos.z));
        
        BlockTypeID block = world.GetBlock(bx, by, bz);
        if (block != BlockRegistry::GetInstance().GetBlockId("air")) {
            // Place block in front of this block
            int px = static_cast<int>(std::floor(prevPos.x));
            int py = static_cast<int>(std::floor(prevPos.y));
            int pz = static_cast<int>(std::floor(prevPos.z));
            
            // Don't place inside player
            if (std::abs(px - static_cast<int>(std::floor(position.x))) <= 1 &&
                std::abs(py - static_cast<int>(std::floor(position.y))) <= 2 &&
                std::abs(pz - static_cast<int>(std::floor(position.z))) <= 1) {
                return false;
            }
            
            world.SetBlock(px, py, pz, type);
            return true;
        }
        prevPos = checkPos;
    }
    return false;
}

bool PlayerController::BreakBlock(World& world) {
    Vec3 lookDir = GetLookDirection();
    Vec3 eyePos = position + Vec3(0, height * 0.8f, 0);
    
    // Ray cast to find block to break
    float maxDist = 5.0f;
    float step = 0.1f;
    
    for (float t = 0; t < maxDist; t += step) {
        Vec3 checkPos = eyePos + lookDir * t;
        int bx = static_cast<int>(std::floor(checkPos.x));
        int by = static_cast<int>(std::floor(checkPos.y));
        int bz = static_cast<int>(std::floor(checkPos.z));
        
        BlockTypeID block = world.GetBlock(bx, by, bz);
        if (block != BlockRegistry::GetInstance().GetBlockId("air")) {
            world.SetBlock(bx, by, bz, BlockRegistry::GetInstance().GetBlockId("air"));
            return true;
        }
    }
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

Vec3 PlayerController::GetRightDirection() const {
    float yawRad = yaw * 3.14159f / 180.0f;
    return Vec3(
        cosf(yawRad),
        0,
        -sinf(yawRad)
    );
}

} // namespace vge
