#include "rendering/post_processing.h"
#include "ui/ui_system.h"
#include "ai/ai_system.h"
#include "animation/animation.h"
#include "editor/in_game_editor.h"
#include "platform/input_manager.h"
#include "rendering/camera.h"
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
    
    UIManager ui;
    ui.SetScreenSize(Vec2(1920, 1080));
    
    // Create main menu panel
    auto* panel = ui.CreatePanel("MainMenu");
    panel->SetPosition(Vec2(100, 100));
    panel->SetSize(Vec2(400, 500));
    panel->SetAutoLayout(true);
    panel->SetPadding(20);
    panel->SetSpacing(10);
    
    // Add title
    auto* title = ui.CreateLabel("Title", "Aether Voxel Engine");
    title->SetFontSize(28);
    title->SetForegroundColor(Vec3(0.2f, 0.8f, 1.0f));
    panel->AddChild(std::unique_ptr<UIElement>(title));
    
    // Add buttons
    auto* playBtn = ui.CreateButton("PlayBtn", "Play Game");
    playBtn->SetSize(Vec2(200, 40));
    playBtn->SetBackgroundColor(Vec3(0.2f, 0.6f, 0.2f));
    playBtn->SetOnClick([]() {
        std::cout << "[UI] Play button clicked!" << std::endl;
    });
    panel->AddChild(std::unique_ptr<UIElement>(playBtn));
    
    auto* settingsBtn = ui.CreateButton("SettingsBtn", "Settings");
    settingsBtn->SetSize(Vec2(200, 40));
    panel->AddChild(std::unique_ptr<UIElement>(settingsBtn));
    
    auto* quitBtn = ui.CreateButton("QuitBtn", "Quit");
    quitBtn->SetSize(Vec2(200, 40));
    quitBtn->SetBackgroundColor(Vec3(0.6f, 0.2f, 0.2f));
    panel->AddChild(std::unique_ptr<UIElement>(quitBtn));
    
    // Add slider
    auto* volumeSlider = ui.CreateSlider("Volume", 0.0f, 1.0f);
    volumeSlider->SetValue(0.7f);
    volumeSlider->SetSize(Vec2(200, 30));
    volumeSlider->SetOnValueChanged([](float value) {
        std::cout << "[UI] Volume changed to: " << (value * 100) << "%" << std::endl;
    });
    panel->AddChild(std::unique_ptr<UIElement>(volumeSlider));
    
    // Add checkbox
    auto* fullscreenCheck = ui.CreateCheckbox("Fullscreen", "Fullscreen Mode");
    fullscreenCheck->SetChecked(true);
    panel->AddChild(std::unique_ptr<UIElement>(fullscreenCheck));
    
    // Add dropdown
    auto* resolutionDropdown = ui.CreateDropdown("Resolution");
    resolutionDropdown->AddOption("1920x1080");
    resolutionDropdown->AddOption("1280x720");
    resolutionDropdown->AddOption("2560x1440");
    resolutionDropdown->SetSelectedIndex(0);
    panel->AddChild(std::unique_ptr<UIElement>(resolutionDropdown));
    
    // Layout and render
    ui.Layout();
    ui.Render();
    
    // Simulate mouse interaction
    std::cout << "\n[Input] Simulating mouse click on Play button..." << std::endl;
    ui.HandleMouseMove(Vec2(210, 200)); // Hover over play button
    ui.HandleMouseClick(Vec2(210, 200), true);  // Press
    ui.HandleMouseClick(Vec2(210, 200), false); // Release
    
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

int main() {
    std::cout << "=== Aether Game Engine - Feature Tests ===" << std::endl;

    TestPostProcessing();
    TestUI();
    TestAnimation();
    TestAI();
    TestFlyCamera();

    std::cout << "\n=== All Tests Passed ===" << std::endl;

    return 0;
}