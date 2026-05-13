#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"

namespace vge {

class Input;
class World;

class PlayerController {
private:
    Vec3 position;
    Vec3 velocity;
    float yaw, pitch;
    float speed;
    float jumpForce;
    float height;
    float radius;
    bool onGround;
    
    Vec3 GetLookDirection() const;
    Vec3 GetRightDirection() const;
    
    float mouseSensitivity;
    float gravity;
    
public:
    PlayerController();
    ~PlayerController();
    
    void Update(float deltaTime, Input& input, World& world);
    bool CheckCollision(const Vec3& pos, World& world);
    
    // Block interaction
    bool PlaceBlock(World& world, BlockTypeID type);
    bool BreakBlock(World& world);
    
    Vec3 GetPosition() const;
    void SetPosition(const Vec3& pos);
    float GetYaw() const;
    float GetPitch() const;
};

} // namespace vge
