#include "core/player_controller.h"
#include "voxel/world.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "physics/collision.h"
#include "platform/input.h"
#include "core/raycast.h"
#include <iostream>

namespace vge {

PlayerController::PlayerController() 
    : position(0, 40, 0), velocity(0, 0, 0),
      yaw(0), pitch(0), speed(5.0f), jumpForce(8.0f),
      height(1.8f), radius(0.3f), onGround(false) {}

PlayerController::~PlayerController() {}

void PlayerController::Update(float deltaTime, Input& input, World& world) {
    // Mouse look
    if (input.IsKeyPressed(GLFW_KEY_UP)) pitch += 2.0f;
    if (input.IsKeyPressed(GLFW_KEY_DOWN)) pitch -= 2.0f;
    if (input.IsKeyPressed(GLFW_KEY_LEFT)) yaw -= 2.0f;
    if (input.IsKeyPressed(GLFW_KEY_RIGHT)) yaw += 2.0f;
    
    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    // Calculate forward/right vectors
    Vec3 forward;
    forward.x = cosf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    forward.y = sinf(pitch * 3.14159f / 180.0f);
    forward.z = sinf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    forward = forward.normalize();
    
    Vec3 right = forward.cross(Vec3(0, 1, 0)).normalize();
    
    // Movement
    Vec3 moveDir(0, 0, 0);
    if (input.IsKeyPressed(GLFW_KEY_W)) moveDir = moveDir + forward;
    if (input.IsKeyPressed(GLFW_KEY_S)) moveDir = moveDir - forward;
    if (input.IsKeyPressed(GLFW_KEY_A)) moveDir = moveDir - right;
    if (input.IsKeyPressed(GLFW_KEY_D)) moveDir = moveDir + right;
    
    // Normalize and apply speed
    if (moveDir.length() > 0) {
        moveDir = moveDir.normalize() * speed * deltaTime;
    }
    
    // Jump
    if (input.IsKeyJustPressed(GLFW_KEY_SPACE) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }
    
    // Block interaction with sound
    if (input.IsKeyJustPressed(GLFW_KEY_E)) { // Place block
        if (PlaceBlock(world, BlockRegistry::GetInstance().GetBlockId("wood"))) {
            // Sound played by engine
        }
    }
    if (input.IsKeyJustPressed(GLFW_KEY_Q)) { // Break block
        if (BreakBlock(world)) {
            // Sound played by engine
        }
    }
    
    // Gravity
    velocity.y -= 20.0f * deltaTime;
    
    // Apply velocity
    moveDir.y = velocity.y * deltaTime;
    
    // Try to move
    Vec3 newPos = position + moveDir;
    if (!CheckCollision(newPos, world)) {
        position = newPos;
        onGround = false;
    } else {
        velocity.y = 0;
        onGround = true;
    }
}

bool PlayerController::CheckCollision(const Vec3& pos, World& world) {
    int minX = (int)(pos.x - radius);
    int maxX = (int)(pos.x + radius);
    int minY = (int)(pos.y);
    int maxY = (int)(pos.y + height);
    int minZ = (int)(pos.z - radius);
    int maxZ = (int)(pos.z + radius);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                BlockTypeID block = world.GetBlock(x, y, z);
                if (block != BlockRegistry::GetInstance().GetBlockId("air") && BlockRegistry::GetInstance().GetBlock(block).IsSolid()) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool PlayerController::PlaceBlock(World& world, BlockTypeID type) {
    Vec3 eyePos = position + Vec3(0, 1.6f, 0);
    Vec3 lookDir = GetLookDirection();
    
    Raycast raycast;
    return raycast.PlaceBlock(eyePos, lookDir, world, type);
}

bool PlayerController::BreakBlock(World& world) {
    Vec3 eyePos = position + Vec3(0, 1.6f, 0);
    Vec3 lookDir = GetLookDirection();
    
    Raycast raycast;
    return raycast.RemoveBlock(eyePos, lookDir, world);
}

Vec3 PlayerController::GetLookDirection() const {
    Vec3 dir;
    dir.x = cosf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    dir.y = sinf(pitch * 3.14159f / 180.0f);
    dir.z = sinf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    return dir.normalize();
}

Vec3 PlayerController::GetPosition() const {
    return position;
}

void PlayerController::SetPosition(const Vec3& pos) {
    position = pos;
}

float PlayerController::GetYaw() const {
    return yaw;
}

float PlayerController::GetPitch() const {
    return pitch;
}

} // namespace vge
