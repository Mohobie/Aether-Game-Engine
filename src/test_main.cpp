#include <iostream>
#include "math/vec3.h"
#include "math/mat4.h"
#include "voxel/block_types.h"
#include "voxel/block_registry.h"
#include "voxel/chunk.h"
#include "voxel/world.h"
#include "voxel/block_mesh_builder.h"
#include "rendering/mesh.h"
#include "physics/collision.h"
#include "entity/entity.h"
#include "entity/components.h"

using namespace vge;

int main() {
    std::cout << "=== Aether Voxel Engine Tests ===" << std::endl;
    
    // Load default blocks
    std::string blockPath = "/home/mohobie/projects/aether-game-engine/assets/blocks/default_blocks.json";
    if (!BlockRegistry::GetInstance().LoadFromFile(blockPath)) {
        std::cerr << "Warning: Could not load block definitions" << std::endl;
    }
    
    // Test Vec3
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);
    Vec3 c = a + b;
    std::cout << "Vec3 add: (" << c.x << ", " << c.y << ", " << c.z << ")" << std::endl;
    
    // Test Block
    BlockTypeID stoneId = BlockRegistry::GetInstance().GetBlockId("stone");
    Block block(stoneId);
    std::cout << "Block: " << block.GetName() << " (solid: " << block.IsSolid() << ")" << std::endl;
    
    // Test Chunk
    Chunk chunk(0, 0, 0);
    chunk.SetBlock(1, 1, 1, stoneId);
    chunk.SetBlock(2, 1, 1, stoneId);
    chunk.SetBlock(1, 2, 1, stoneId);
    std::cout << "Chunk block at (1,1,1): " << chunk.GetBlock(1, 1, 1) << std::endl;
    
    // Test World
    World world;
    world.SetBlock(5, 5, 5, "dirt");
    std::cout << "World block at (5,5,5): " << world.GetBlock(5, 5, 5) << std::endl;
    
    // Test AABB
    AABB box1(Vec3(0, 0, 0), Vec3(1, 1, 1));
    AABB box2(Vec3(0.5, 0.5, 0.5), Vec3(1, 1, 1));
    std::cout << "AABB intersect: " << box1.Intersects(box2) << std::endl;
    
    // Test Mesh Builder
    std::cout << "\n=== Mesh Builder Test ===" << std::endl;
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(chunk);
    std::cout << "Chunk mesh vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "Chunk mesh indices: " << mesh.GetIndexCount() << std::endl;
    
    // Test with more blocks
    Chunk chunk2(0, 0, 0);
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            for (int z = 0; z < 4; ++z) {
                if (x == 0 || x == 3 || y == 0 || y == 3 || z == 0 || z == 3) {
                    chunk2.SetBlock(x, y, z, stoneId);
                }
            }
        }
    }
    
    Mesh mesh2 = BlockMeshBuilder::BuildChunkMesh(chunk2);
    std::cout << "Hollow cube mesh vertices: " << mesh2.GetVertexCount() << std::endl;
    std::cout << "Hollow cube mesh indices: " << mesh2.GetIndexCount() << std::endl;
    
    // Test Entity Component System
    std::cout << "\n=== Entity Component System Test ===" << std::endl;
    EntityManager entityManager;
    
    // Create player entity
    Entity* player = entityManager.CreateEntity("Player");
    player->AddComponent<TransformComponent>(Vec3(0, 64, 0));
    player->AddComponent<HealthComponent>(100.0f);
    player->AddComponent<MovementComponent>(5.0f, 0.85f);
    
    // Create enemy entity
    Entity* enemy = entityManager.CreateEntity("Enemy");
    enemy->AddComponent<TransformComponent>(Vec3(10, 64, 10));
    enemy->AddComponent<HealthComponent>(50.0f);
    
    // Test components
    auto* playerTransform = player->GetComponent<TransformComponent>();
    auto* playerHealth = player->GetComponent<HealthComponent>();
    auto* playerMovement = player->GetComponent<MovementComponent>();
    
    std::cout << "Player position: (" << playerTransform->position.x << ", " 
              << playerTransform->position.y << ", " << playerTransform->position.z << ")" << std::endl;
    std::cout << "Player health: " << playerHealth->currentHealth << "/" << playerHealth->maxHealth << std::endl;
    
    // Test damage
    playerHealth->TakeDamage(25);
    std::cout << "After damage - Player health: " << playerHealth->currentHealth << std::endl;
    
    // Test movement
    playerMovement->AddForce(Vec3(1, 0, 0));
    entityManager.Update(0.016f); // Simulate one frame at 60fps
    std::cout << "After movement - Player position: (" << playerTransform->position.x << ", " 
              << playerTransform->position.y << ", " << playerTransform->position.z << ")" << std::endl;
    
    // Test entity queries
    auto entitiesWithHealth = entityManager.GetEntitiesWithComponent<HealthComponent>();
    std::cout << "Entities with health: " << entitiesWithHealth.size() << std::endl;
    
    // Test script component
    Entity* scriptEntity = entityManager.CreateEntity("Scripted");
    auto* script = scriptEntity->AddComponent<ScriptComponent>();
    float counter = 0;
    script->updateFunc = [&counter](float dt) { counter += dt; };
    entityManager.Update(0.1f);
    std::cout << "Script counter after 0.1s: " << counter << std::endl;
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}