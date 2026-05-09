#include "player_controller.h"
#include "voxel/world.h"
#include "voxel/block.h"
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
    if (input.IsMouseButtonPressed(0)) { // Left button for look
        // In real implementation, would use mouse delta
        // For now, arrow keys control look
        if (input.IsKeyPressed(GLFW_KEY_UP)) pitch += 2.0f;
        if (input.IsKeyPressed(GLFW_KEY_DOWN)) pitch -= 2.0f;
        if (input.IsKeyPressed(GLFW_KEY_LEFT)) yaw -= 2.0f;
        if (input.IsKeyPressed(GLFW_KEY_RIGHT)) yaw += 2.0f;
    }
    
    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    // Calculate forward/right vectors
    Vec3 forward;
    forward.x = cosf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    forward.y = sinf(pitch * 3.14159f / 180.0f);
    forward.z = sinf(yaw * 3.14159f / 180.0f) * cosf(pitch * 3.14159f / 180.0f);
    forward = forward.Normalized();
    
    Vec3 right = forward.Cross(Vec3(0, 1, 0)).Normalized();
    Vec3 up(0, 1, 0);
    
    // Movement
    Vec3 moveDir(0, 0, 0);
    if (input.IsKeyPressed(GLFW_KEY_W)) moveDir = moveDir + forward;
    if (input.IsKeyPressed(GLFW_KEY_S)) moveDir = moveDir - forward;
    if (input.IsKeyPressed(GLFW_KEY_A)) moveDir = moveDir - right;
    if (input.IsKeyPressed(GLFW_KEY_D)) moveDir = moveDir + right;
    
    // Normalize and apply speed
    if (moveDir.Length() > 0) {
        moveDir = moveDir.Normalized() * speed * deltaTime;
    }
    
    // Jump
    if (input.IsKeyPressed(GLFW_KEY_SPACE) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }
    
    // Gravity
    velocity.y -= 20.0f * deltaTime; // Gravity
    
    // Apply velocity
    moveDir.y = velocity.y * deltaTime;
    
    // Try to move, with collision
    Vec3 newPos = position + moveDir;
    
    // Simple collision check
    if (!CheckCollision(newPos, world)) {
        position = newPos;
        onGround = false;
    } else {
        // Try moving only horizontally
        Vec3 horizontalMove(moveDir.x, 0, moveDir.z);
        Vec3 horizontalPos = position + horizontalMove;
        if (!CheckCollision(horizontalPos, world)) {
            position = horizontalPos;
        }
        
        // Check if on ground
        if (velocity.y < 0) {
            Vec3 groundCheck = position + Vec3(0, -0.1f, 0);
            if (CheckCollision(groundCheck, world)) {
                onGround = true;
                velocity.y = 0;
            }
        }
    }
    
    // Block interaction
    if (input.IsKeyJustPressed(GLFW_KEY_E)) {
        // Place block
        Raycast raycast;
        Vec3 lookDir = forward;
        if (raycast.PlaceBlock(position, lookDir, world, BlockType::Stone)) {
            std::cout << "[Player] Placed block" << std::endl;
        }
    }
    
    if (input.IsKeyJustPressed(GLFW_KEY_Q)) {
        // Remove block
        Raycast raycast;
        Vec3 lookDir = forward;
        if (raycast.RemoveBlock(position, lookDir, world)) {
            std::cout << "[Player] Removed block" << std::endl;
        }
    }
}

bool PlayerController::CheckCollision(const Vec3& pos, World& world) {
    // Check if the player's bounding box intersects with any solid blocks
    int minX = (int)(pos.x - radius);
    int maxX = (int)(pos.x + radius);
    int minY = (int)(pos.y);
    int maxY = (int)(pos.y + height);
    int minZ = (int)(pos.z - radius);
    int maxZ = (int)(pos.z + radius);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                BlockType block = world.GetBlock(x, y, z);
                if (block != BlockType::Air && BlockRegistry::GetInstance().GetBlock(block).IsSolid()) {
                    return true; // Collision
                }
            }
        }
    }
    
    return false;
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