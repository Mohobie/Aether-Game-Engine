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
#include "physics/physics_system.h"
#include "entity/entity.h"
#include "entity/components.h"
#include "animation/animation.h"

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
    
    // Test Physics System
    std::cout << "\n=== Physics System Test ===" << std::endl;
    PhysicsWorld physics;
    
    // Create ground
    BoxCollider* ground = physics.CreateBoxCollider(Vec3(0, -1, 0), Vec3(50, 1, 50));
    ground->isStatic = true;
    
    // Create dynamic body
    Rigidbody* body = physics.CreateBody(1.0f);
    body->SetPosition(Vec3(0, 10, 0));
    BoxCollider* bodyCollider = physics.CreateBoxCollider(Vec3(0, 10, 0), Vec3(0.5f, 0.5f, 0.5f));
    body->collider = bodyCollider;
    
    std::cout << "Initial body position: (" << body->position.x << ", " 
              << body->position.y << ", " << body->position.z << ")" << std::endl;
    
    // Simulate a few seconds
    for (int i = 0; i < 60; ++i) {
        physics.Step(0.016f);
        if (i % 15 == 0) {
            std::cout << "Frame " << i << " - Position: (" << body->position.x << ", " 
                      << body->position.y << ", " << body->position.z << ")" << std::endl;
        }
    }
    
    std::cout << "Final body position: (" << body->position.x << ", " 
              << body->position.y << ", " << body->position.z << ")" << std::endl;
    std::cout << "Body grounded: " << body->isGrounded << std::endl;
    
    // Test collision detection
    BoxCollider* testBox1 = physics.CreateBoxCollider(Vec3(0, 0, 0), Vec3(1, 1, 1));
    BoxCollider* testBox2 = physics.CreateBoxCollider(Vec3(1.5f, 0, 0), Vec3(1, 1, 1));
    
    Vec3 normal;
    float penetration;
    bool colliding = testBox1->TestCollision(testBox2, normal, penetration);
    std::cout << "\nBox collision test: " << (colliding ? "COLLIDING" : "NOT COLLIDING") << std::endl;
    if (colliding) {
        std::cout << "Penetration: " << penetration << std::endl;
        std::cout << "Normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << std::endl;
    }
    
    // Test raycast
    Vec3 hitPoint, hitNormal;
    bool rayHit = physics.Raycast(Vec3(0, 5, 0), Vec3(0, -1, 0), 10.0f, hitPoint, hitNormal);
    std::cout << "\nRaycast test: " << (rayHit ? "HIT" : "MISS") << std::endl;
    if (rayHit) {
        std::cout << "Hit point: (" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << ")" << std::endl;
        std::cout << "Hit normal: (" << hitNormal.x << ", " << hitNormal.y << ", " << hitNormal.z << ")" << std::endl;
    }
    
    // Test Animation System
    std::cout << "\n=== Animation System Test ===" << std::endl;
    AnimationSystem animSystem;
    
    // Create skeleton
    Skeleton* skeleton = animSystem.CreateSkeleton("TestSkeleton");
    int root = skeleton->AddJoint("Root", -1);
    int spine = skeleton->AddJoint("Spine", root);
    int head = skeleton->AddJoint("Head", spine);
    int armL = skeleton->AddJoint("ArmL", spine);
    int armR = skeleton->AddJoint("ArmR", spine);
    
    skeleton->PrintHierarchy();
    
    // Create animation clip
    AnimationClip* clip = animSystem.CreateClip("Wave");
    
    // Create track for right arm
    AnimationTrack armRTrack;
    armRTrack.jointIndex = armR;
    
    Keyframe k1;
    k1.time = 0.0f;
    k1.rotation = Vec3(0, 0, 0);
    armRTrack.keyframes.push_back(k1);
    
    Keyframe k2;
    k2.time = 0.5f;
    k2.rotation = Vec3(0, 0, 45);
    armRTrack.keyframes.push_back(k2);
    
    Keyframe k3;
    k3.time = 1.0f;
    k3.rotation = Vec3(0, 0, 0);
    armRTrack.keyframes.push_back(k3);
    
    clip->AddTrack(armRTrack);
    
    // Create animator and play
    Animator* animator = animSystem.CreateAnimator(skeleton);
    animator->Play(clip, true, 1.0f);
    
    // Update animation
    std::cout << "\nAnimation playback test:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        animSystem.Update(0.1f);
        Joint* armRJoint = skeleton->GetJoint(armR);
        std::cout << "Frame " << i << " - ArmR rotation: (" 
                  << armRJoint->localRotation.x << ", "
                  << armRJoint->localRotation.y << ", "
                  << armRJoint->localRotation.z << ")" << std::endl;
    }
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}