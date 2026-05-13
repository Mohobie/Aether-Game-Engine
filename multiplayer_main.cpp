#include "network/multiplayer_game.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    vge::MultiplayerGame game;
    
    std::cout << "=================================" << std::endl;
    std::cout << "  Aether Voxel - Multiplayer" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
    
    if (argc < 2) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  Server:   ./multiplayer server [port]" << std::endl;
        std::cout << "  Client:   ./multiplayer client <address> [port]" << std::endl;
        std::cout << "  Single:   ./multiplayer single" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  ./multiplayer server 7777" << std::endl;
        std::cout << "  ./multiplayer client localhost 7777" << std::endl;
        std::cout << "  ./multiplayer single" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "server") {
        uint16_t port = 7777;
        if (argc >= 3) {
            port = static_cast<uint16_t>(std::stoi(argv[2]));
        }
        
        std::cout << "Starting server on port " << port << "..." << std::endl;
        
        if (!game.HostServer(port)) {
            std::cerr << "Failed to start server!" << std::endl;
            return 1;
        }
        
        std::cout << "Server running! Press Ctrl+C to stop." << std::endl;
        game.Run();
        
    } else if (mode == "client") {
        if (argc < 3) {
            std::cerr << "Error: Client mode requires server address" << std::endl;
            return 1;
        }
        
        std::string address = argv[2];
        uint16_t port = 7777;
        if (argc >= 4) {
            port = static_cast<uint16_t>(std::stoi(argv[3]));
        }
        
        std::cout << "Connecting to " << address << ":" << port << "..." << std::endl;
        
        if (!game.JoinServer(address, port)) {
            std::cerr << "Failed to connect to server!" << std::endl;
            return 1;
        }
        
        std::cout << "Connected! Use WASD to move, mouse to look." << std::endl;
        std::cout << "Q = break block, E = place block" << std::endl;
        game.Run();
        
    } else if (mode == "single") {
        std::cout << "Starting single player..." << std::endl;
        
        if (!game.StartSinglePlayer()) {
            std::cerr << "Failed to start single player!" << std::endl;
            return 1;
        }
        
        std::cout << "Single player started! Use WASD to move, mouse to look." << std::endl;
        std::cout << "Q = break block, E = place block" << std::endl;
        game.Run();
        
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
