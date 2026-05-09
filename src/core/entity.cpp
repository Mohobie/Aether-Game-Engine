#include "entity.h"
#include <iostream>

namespace vge {

// Entity factory for common game objects
Entity* Entity::CreatePlayer(const Vec3& position) {
    Entity* player = new Entity("Player");
    player->SetPosition(position);
    
    auto transform = player->AddComponent<TransformComponent>();
    transform->position = position;
    
    auto physics = player->AddComponent<PhysicsComponent>();
    physics->mass = 70.0f; // 70kg player
    physics->useGravity = true;
    
    auto render = player->AddComponent<RenderComponent>();
    render->color = Vec3(0.2f, 0.6f, 1.0f); // Blue player
    
    return player;
}

Entity* Entity::CreateBlockEntity(const Vec3& position, BlockType type) {
    Entity* block = new Entity("Block");
    block->SetPosition(position);
    
    auto transform = block->AddComponent<TransformComponent>();
    transform->position = position;
    transform->scale = Vec3(0.5f, 0.5f, 0.5f); // Half size
    
    auto physics = block->AddComponent<PhysicsComponent>();
    physics->isStatic = true; // Blocks don't move
    physics->useGravity = false;
    
    auto render = block->AddComponent<RenderComponent>();
    // Color based on block type
    switch (type) {
        case BlockType::Grass: render->color = Vec3(0.2f, 0.8f, 0.2f); break;
        case BlockType::Dirt: render->color = Vec3(0.55f, 0.27f, 0.07f); break;
        case BlockType::Stone: render->color = Vec3(0.5f, 0.5f, 0.5f); break;
        case BlockType::Wood: render->color = Vec3(0.6f, 0.4f, 0.2f); break;
        default: render->color = Vec3(1.0f, 1.0f, 1.0f); break;
    }
    
    return block;
}

Entity* Entity::CreateCameraEntity(const Vec3& position) {
    Entity* camera = new Entity("Camera");
    camera->SetPosition(position);
    
    auto transform = camera->AddComponent<TransformComponent>();
    transform->position = position;
    
    // Camera doesn't have physics or render component
    
    return camera;
}

} // namespace vge
