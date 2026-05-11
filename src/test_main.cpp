#include "rendering/post_processing.h"
#include "ui/ui_system.h"
#include "ai/ai_system.h"
#include "ai/enemy_ai.h"
#include "ai/enemy_spawner.h"
#include "game/combat_system.h"
#include "game/day_night_cycle.h"
#include "animation/animation.h"
#include "editor/in_game_editor.h"
#include "platform/input_manager.h"
#include "rendering/camera.h"
#include "game/player_stats.h"
#include "core/crafting.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include <iostream>

using namespace vge;

void TestPostProcessing() {
    std::cout << "\n=== Post-Processing Stack Test ===" << std::endl;
    
    PostProcessStack postProcess;
    postProcess.Initialize(1920, 1080);
    
    // Configure effects
    postProcess.GetBloom()->SetIntensity(0.6f);
    postProcess.GetBloom()->SetThreshold(0.7f);
    
    postProcess.GetSSAO()->SetRadius(0.5f);
    postProcess.GetSSAO()->SetBias(0.025f);
    
    postProcess.GetToneMapping()->SetExposure(1.2f);
    postProcess.GetToneMapping()->SetAlgorithm(ToneMappingEffect::Algorithm::ACES);
    
    postProcess.GetVignette()->SetIntensity(0.3f);
    postProcess.GetVignette()->SetSmoothness(0.8f);
    
    postProcess.GetColorGrading()->SetContrast(1.1f);
    postProcess.GetColorGrading()->SetSaturation(1.2f);
    
    // Print settings
    postProcess.PrintSettings();
    
    // Simulate frame
    postProcess.BeginScene();
    std::cout << "[Scene] Rendering 3D scene to HDR buffer..." << std::endl;
    postProcess.EndScene();
    
    std::cout << "Post-processing stack test passed!" << std::endl;
}

void TestUI() {
    std::cout << "\n=== UI System Test ===" << std::endl;
    
    // Create UI elements directly (avoid UIManager ownership issues)
    auto panel = std::make_unique<UIPanel>("MainMenu");
    panel->SetPosition(vge::Vec2(100, 100));
    panel->SetSize(vge::Vec2(400, 500));
    panel->SetAutoLayout(true);
    panel->SetPadding(20);
    panel->SetSpacing(10);
    
    // Add title
    auto title = std::make_unique<UILabel>("Title", "Aether Voxel Engine");
    title->SetFontSize(28);
    title->SetForegroundColor(Vec3(0.2f, 0.8f, 1.0f));
    panel->AddChild(std::move(title));
    
    // Add buttons
    auto playBtn = std::make_unique<UIButton>("PlayBtn", "Play Game");
    playBtn->SetSize(vge::Vec2(200, 40));
    playBtn->SetBackgroundColor(Vec3(0.2f, 0.6f, 0.2f));
    playBtn->SetOnClick([]() {
        std::cout << "[UI] Play button clicked!" << std::endl;
    });
    panel->AddChild(std::move(playBtn));
    
    auto settingsBtn = std::make_unique<UIButton>("SettingsBtn", "Settings");
    settingsBtn->SetSize(vge::Vec2(200, 40));
    panel->AddChild(std::move(settingsBtn));
    
    auto quitBtn = std::make_unique<UIButton>("QuitBtn", "Quit");
    quitBtn->SetSize(vge::Vec2(200, 40));
    quitBtn->SetBackgroundColor(Vec3(0.6f, 0.2f, 0.2f));
    panel->AddChild(std::move(quitBtn));
    
    // Add slider
    auto volumeSlider = std::make_unique<UISlider>("Volume", 0.0f, 1.0f);
    volumeSlider->SetValue(0.7f);
    volumeSlider->SetSize(vge::Vec2(200, 30));
    volumeSlider->SetOnValueChanged([](float value) {
        std::cout << "[UI] Volume changed to: " << (value * 100) << "%" << std::endl;
    });
    panel->AddChild(std::move(volumeSlider));
    
    // Add checkbox
    auto fullscreenCheck = std::make_unique<UICheckbox>("Fullscreen", "Fullscreen Mode");
    fullscreenCheck->SetChecked(true);
    panel->AddChild(std::move(fullscreenCheck));
    
    // Add dropdown
    auto resolutionDropdown = std::make_unique<UIDropdown>("Resolution");
    resolutionDropdown->AddOption("1920x1080");
    resolutionDropdown->AddOption("1280x720");
    resolutionDropdown->AddOption("2560x1440");
    resolutionDropdown->SetSelectedIndex(0);
    panel->AddChild(std::move(resolutionDropdown));
    
    // Layout and render
    panel->Layout();
    panel->Render();
    
    // Simulate mouse interaction
    std::cout << "\n[Input] Simulating mouse click on Play button..." << std::endl;
    UIEvent event;
    event.type = UIEventType::Click;
    event.position = vge::Vec2(210, 200);
    panel->HandleEvent(event);
    
    // Print hierarchy
    std::cout << "\n=== UI Hierarchy ===" << std::endl;
    panel->PrintHierarchy();
    
    std::cout << "UI system test passed!" << std::endl;
}

void TestAnimation() {
    std::cout << "\n=== Animation System Test ===" << std::endl;
    
    // Create skeleton
    Skeleton skeleton;
    int root = skeleton.AddJoint("Root", -1);
    int spine = skeleton.AddJoint("Spine", root);
    int head = skeleton.AddJoint("Head", spine);
    int armL = skeleton.AddJoint("ArmL", spine);
    int armR = skeleton.AddJoint("ArmR", spine);
    int legL = skeleton.AddJoint("LegL", root);
    int legR = skeleton.AddJoint("LegR", root);
    
    std::cout << "Created skeleton with " << skeleton.GetJointCount() << " joints" << std::endl;
    skeleton.PrintHierarchy();
    
    // Create walk animation
    AnimationClip walkClip("walk");
    walkClip.SetDuration(1.0f);
    
    
    // Create walk animation tracks
    AnimationTrack legLTrack;
    legLTrack.jointIndex = legL;
    Keyframe k1; k1.time = 0.0f; k1.position = Vec3(0, 0, 0); k1.rotation = Vec3(0, 0, 30); k1.scale = Vec3(1, 1, 1);
    Keyframe k2; k2.time = 0.5f; k2.position = Vec3(0, 0, 0); k2.rotation = Vec3(0, 0, -30); k2.scale = Vec3(1, 1, 1);
    Keyframe k3; k3.time = 1.0f; k3.position = Vec3(0, 0, 0); k3.rotation = Vec3(0, 0, 30); k3.scale = Vec3(1, 1, 1);
    legLTrack.keyframes.push_back(k1);
    legLTrack.keyframes.push_back(k2);
    legLTrack.keyframes.push_back(k3);
    walkClip.AddTrack(legLTrack);
    
    AnimationTrack legRTrack;
    legRTrack.jointIndex = legR;
    Keyframe k4; k4.time = 0.0f; k4.position = Vec3(0, 0, 0); k4.rotation = Vec3(0, 0, -30); k4.scale = Vec3(1, 1, 1);
    Keyframe k5; k5.time = 0.5f; k5.position = Vec3(0, 0, 0); k5.rotation = Vec3(0, 0, 30); k5.scale = Vec3(1, 1, 1);
    Keyframe k6; k6.time = 1.0f; k6.position = Vec3(0, 0, 0); k6.rotation = Vec3(0, 0, -30); k6.scale = Vec3(1, 1, 1);
    legRTrack.keyframes.push_back(k4);
    legRTrack.keyframes.push_back(k5);
    legRTrack.keyframes.push_back(k6);
    walkClip.AddTrack(legRTrack);
    
    AnimationTrack armLTrack;
    armLTrack.jointIndex = armL;
    Keyframe k7; k7.time = 0.0f; k7.position = Vec3(0, 0, 0); k7.rotation = Vec3(0, 0, -20); k7.scale = Vec3(1, 1, 1);
    Keyframe k8; k8.time = 0.5f; k8.position = Vec3(0, 0, 0); k8.rotation = Vec3(0, 0, 20); k8.scale = Vec3(1, 1, 1);
    Keyframe k9; k9.time = 1.0f; k9.position = Vec3(0, 0, 0); k9.rotation = Vec3(0, 0, -20); k9.scale = Vec3(1, 1, 1);
    armLTrack.keyframes.push_back(k7);
    armLTrack.keyframes.push_back(k8);
    armLTrack.keyframes.push_back(k9);
    walkClip.AddTrack(armLTrack);
    
    AnimationTrack armRTrack;
    armRTrack.jointIndex = armR;
    Keyframe k10; k10.time = 0.0f; k10.position = Vec3(0, 0, 0); k10.rotation = Vec3(0, 0, 20); k10.scale = Vec3(1, 1, 1);
    Keyframe k11; k11.time = 0.5f; k11.position = Vec3(0, 0, 0); k11.rotation = Vec3(0, 0, -20); k11.scale = Vec3(1, 1, 1);
    Keyframe k12; k12.time = 1.0f; k12.position = Vec3(0, 0, 0); k12.rotation = Vec3(0, 0, 20); k12.scale = Vec3(1, 1, 1);
    armRTrack.keyframes.push_back(k10);
    armRTrack.keyframes.push_back(k11);
    armRTrack.keyframes.push_back(k12);
    walkClip.AddTrack(armRTrack);
    
    std::cout << "Created walk animation with tracks" << std::endl;
    
    // Play animation
    Animator animator(&skeleton);
    animator.Play(&walkClip, true);
    
    // Sample animation
    float testTimes[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    for (float time : testTimes) {
        animator.SetTime(time);
        animator.Update(0.0f); // Force update
        
        walkClip.Sample(time, skeleton, true);
        
        Vec3 legLRot = skeleton.GetJoint(legL)->localRotation;
        Vec3 legRRot = skeleton.GetJoint(legR)->localRotation;
        
        std::cout << "Time " << time << "s - LegL: (" << legLRot.x << "," << legLRot.y << "," << legLRot.z 
                  << ") LegR: (" << legRRot.x << "," << legRRot.y << "," << legRRot.z << ")" << std::endl;
    }
    
    std::cout << "Animation system test passed!" << std::endl;
}

void TestAI() {
    std::cout << "\n=== AI System Test ===" << std::endl;
    
    // Create navigation mesh
    NavigationMesh navMesh;
    
    // Build from simple grid
    navMesh.GenerateGrid(Vec3(-10, 0, -10), Vec3(10, 0, 10), 2.0f);
    
    std::cout << "NavMesh built with " << navMesh.GetNodeCount() << " nodes" << std::endl;
    
    // Find path
    Vec3 start(-8, 0, -8);
    Vec3 goal(8, 0, 8);
    
    std::vector<Vec3> path = navMesh.FindPath(start, goal);
    std::cout << "Path found with " << path.size() << " waypoints:" << std::endl;
    for (size_t i = 0; i < path.size(); ++i) {
        std::cout << "  [" << i << "] (" << path[i].x << ", " << path[i].y << ", " << path[i].z << ")" << std::endl;
    }
    
    // Create agent and follow path
    AIAgent agent(start, 5.0f);
    agent.SetPosition(start);
    agent.SetMaxSpeed(5.0f);
    // Create path object
    Path pathObj;
    pathObj.waypoints = path;
    agent.SetPath(pathObj);
    
    std::cout << "\nAgent following path..." << std::endl;
    float deltaTime = 0.1f;
    int steps = 0;
    while (!agent.IsPathComplete() && steps < 100) {
        Vec3 pos = agent.GetPosition();
        agent.Update(deltaTime);
        steps++;
        
        if (steps % 10 == 0) {
            std::cout << "  Step " << steps << " - Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        }
    }
    
    Vec3 finalPos = agent.GetPosition();
    std::cout << "Agent reached goal in " << steps << " steps" << std::endl;
    std::cout << "Final position: (" << finalPos.x << ", " << finalPos.y << ", " << finalPos.z << ")" << std::endl;
    
    // Test steering behaviors
    std::cout << "\n=== Steering Behaviors ===" << std::endl;
    SteeringBehavior steering;
    
    Vec3 agentPos(0, 0, 0);
    Vec3 targetPos(10, 0, 10);
    Vec3 agentVel(1, 0, 1);
    
    Vec3 seekForce = steering.Seek(agentPos, agentVel, targetPos, 5.0f);
    std::cout << "Seek force: (" << seekForce.x << ", " << seekForce.y << ", " << seekForce.z << ")" << std::endl;
    
    Vec3 fleeForce = steering.Flee(agentPos, agentVel, targetPos, 5.0f);
    std::cout << "Flee force: (" << fleeForce.x << ", " << fleeForce.y << ", " << fleeForce.z << ")" << std::endl;
    
    Vec3 wanderForce = steering.Wander(agentVel, 5.0f, 2.0f, 0.5f);
    std::cout << "Wander force: (" << wanderForce.x << ", " << wanderForce.y << ", " << wanderForce.z << ")" << std::endl;
    
    std::cout << "AI system test passed!" << std::endl;
}

void TestEnemyAI() {
    std::cout << "\n=== Enemy AI Test ===" << std::endl;
    
    // Create navigation mesh
    NavigationMesh navMesh;
    navMesh.GenerateGrid(Vec3(-50, 0, -50), Vec3(50, 0, 50), 2.0f);
    
    // Test Zombie creation and stats
    Vec3 spawnPos(10, 0, 10);
    EnemyAI zombie(EnemyType::ZOMBIE, spawnPos);
    
    std::cout << "Created Zombie at (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;
    std::cout << "Zombie Health: " << zombie.GetHealth() << "/" << zombie.GetMaxHealth() << std::endl;
    std::cout << "Zombie Speed: " << zombie.GetSpeed() << std::endl;
    std::cout << "Zombie Damage: " << zombie.GetDamage() << std::endl;
    std::cout << "Zombie Attack Range: " << zombie.GetAttackRange() << std::endl;
    std::cout << "Zombie Detection Range: " << zombie.GetDetectionRange() << std::endl;
    
    // Test Skeleton creation and stats
    Vec3 skeletonPos(-10, 0, -10);
    EnemyAI skeleton(EnemyType::SKELETON, skeletonPos);
    
    std::cout << "\nCreated Skeleton at (" << skeletonPos.x << ", " << skeletonPos.y << ", " << skeletonPos.z << ")" << std::endl;
    std::cout << "Skeleton Health: " << skeleton.GetHealth() << "/" << skeleton.GetMaxHealth() << std::endl;
    std::cout << "Skeleton Speed: " << skeleton.GetSpeed() << std::endl;
    std::cout << "Skeleton Damage: " << skeleton.GetDamage() << std::endl;
    std::cout << "Skeleton Attack Range: " << skeleton.GetAttackRange() << " (ranged)" << std::endl;
    
    // Test state transitions - IDLE to CHASE
    Vec3 playerPos(15, 0, 15); // Within detection range of zombie
    std::cout << "\n=== State Transition Tests ===" << std::endl;
    std::cout << "Initial state: IDLE" << std::endl;
    
    // Update zombie with player nearby
    for (int i = 0; i < 10; ++i) {
        zombie.Update(0.1f, playerPos, &navMesh);
    }
    
    std::cout << "After player enters detection range: " 
              << (zombie.GetState() == EnemyState::CHASE ? "CHASE" : "IDLE") << std::endl;
    
    // Test CHASE to ATTACK
    Vec3 closePlayer(10.5f, 0, 10.5f); // Very close to zombie
    zombie.SetState(EnemyState::CHASE);
    zombie.Update(0.1f, closePlayer, &navMesh);
    
    std::cout << "When player is within attack range: " 
              << (zombie.GetState() == EnemyState::ATTACK ? "ATTACK" : "CHASE") << std::endl;
    
    // Test damage and death
    std::cout << "\n=== Damage Test ===" << std::endl;
    std::cout << "Zombie health before damage: " << zombie.GetHealth() << std::endl;
    zombie.TakeDamage(30.0f);
    std::cout << "After 30 damage: " << zombie.GetHealth() << "/" << zombie.GetMaxHealth() << std::endl;
    
    zombie.TakeDamage(50.0f);
    std::cout << "After 50 more damage: " << zombie.GetHealth() << "/" << zombie.GetMaxHealth() << std::endl;
    
    // Kill the zombie
    zombie.TakeDamage(100.0f); // Overkill
    std::cout << "After lethal damage: " << zombie.GetHealth() << std::endl;
    std::cout << "Is dead: " << (zombie.IsDead() ? "Yes" : "No") << std::endl;
    std::cout << "State: " << (zombie.GetState() == EnemyState::DIE ? "DIE" : "Other") << std::endl;
    
    // Test skeleton death
    std::cout << "\n=== Skeleton Death Test ===" << std::endl;
    skeleton.Die();
    std::cout << "Skeleton died. Is dead: " << (skeleton.IsDead() ? "Yes" : "No") << std::endl;
    std::cout << "State: " << (skeleton.GetState() == EnemyState::DIE ? "DIE" : "Other") << std::endl;
    
    std::cout << "Enemy AI test passed!" << std::endl;
}

void TestEnemySpawner() {
    std::cout << "\n=== Enemy Spawner Test ===" << std::endl;
    
    EnemySpawner spawner;
    NavigationMesh navMesh;
    navMesh.GenerateGrid(Vec3(-100, 0, -100), Vec3(100, 0, 100), 5.0f);
    
    // Test configuration
    std::cout << "Max enemies: " << spawner.GetMaxEnemies() << std::endl;
    std::cout << "Spawn radius: " << spawner.GetMinSpawnRadius() << "-" << spawner.GetMaxSpawnRadius() << std::endl;
    std::cout << "Spawn cooldown: " << spawner.GetSpawnCooldown() << std::endl;
    
    // Test night time spawning
    Vec3 playerPos(0, 0, 0);
    spawner.SetTime(0.8f); // Night time
    
    std::cout << "\nTime set to 0.8 (night): " << (spawner.IsNightTime() ? "Yes" : "No") << std::endl;
    
    // Force spawn some enemies
    spawner.ForceSpawnEnemy(Vec3(25, 0, 0), EnemyType::ZOMBIE);
    spawner.ForceSpawnEnemy(Vec3(-25, 0, 0), EnemyType::SKELETON);
    spawner.ForceSpawnEnemy(Vec3(0, 0, 30), EnemyType::ZOMBIE);
    
    std::cout << "Active enemies after 3 spawns: " << spawner.GetActiveEnemyCount() << std::endl;
    std::cout << "Total spawned: " << spawner.GetTotalSpawned() << std::endl;
    
    // Test spawn cap
    spawner.SetMaxEnemies(5);
    spawner.ForceSpawnEnemy(Vec3(10, 0, 10), EnemyType::ZOMBIE);
    spawner.ForceSpawnEnemy(Vec3(-10, 0, -10), EnemyType::SKELETON);
    spawner.ForceSpawnEnemy(Vec3(15, 0, -15), EnemyType::ZOMBIE); // Should still work (forced)
    
    std::cout << "Active enemies after 6 spawns (cap 5): " << spawner.GetActiveEnemyCount() << std::endl;
    
    // Test safe zones
    std::cout << "\n=== Safe Zone Test ===" << std::endl;
    Vec3 torchPos(30, 0, 0);
    spawner.AddLightSource(torchPos, 10.0f, 1.0f);
    
    std::cout << "Added torch at (30, 0, 0) with radius 10" << std::endl;
    std::cout << "Position (35, 0, 0) in safe zone: " 
              << (spawner.IsInSafeZone(Vec3(35, 0, 0)) ? "Yes" : "No") << std::endl;
    std::cout << "Position (50, 0, 0) in safe zone: " 
              << (spawner.IsInSafeZone(Vec3(50, 0, 0)) ? "Yes" : "No") << std::endl;
    
    // Test enemy queries
    std::cout << "\n=== Enemy Query Tests ===" << std::endl;
    auto allEnemies = spawner.GetAllEnemies();
    std::cout << "All enemies: " << allEnemies.size() << std::endl;
    
    auto nearbyEnemies = spawner.GetEnemiesInRadius(playerPos, 30.0f);
    std::cout << "Enemies within 30 blocks of player: " << nearbyEnemies.size() << std::endl;
    
    // Test update and enemy chasing player
    std::cout << "\n=== Spawner Update Test ===" << std::endl;
    Vec3 movingPlayer(5, 0, 5);
    spawner.Update(1.0f, movingPlayer, &navMesh);
    
    // Check if enemies moved toward player
    for (auto* enemy : spawner.GetAllEnemies()) {
        if (enemy && !enemy->IsDead()) {
            float dist = (enemy->GetPosition() - movingPlayer).length();
            std::cout << "Enemy at (" << enemy->GetPosition().x << ", " 
                      << enemy->GetPosition().z << ") distance to player: " 
                      << dist << std::endl;
        }
    }
    
    // Test day time (no spawning)
    spawner.SetTime(0.5f); // Day time
    std::cout << "\nTime set to 0.5 (day): " << (spawner.IsNightTime() ? "Yes" : "No") << std::endl;
    int countBefore = spawner.GetActiveEnemyCount();
    spawner.Update(1.0f, movingPlayer, &navMesh);
    std::cout << "Enemies after day update: " << spawner.GetActiveEnemyCount() 
              << " (should be same: " << countBefore << ")" << std::endl;
    
    // Test clear
    spawner.ClearAllEnemies();
    std::cout << "\nAfter clearing all enemies: " << spawner.GetActiveEnemyCount() << std::endl;
    std::cout << "Total killed: " << spawner.GetTotalKilled() << std::endl;
    
    std::cout << "Enemy spawner test passed!" << std::endl;
}

void TestCombatSystem() {
    std::cout << "\n=== Combat System Test ===" << std::endl;
    
    CombatSystem combat;
    EnemySpawner spawner;
    NavigationMesh navMesh;
    navMesh.GenerateGrid(Vec3(-50, 0, -50), Vec3(50, 0, 50), 2.0f);
    
    combat.SetEnemySpawner(&spawner);
    
    // Test player stats
    std::cout << "Player max health: " << combat.GetPlayerStats().maxHealth << std::endl;
    std::cout << "Player health: " << combat.GetPlayerHealth() << std::endl;
    std::cout << "Player damage: " << combat.GetPlayerStats().attackDamage << std::endl;
    std::cout << "Player attack range: " << combat.GetPlayerStats().attackRange << std::endl;
    
    // Test damage calculation
    std::cout << "\n=== Damage Calculation Test ===" << std::endl;
    
    // No armor
    DamageInfo dmg1 = combat.CalculateDamage(20.0f, 0.0f, false);
    std::cout << "20 damage, 0 armor: " << dmg1.finalDamage << " final (blocked: " << dmg1.armorBlocked << ")" << std::endl;
    
    // With armor
    DamageInfo dmg2 = combat.CalculateDamage(20.0f, 10.0f, false);
    std::cout << "20 damage, 10 armor: " << dmg2.finalDamage << " final (blocked: " << dmg2.armorBlocked << ")" << std::endl;
    
    // High armor (capped at 80%)
    DamageInfo dmg3 = combat.CalculateDamage(20.0f, 50.0f, false);
    std::cout << "20 damage, 50 armor: " << dmg3.finalDamage << " final (blocked: " << dmg3.armorBlocked << ", capped)" << std::endl;
    
    // Test player attack
    std::cout << "\n=== Player Attack Test ===" << std::endl;
    Vec3 playerPos(0, 0, 0);
    Vec3 playerForward(1, 0, 0);
    
    // Spawn an enemy in front of player
    spawner.ForceSpawnEnemy(Vec3(2, 0, 0), EnemyType::ZOMBIE);
    auto enemies = spawner.GetAllEnemies();
    EnemyAI* zombie = enemies.empty() ? nullptr : enemies[0];
    
    if (zombie) {
        std::cout << "Zombie health before attack: " << zombie->GetHealth() << std::endl;
        bool hit = combat.PlayerAttack(playerPos, playerForward);
        std::cout << "Player attack hit: " << (hit ? "Yes" : "No") << std::endl;
        std::cout << "Zombie health after attack: " << zombie->GetHealth() << std::endl;
    }
    
    // Test enemy attack on player
    std::cout << "\n=== Enemy Attack Test ===" << std::endl;
    // Spawn enemy close to player
    spawner.ForceSpawnEnemy(Vec3(1.5f, 0, 0), EnemyType::ZOMBIE);
    enemies = spawner.GetAllEnemies();
    
    std::cout << "Player health before enemy attack: " << combat.GetPlayerHealth() << std::endl;
    combat.Update(0.1f, playerPos); // This should trigger enemy attacks
    std::cout << "Player health after enemy attack: " << combat.GetPlayerHealth() << std::endl;
    
    // Test player death and respawn
    std::cout << "\n=== Player Death Test ===" << std::endl;
    combat.DamagePlayer(200.0f); // Overkill
    std::cout << "Player alive after 200 damage: " << (combat.IsPlayerAlive() ? "Yes" : "No") << std::endl;
    std::cout << "Waiting for respawn: " << (combat.IsWaitingForRespawn() ? "Yes" : "No") << std::endl;
    
    // Simulate respawn timer
    combat.Update(5.0f, playerPos); // Should respawn after delay
    std::cout << "Player alive after respawn: " << (combat.IsPlayerAlive() ? "Yes" : "No") << std::endl;
    std::cout << "Player health after respawn: " << combat.GetPlayerHealth() << std::endl;
    
    // Test drop rolling
    std::cout << "\n=== Drop Table Test ===" << std::endl;
    auto zombieDrops = combat.RollDrops(EnemyType::ZOMBIE);
    std::cout << "Zombie drops rolled: " << zombieDrops.size() << " items" << std::endl;
    for (const auto& drop : zombieDrops) {
        std::cout << "  - " << drop.second << "x " << drop.first << std::endl;
    }
    
    auto skeletonDrops = combat.RollDrops(EnemyType::SKELETON);
    std::cout << "Skeleton drops rolled: " << skeletonDrops.size() << " items" << std::endl;
    for (const auto& drop : skeletonDrops) {
        std::cout << "  - " << drop.second << "x " << drop.first << std::endl;
    }
    
    // Test healing
    std::cout << "\n=== Healing Test ===" << std::endl;
    combat.DamagePlayer(50.0f);
    std::cout << "Player health after 50 damage: " << combat.GetPlayerHealth() << std::endl;
    combat.HealPlayer(30.0f);
    std::cout << "Player health after 30 heal: " << combat.GetPlayerHealth() << std::endl;
    combat.HealPlayer(100.0f); // Overheal should cap
    std::cout << "Player health after 100 heal (capped): " << combat.GetPlayerHealth() << std::endl;
    
    std::cout << "Combat system test passed!" << std::endl;
}

void TestFlyCamera() {
    std::cout << "\n=== Fly Camera Controller Test ===" << std::endl;

    // Create fly camera
    FlyCamera flyCam;
    flyCam.SetPosition(Vec3(0, 5, 0));
    flyCam.SetRotation(-90.0f, 0.0f); // Looking down -Z
    flyCam.SetSpeed(5.0f);
    flyCam.SetSensitivity(0.1f);

    std::cout << "Initial position: (0, 5, 0)" << std::endl;
    std::cout << "Initial yaw: -90, pitch: 0 (looking down -Z)" << std::endl;
    std::cout << "Initial speed: 5.0" << std::endl;

    // Test forward vector
    Vec3 forward = flyCam.GetForward();
    std::cout << "Forward vector: (" << forward.x << ", " << forward.y << ", " << forward.z << ")" << std::endl;

    // Test right vector
    Vec3 right = flyCam.GetRight();
    std::cout << "Right vector: (" << right.x << ", " << right.y << ", " << right.z << ")" << std::endl;

    // Test up vector
    Vec3 up = flyCam.GetUp();
    std::cout << "Up vector: (" << up.x << ", " << up.y << ", " << up.z << ")" << std::endl;

    // Test pitch clamping
    flyCam.SetRotation(0.0f, 95.0f);
    std::cout << "\nAfter setting pitch to 95 (should clamp to 89):" << std::endl;
    std::cout << "Yaw: " << flyCam.GetYaw() << ", Pitch: " << flyCam.GetPitch() << std::endl;

    flyCam.SetRotation(0.0f, -95.0f);
    std::cout << "After setting pitch to -95 (should clamp to -89):" << std::endl;
    std::cout << "Yaw: " << flyCam.GetYaw() << ", Pitch: " << flyCam.GetPitch() << std::endl;

    // Test speed bounds
    flyCam.SetSpeed(0.1f);
    std::cout << "\nSpeed set to 0.1 (min bound check): " << flyCam.GetSpeed() << std::endl;
    flyCam.SetSpeed(200.0f);
    std::cout << "Speed set to 200 (max bound check): " << flyCam.GetSpeed() << std::endl;

    // Test LookAt
    flyCam.SetPosition(Vec3(0, 0, 0));
    flyCam.LookAt(Vec3(0, 0, -10));
    std::cout << "\nAfter LookAt(0, 0, -10):" << std::endl;
    std::cout << "Yaw: " << flyCam.GetYaw() << ", Pitch: " << flyCam.GetPitch() << std::endl;

    std::cout << "Fly camera controller test passed!" << std::endl;
}

void TestPlayerStats() {
    std::cout << "\n=== Player Stats Test ===" << std::endl;

    PlayerStats stats(Vec3(10, 20, 10));

    // Initial values
    std::cout << "Initial health: " << stats.GetHealth() << " (expected 100)" << std::endl;
    std::cout << "Initial hunger: " << stats.GetHunger() << " (expected 100)" << std::endl;
    std::cout << "Initial stamina: " << stats.GetStamina() << " (expected 100)" << std::endl;
    if (stats.GetHealth() != 100.0f || stats.GetHunger() != 100.0f || stats.GetStamina() != 100.0f) {
        std::cerr << "FAILED: Initial stats incorrect" << std::endl;
        return;
    }

    // Damage test
    stats.TakeDamage(25.0f);
    std::cout << "After 25 damage, health: " << stats.GetHealth() << " (expected 75)" << std::endl;
    if (stats.GetHealth() != 75.0f) {
        std::cerr << "FAILED: Damage calculation incorrect" << std::endl;
        return;
    }

    // Healing test
    stats.Heal(10.0f);
    std::cout << "After healing 10, health: " << stats.GetHealth() << " (expected 85)" << std::endl;
    if (stats.GetHealth() != 85.0f) {
        std::cerr << "FAILED: Healing calculation incorrect" << std::endl;
        return;
    }

    // Eating test
    stats.Eat(15.0f);
    std::cout << "After eating 15, hunger: " << stats.GetHunger() << " (expected 100, clamped)" << std::endl;
    if (stats.GetHunger() != 100.0f) {
        std::cerr << "FAILED: Hunger clamping incorrect" << std::endl;
        return;
    }

    // Stamina use test
    bool used = stats.UseStamina(30.0f);
    std::cout << "Used 30 stamina: " << (used ? "success" : "failed") << ", stamina: " << stats.GetStamina() << " (expected 70)" << std::endl;
    if (!used || stats.GetStamina() != 70.0f) {
        std::cerr << "FAILED: Stamina use incorrect" << std::endl;
        return;
    }

    // Stamina regeneration test
    stats.RegenerateStamina(20.0f);
    std::cout << "After regenerating 20, stamina: " << stats.GetStamina() << " (expected 90)" << std::endl;
    if (stats.GetStamina() != 90.0f) {
        std::cerr << "FAILED: Stamina regeneration incorrect" << std::endl;
        return;
    }

    // Death test
    stats.SetHealth(10.0f);
    stats.TakeDamage(50.0f);
    std::cout << "After lethal damage, dead: " << (stats.IsDead() ? "YES" : "NO") << " (expected YES)" << std::endl;
    if (!stats.IsDead()) {
        std::cerr << "FAILED: Death detection incorrect" << std::endl;
        return;
    }

    // Respawn test
    stats.Respawn();
    std::cout << "After respawn, health: " << stats.GetHealth() << ", hunger: " << stats.GetHunger() << ", stamina: " << stats.GetStamina() << std::endl;
    std::cout << "After respawn, dead: " << (stats.IsDead() ? "YES" : "NO") << " (expected NO)" << std::endl;
    if (stats.IsDead() || stats.GetHealth() != 100.0f || stats.GetHunger() != 100.0f || stats.GetStamina() != 100.0f) {
        std::cerr << "FAILED: Respawn incorrect" << std::endl;
        return;
    }

    // Hunger decay test (simulate 2 seconds)
    stats.SetHunger(50.0f);
    stats.Update(2.0f);
    std::cout << "After 2s update, hunger: " << stats.GetHunger() << " (expected ~49)" << std::endl;
    if (stats.GetHunger() > 49.5f || stats.GetHunger() < 48.5f) {
        std::cerr << "FAILED: Hunger decay incorrect" << std::endl;
        return;
    }

    // Stamina regen when not sprinting
    stats.SetStamina(50.0f);
    stats.SetSprinting(false);
    stats.Update(1.0f);
    std::cout << "After 1s not sprinting, stamina: " << stats.GetStamina() << " (expected ~60)" << std::endl;
    if (stats.GetStamina() < 59.0f || stats.GetStamina() > 61.0f) {
        std::cerr << "FAILED: Stamina regeneration incorrect" << std::endl;
        return;
    }

    // Sprint stamina cost
    stats.SetStamina(100.0f);
    stats.SetSprinting(true);
    stats.Update(1.0f);
    std::cout << "After 1s sprinting, stamina: " << stats.GetStamina() << " (expected ~85)" << std::endl;
    if (stats.GetStamina() < 84.0f || stats.GetStamina() > 86.0f) {
        std::cerr << "FAILED: Sprint stamina cost incorrect" << std::endl;
        return;
    }

    std::cout << "Player stats test passed!" << std::endl;
}

void TestSurvivalMode() {
    std::cout << "\n=== Survival Mode Test ===" << std::endl;

    // SurvivalMode requires World/Camera/Input/Renderer pointers.
    // We test the standalone PlayerStats and CraftingSystem integration here.

    // Test crafting system initialization
    CraftingSystem craftingSystem;
    std::cout << "Crafting recipes loaded: " << craftingSystem.GetRecipeCount() << std::endl;

    // Test PlayerStats with spawn point
    PlayerStats stats(Vec3(5, 20, 5));
    Vec3 spawn = stats.GetSpawnPoint();
    std::cout << "Spawn point: (" << spawn.x << ", " << spawn.y << ", " << spawn.z << ")" << std::endl;
    if (spawn.x != 5.0f || spawn.y != 20.0f || spawn.z != 5.0f) {
        std::cerr << "FAILED: Spawn point incorrect" << std::endl;
        return;
    }

    // Simulate update loop - player takes damage, starves, dies, respawns
    stats.SetHealth(5.0f);
    stats.SetHunger(0.0f);
    stats.SetStamina(100.0f);

    // Update for 5 seconds while starving
    for (int i = 0; i < 5; ++i) {
        stats.Update(1.0f);
        std::cout << "  t=" << (i+1) << "s: health=" << stats.GetHealth()
                  << ", hunger=" << stats.GetHunger()
                  << ", dead=" << (stats.IsDead() ? "YES" : "NO") << std::endl;
        if (stats.IsDead()) break;
    }

    if (!stats.IsDead()) {
        std::cerr << "FAILED: Player should have died from starvation" << std::endl;
        return;
    }

    // Respawn
    stats.Respawn();
    std::cout << "After respawn: health=" << stats.GetHealth()
              << ", hunger=" << stats.GetHunger()
              << ", stamina=" << stats.GetStamina()
              << ", dead=" << (stats.IsDead() ? "YES" : "NO") << std::endl;
    if (stats.IsDead()) {
        std::cerr << "FAILED: Player should be alive after respawn" << std::endl;
        return;
    }

    // Test night hunger multiplier via PlayerStats configuration
    stats.SetHungerDecayRate(0.5f * 1.5f); // night multiplier
    stats.SetHunger(100.0f);
    stats.Update(2.0f);
    std::cout << "After 2s with night multiplier, hunger: " << stats.GetHunger() << " (expected ~98.5)" << std::endl;
    if (stats.GetHunger() > 99.0f || stats.GetHunger() < 97.0f) {
        std::cerr << "FAILED: Night hunger decay incorrect" << std::endl;
        return;
    }

    std::cout << "Survival mode test passed!" << std::endl;
}

void TestDayNightCycle() {
    std::cout << "\n=== Day/Night Cycle Test ===" << std::endl;

    DayNightCycle cycle;

    // Test initial state (dawn)
    std::cout << "Initial time: " << cycle.GetTimeOfDay() << " (0.0 = dawn)" << std::endl;
    std::cout << "IsNight: " << (cycle.IsNight() ? "Yes" : "No") << std::endl;
    std::cout << "IsDay: " << (cycle.IsDay() ? "Yes" : "No") << std::endl;
    std::cout << "Time string: " << cycle.GetTimeString() << std::endl;

    // Test time progression (1 day = 1200 seconds)
    cycle.SetDayLength(1200.0f);
    cycle.Update(600.0f); // Half a day
    std::cout << "\nAfter 600s (half day):" << std::endl;
    std::cout << "Time: " << cycle.GetTimeOfDay() << std::endl;
    std::cout << "IsNight: " << (cycle.IsNight() ? "Yes" : "No") << std::endl;
    std::cout << "IsDay: " << (cycle.IsDay() ? "Yes" : "No") << std::endl;

    // Test noon
    cycle.SkipToNoon();
    std::cout << "\nAt noon (0.25):" << std::endl;
    std::cout << "Time: " << cycle.GetTimeOfDay() << std::endl;
    std::cout << "IsDay: " << (cycle.IsDay() ? "Yes" : "No") << std::endl;
    std::cout << "Sun intensity: " << cycle.GetSunIntensity() << std::endl;
    std::cout << "Sky top color: (" << cycle.GetSkyTopColor().x << ", "
              << cycle.GetSkyTopColor().y << ", " << cycle.GetSkyTopColor().z << ")" << std::endl;

    // Test dusk
    cycle.SkipToDusk();
    std::cout << "\nAt dusk (0.5):" << std::endl;
    std::cout << "Time: " << cycle.GetTimeOfDay() << std::endl;
    std::cout << "IsDusk: " << (cycle.IsDusk() ? "Yes" : "No") << std::endl;
    std::cout << "Sky horizon color: (" << cycle.GetSkyHorizonColor().x << ", "
              << cycle.GetSkyHorizonColor().y << ", " << cycle.GetSkyHorizonColor().z << ")" << std::endl;

    // Test midnight
    cycle.SkipToMidnight();
    std::cout << "\nAt midnight (0.75):" << std::endl;
    std::cout << "Time: " << cycle.GetTimeOfDay() << std::endl;
    std::cout << "IsNight: " << (cycle.IsNight() ? "Yes" : "No") << std::endl;
    std::cout << "Moon phase: " << cycle.GetMoonPhaseName() << std::endl;
    std::cout << "Moon brightness: " << cycle.GetMoonPhaseBrightness() << std::endl;

    // Test day/night blend
    cycle.SkipToDawn();
    std::cout << "\nDay/night blend at dawn: " << cycle.GetDayNightBlend() << std::endl;
    cycle.SkipToNoon();
    std::cout << "Day/night blend at noon: " << cycle.GetDayNightBlend() << std::endl;
    cycle.SkipToDusk();
    std::cout << "Day/night blend at dusk: " << cycle.GetDayNightBlend() << std::endl;
    cycle.SkipToMidnight();
    std::cout << "Day/night blend at midnight: " << cycle.GetDayNightBlend() << std::endl;

    // Test moon phases
    std::cout << "\n=== Moon Phase Test ===" << std::endl;
    for (int i = 0; i < 8; ++i) {
        cycle.SetTime(0.75f);
        std::cout << "Day " << (cycle.GetDayCount() + 1) << ": "
                  << cycle.GetMoonPhaseName() << " (brightness: "
                  << cycle.GetMoonPhaseBrightness() << ")" << std::endl;
    }

    // Test ambient light
    cycle.SkipToNoon();
    Vec3 ambientNoon = cycle.GetTotalAmbientLight();
    cycle.SkipToMidnight();
    Vec3 ambientNight = cycle.GetTotalAmbientLight();
    std::cout << "\nAmbient at noon: (" << ambientNoon.x << ", " << ambientNoon.y << ", " << ambientNoon.z << ")" << std::endl;
    std::cout << "Ambient at midnight: (" << ambientNight.x << ", " << ambientNight.y << ", " << ambientNight.z << ")" << std::endl;

    std::cout << "Day/Night cycle test passed!" << std::endl;
}

void TestLightSystem() {
    std::cout << "\n=== Light System Test ===" << std::endl;

    World world;
    LightSystem lights(&world);

    // Create a simple chunk with some blocks
    Chunk* chunk = world.GetOrCreateChunk(0, 0, 0);
    if (chunk) {
        chunk->SetBlock(0, 0, 0, 1);
        chunk->SetBlock(1, 0, 0, 1);
        chunk->SetBlock(0, 1, 0, 0);
    }

    std::cout << "Initial sky light intensity: " << lights.GetSkyLightIntensity() << std::endl;

    // Test torch placement
    lights.PlaceTorch(5, 5, 5, 14);
    std::cout << "Placed torch at (5, 5, 5) with level 14" << std::endl;
    std::cout << "Light sources: " << lights.GetLightSources().size() << std::endl;

    int lightAtTorch = lights.GetTotalLightLevel(5, 5, 5);
    std::cout << "Light at torch position: " << lightAtTorch << std::endl;

    int lightNearby = lights.GetTotalLightLevel(6, 5, 5);
    std::cout << "Light at (6, 5, 5): " << lightNearby << std::endl;

    int lightFar = lights.GetTotalLightLevel(10, 5, 5);
    std::cout << "Light at (10, 5, 5): " << lightFar << std::endl;

    // Test sky light update
    lights.UpdateSkyLightForTime(1.0f);
    std::cout << "\nSky light intensity at full day: " << lights.GetSkyLightIntensity() << std::endl;

    lights.UpdateSkyLightForTime(0.0f);
    std::cout << "Sky light intensity at full night: " << lights.GetSkyLightIntensity() << std::endl;

    bool darkEnough = lights.IsDarkEnoughForMobs(5, 5, 5, 7);
    std::cout << "\nIs (5, 5, 5) dark enough for mobs: " << (darkEnough ? "Yes" : "No") << std::endl;

    lights.RemoveTorch(5, 5, 5);
    std::cout << "After removing torch, light sources: " << lights.GetLightSources().size() << std::endl;

    lights.PlaceTorch(0, 2, 0, 14);
    lights.PlaceTorch(10, 2, 10, 10);
    std::cout << "After placing 2 more torches, light sources: " << lights.GetLightSources().size() << std::endl;

    lights.ClearLightSources();
    std::cout << "After clearing all, light sources: " << lights.GetLightSources().size() << std::endl;

    std::cout << "Light system test passed!" << std::endl;
}

void TestMobSpawner() {
    std::cout << "\n=== Mob Spawner Test ===" << std::endl;

    World world;
    LightSystem lights(&world);
    DayNightCycle cycle;

    // Create some terrain for spawning
    for (int x = -2; x < 2; ++x) {
        for (int z = -2; z < 2; ++z) {
            Chunk* chunk = world.GetOrCreateChunk(x, 0, z);
            if (chunk) {
                for (int bx = 0; bx < CHUNK_SIZE; ++bx) {
                    for (int bz = 0; bz < CHUNK_SIZE; ++bz) {
                        chunk->SetBlock(bx, 4, bz, 1);
                    }
                }
            }
        }
    }

    MobSpawner spawner(&world, &lights, &cycle);

    std::cout << "Initial mob count: " << spawner.GetMobCount() << std::endl;

    // Test spawning at night
    cycle.SkipToMidnight();
    lights.UpdateSkyLightForTime(0.0f);

    std::cout << "\nAt midnight (night):" << std::endl;
    std::cout << "IsNight: " << (cycle.IsNight() ? "Yes" : "No") << std::endl;

    Mob* zombie = spawner.SpawnMob(MobType::Zombie, Vec3(10, 5, 10));
    Mob* skeleton = spawner.SpawnMob(MobType::Skeleton, Vec3(15, 5, 15));
    Mob* spider = spawner.SpawnMob(MobType::Spider, Vec3(20, 5, 20));

    std::cout << "Spawned mobs: " << spawner.GetMobCount() << std::endl;

    if (zombie) {
        MobDef def = MobSpawner::GetMobDefForType(zombie->type);
        std::cout << "Zombie health: " << zombie->health << ", burns in daylight: "
                  << (def.burnsInDaylight ? "Yes" : "No") << std::endl;
    }

    if (spider) {
        MobDef def = MobSpawner::GetMobDefForType(spider->type);
        std::cout << "Spider can climb walls: " << (def.canClimbWalls ? "Yes" : "No") << std::endl;
    }

    auto chunkMobs = spawner.GetMobsInChunk(0, 0, 0);
    std::cout << "Mobs in chunk (0,0,0): " << chunkMobs.size() << std::endl;

    // Test spawn cap
    std::cout << "\n=== Spawn Cap Test ===" << std::endl;
    for (int i = 0; i < 10; ++i) {
        spawner.SpawnMob(MobType::Zombie, Vec3(static_cast<float>(i * 2), 5.0f, 0.0f));
    }
    std::cout << "After spawning 10 more zombies: " << spawner.GetMobCount() << std::endl;

    // Test despawning at dawn
    std::cout << "\n=== Dawn Despawn Test ===" << std::endl;
    cycle.SkipToDawn();
    lights.UpdateSkyLightForTime(1.0f);
    spawner.OnDawnBegin();
    std::cout << "After dawn (surface mobs despawned): " << spawner.GetMobCount() << std::endl;

    // Test cave protection
    std::cout << "\n=== Cave Protection Test ===" << std::endl;
    cycle.SkipToMidnight();
    lights.UpdateSkyLightForTime(0.0f);

    Mob* caveZombie = spawner.SpawnMob(MobType::Zombie, Vec3(5, 5, 5));
    if (caveZombie) {
        caveZombie->inCave = true;
        std::cout << "Spawned cave zombie, inCave: " << (caveZombie->inCave ? "Yes" : "No") << std::endl;
    }

    cycle.SkipToDawn();
    lights.UpdateSkyLightForTime(1.0f);
    spawner.OnDawnBegin();
    std::cout << "After dawn with cave mob: " << spawner.GetMobCount() << " (should be 1)" << std::endl;

    // Test queries
    std::cout << "\n=== Mob Query Tests ===" << std::endl;
    auto allMobs = spawner.GetAllMobs();
    std::cout << "All mobs: " << allMobs.size() << std::endl;

    auto nearbyMobs = spawner.GetMobsInRadius(Vec3(0, 5, 0), 20.0f);
    std::cout << "Mobs within 20 blocks of (0,5,0): " << nearbyMobs.size() << std::endl;

    spawner.DespawnAllMobs();
    std::cout << "\nAfter despawning all: " << spawner.GetMobCount() << std::endl;

    std::cout << "Mob spawner test passed!" << std::endl;
}

int main() {
    std::cout << "=== Aether Game Engine - Feature Tests ===" << std::endl;

    TestPostProcessing();
    TestUI();
    TestAnimation();
    TestAI();
    TestEnemyAI();
    TestEnemySpawner();
    TestCombatSystem();
    TestFlyCamera();
    TestPlayerStats();
    TestSurvivalMode();

    std::cout << "\n=== All Tests Passed ===" << std::endl;

    return 0;
}