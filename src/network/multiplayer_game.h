#pragma once
#include "game_server.h"
#include "game_client.h"
#include "platform/window.h"
#include "rendering/renderer.h"
#include "platform/input_manager.h"
#include <memory>
#include <string>

namespace vge {

// ============================================
// Multiplayer Game - Can run as client or server
// ============================================
class MultiplayerGame {
public:
    MultiplayerGame();
    ~MultiplayerGame();

    // Host a server
    bool HostServer(uint16_t port, int maxPlayers = 20);
    
    // Join a server
    bool JoinServer(const std::string& address, uint16_t port);
    
    // Single player (local server)
    bool StartSinglePlayer();
    
    // Main loop
    void Run();
    void Stop();
    
    // State
    bool IsRunning() const { return running; }
    bool IsServer() const { return isServer; }
    bool IsClient() const { return isClient; }

private:
    // Mode flags
    bool isServer;
    bool isClient;
    bool running;
    
    // Server (if hosting)
    std::unique_ptr<GameServer> server;
    
    // Client (if playing)
    std::unique_ptr<GameClient> client;
    
    // Local game (if single player)
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Input> input;
    
    // Game loop
    void RunClientLoop();
    void RunServerLoop();
    void ProcessInput(float deltaTime);
    void Render();
};

} // namespace vge
