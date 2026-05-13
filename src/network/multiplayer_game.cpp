#include "multiplayer_game.h"
#include "game_network_protocol.h"
#include "voxel/world_generator.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace vge {

MultiplayerGame::MultiplayerGame()
    : isServer(false), isClient(false), running(false) {
}

MultiplayerGame::~MultiplayerGame() {
    Stop();
}

bool MultiplayerGame::HostServer(uint16_t port, int maxPlayers) {
    // Create server
    server = std::make_unique<GameServer>(port);
    server->SetMaxPlayers(maxPlayers);
    
    if (!server->Start()) {
        std::cerr << "[MultiplayerGame] Failed to start server" << std::endl;
        return false;
    }
    
    // Also create a local client to play on the server
    client = std::make_unique<GameClient>();
    if (!client->Connect("localhost", port)) {
        std::cerr << "[MultiplayerGame] Failed to connect local client to server" << std::endl;
        server->Stop();
        return false;
    }
    
    // Create window and renderer for local play
    window = std::make_unique<Window>();
    if (!window->Initialize(1280, 720, "Aether Voxel - Server")) {
        std::cerr << "[MultiplayerGame] Failed to create window" << std::endl;
        return false;
    }
    
    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize()) {
        std::cerr << "[MultiplayerGame] Failed to initialize renderer" << std::endl;
        return false;
    }
    renderer->SetViewport(0, 0, 1280, 720);
    
    input = std::make_unique<Input>();
    
    isServer = true;
    isClient = true;
    running = true;
    
    std::cout << "[MultiplayerGame] Hosting server on port " << port << std::endl;
    return true;
}

bool MultiplayerGame::JoinServer(const std::string& address, uint16_t port) {
    // Create client
    client = std::make_unique<GameClient>();
    
    if (!client->Connect(address, port)) {
        std::cerr << "[MultiplayerGame] Failed to connect to server" << std::endl;
        return false;
    }
    
    // Create window and renderer
    window = std::make_unique<Window>();
    if (!window->Initialize(1280, 720, "Aether Voxel - Client")) {
        std::cerr << "[MultiplayerGame] Failed to create window" << std::endl;
        return false;
    }
    
    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize()) {
        std::cerr << "[MultiplayerGame] Failed to initialize renderer" << std::endl;
        return false;
    }
    renderer->SetViewport(0, 0, 1280, 720);
    
    input = std::make_unique<Input>();
    
    isClient = true;
    running = true;
    
    std::cout << "[MultiplayerGame] Connected to " << address << ":" << port << std::endl;
    return true;
}

bool MultiplayerGame::StartSinglePlayer() {
    // Single player is just a local server + client
    return HostServer(7777, 1);
}

void MultiplayerGame::Run() {
    if (!running) return;
    
    if (isClient) {
        RunClientLoop();
    } else if (isServer) {
        RunServerLoop();
    }
}

void MultiplayerGame::Stop() {
    running = false;
    
    if (client) {
        client->Disconnect();
        client.reset();
    }
    
    if (server) {
        server->Stop();
        server.reset();
    }
    
    if (renderer) {
        renderer->Shutdown();
        renderer.reset();
    }
    
    if (window) {
        window->Shutdown();
        window.reset();
    }
    
    isServer = false;
    isClient = false;
}

void MultiplayerGame::RunClientLoop() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Handle window events
        window->PollEvents();
        if (window->ShouldClose()) {
            running = false;
            break;
        }
        
        // Update input
        input->Update();
        
        // Process input and send to server
        ProcessInput(deltaTime);
        
        // Update client (network, interpolation)
        client->Update(deltaTime);
        
        // Update server if hosting
        if (isServer && server) {
            server->Update(deltaTime);
        }
        
        // Render
        Render();
        
        // Swap buffers
        window->SwapBuffers();
    }
}

void MultiplayerGame::RunServerLoop() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update server
        server->Update(deltaTime);
        
        // Simple sleep to not consume 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void MultiplayerGame::ProcessInput(float deltaTime) {
    if (!client || !client->IsConnected()) return;
    
    PlayerController* player = client->GetLocalPlayer();
    if (!player) return;
    
    // Build input message
    PlayerInputMsg inputMsg;
    
    // Movement
    inputMsg.moveForward = 0.0f;
    inputMsg.moveRight = 0.0f;
    
    if (input->IsKeyPressed(KeyCode::W)) inputMsg.moveForward += 1.0f;
    if (input->IsKeyPressed(KeyCode::S)) inputMsg.moveForward -= 1.0f;
    if (input->IsKeyPressed(KeyCode::D)) inputMsg.moveRight += 1.0f;
    if (input->IsKeyPressed(KeyCode::A)) inputMsg.moveRight -= 1.0f;
    
    // Actions
    inputMsg.jump = input->IsKeyPressed(KeyCode::Space);
    inputMsg.sprint = input->IsKeyPressed(KeyCode::Shift);
    
    // Mouse look
    float dx, dy;
    input->GetMouseDelta(dx, dy);
    
    // Update local player rotation immediately for responsiveness
    float newYaw = player->GetYaw() + dx * 0.15f;
    float newPitch = player->GetPitch() - dy * 0.15f;
    
    // Clamp pitch
    if (newPitch > 89.0f) newPitch = 89.0f;
    if (newPitch < -89.0f) newPitch = -89.0f;
    
    inputMsg.yaw = newYaw;
    inputMsg.pitch = newPitch;
    
    // Send to server
    client->SendPlayerInput(inputMsg);
    
    // Update local player immediately (client prediction)
    player->Update(deltaTime, *input, *client->GetWorld());
    
    // Block interaction
    if (input->IsKeyJustPressed(KeyCode::Q)) {
        // Break block
        Vec3 lookDir = player->GetLookDirection();
        Vec3 eyePos = player->GetPosition() + Vec3(0, 1.6f, 0);
        
        // Ray cast to find block
        for (float t = 0; t < 5.0f; t += 0.1f) {
            Vec3 checkPos = eyePos + lookDir * t;
            int bx = static_cast<int>(std::floor(checkPos.x));
            int by = static_cast<int>(std::floor(checkPos.y));
            int bz = static_cast<int>(std::floor(checkPos.z));
            
            BlockTypeID block = client->GetWorld()->GetBlock(bx, by, bz);
            if (block != BlockRegistry::GetInstance().GetBlockId("air")) {
                client->SendBlockBreak(bx, by, bz);
                break;
            }
        }
    }
    
    if (input->IsKeyJustPressed(KeyCode::E)) {
        // Place block
        Vec3 lookDir = player->GetLookDirection();
        Vec3 eyePos = player->GetPosition() + Vec3(0, 1.6f, 0);
        
        Vec3 prevPos = eyePos;
        for (float t = 0; t < 5.0f; t += 0.1f) {
            Vec3 checkPos = eyePos + lookDir * t;
            int bx = static_cast<int>(std::floor(checkPos.x));
            int by = static_cast<int>(std::floor(checkPos.y));
            int bz = static_cast<int>(std::floor(checkPos.z));
            
            BlockTypeID block = client->GetWorld()->GetBlock(bx, by, bz);
            if (block != BlockRegistry::GetInstance().GetBlockId("air")) {
                // Place in front of this block
                int px = static_cast<int>(std::floor(prevPos.x));
                int py = static_cast<int>(std::floor(prevPos.y));
                int pz = static_cast<int>(std::floor(prevPos.z));
                
                client->SendBlockPlace(px, py, pz, BlockRegistry::GetInstance().GetBlockId("stone"));
                break;
            }
            prevPos = checkPos;
        }
    }
    
    // Chat
    if (input->IsKeyJustPressed(KeyCode::Enter)) {
        client->SendChatMessage("Hello from client!");
    }
}

void MultiplayerGame::Render() {
    if (!renderer || !client) return;
    
    // Set sky color based on time
    float dayTime = client->GetDayTime();
    float t = dayTime / 24.0f;
    
    // Simple day/night color
    float r, g, b;
    if (t < 0.25f || t > 0.75f) {
        // Night
        r = 0.05f; g = 0.05f; b = 0.1f;
    } else if (t < 0.3f || t > 0.7f) {
        // Dawn/Dusk
        r = 0.8f; g = 0.5f; b = 0.3f;
    } else {
        // Day
        r = 0.53f; g = 0.81f; b = 0.98f;
    }
    
    renderer->SetClearColor(r, g, b, 1.0f);
    renderer->BeginFrame();
    
    // Render world
    renderer->RenderWorld(*client->GetWorld(), *client->GetCamera());
    
    // Render remote players
    for (const auto& [entityId, remotePlayer] : client->GetRemotePlayers()) {
        // Render a simple cube for each remote player
        // In a full implementation, you'd render a player model
        glPushMatrix();
        glTranslatef(remotePlayer->position.x, remotePlayer->position.y + 0.9f, remotePlayer->position.z);
        glScalef(0.3f, 0.9f, 0.3f);
        glColor3f(1.0f, 0.0f, 0.0f); // Red for other players
        
        glBegin(GL_QUADS);
        // Simple cube
        glVertex3f(-1, -1, 1); glVertex3f(1, -1, 1); glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);
        glVertex3f(-1, -1, -1); glVertex3f(-1, 1, -1); glVertex3f(1, 1, -1); glVertex3f(1, -1, -1);
        glVertex3f(-1, 1, -1); glVertex3f(-1, 1, 1); glVertex3f(1, 1, 1); glVertex3f(1, 1, -1);
        glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1); glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1);
        glVertex3f(1, -1, -1); glVertex3f(1, 1, -1); glVertex3f(1, 1, 1); glVertex3f(1, -1, 1);
        glVertex3f(-1, -1, -1); glVertex3f(-1, -1, 1); glVertex3f(-1, 1, 1); glVertex3f(-1, 1, -1);
        glEnd();
        
        glPopMatrix();
    }
    
    // Render crosshair
    renderer->RenderCrosshair(1280, 720);
    
    renderer->EndFrame();
}

} // namespace vge
