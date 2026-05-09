#include "network_manager.h"
#include <iostream>

namespace vge {

NetworkManager::NetworkManager() : initialized(false), port(7777) {}

NetworkManager::~NetworkManager() {
    if (initialized) Shutdown();
}

bool NetworkManager::Initialize(int listenPort) {
    port = listenPort;
    std::cout << "[Network] Initializing on port " << port << " (stub)" << std::endl;
    initialized = true;
    return true;
}

void NetworkManager::Shutdown() {
    std::cout << "[Network] Shutting down" << std::endl;
    initialized = false;
}

void NetworkManager::SendPacket(const Packet& packet, const std::string& address) {
    // Stub: Would send UDP packet
}

void NetworkManager::BroadcastPacket(const Packet& packet) {
    // Stub: Would broadcast to all connected clients
}

void NetworkManager::Update() {
    // Stub: Would process incoming packets
}

bool NetworkManager::IsInitialized() const {
    return initialized;
}

} // namespace vge
