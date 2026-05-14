#include "lan_discovery.h"
#include "core/logger.h"
#include <cstring>
#include <chrono>

namespace vge {

LANDiscovery::LANDiscovery()
    : isBroadcasting(false), isDiscovering(false),
      broadcastPort(7777), broadcastMaxPlayers(20),
      broadcastTimer(0.0f), discoveryTimer(0.0f) {
}

LANDiscovery::~LANDiscovery() {
    StopBroadcasting();
    StopDiscovery();
}

bool LANDiscovery::StartBroadcasting(const std::string& serverName, uint16_t port, int maxPlayers) {
    broadcastName = serverName;
    broadcastPort = port;
    broadcastMaxPlayers = maxPlayers;
    isBroadcasting = true;
    
    Logger::Info("[LANDiscovery] Broadcasting server: " + serverName + " on port " + std::to_string(port));
    return true;
}

void LANDiscovery::StopBroadcasting() {
    isBroadcasting = false;
    Logger::Info("[LANDiscovery] Stopped broadcasting");
}

bool LANDiscovery::StartDiscovery() {
    isDiscovering = true;
    discoveredServers.clear();
    
    Logger::Info("[LANDiscovery] Started discovering LAN servers");
    return true;
}

void LANDiscovery::StopDiscovery() {
    isDiscovering = false;
    Logger::Info("[LANDiscovery] Stopped discovering");
}

std::vector<DiscoveredServer> LANDiscovery::GetDiscoveredServers() {
    return discoveredServers;
}

void LANDiscovery::ClearDiscoveredServers() {
    discoveredServers.clear();
}

void LANDiscovery::OnServerDiscovered(std::function<void(const DiscoveredServer&)> callback) {
    onServerDiscovered = callback;
}

void LANDiscovery::Update(float deltaTime) {
    // Broadcast server presence
    if (isBroadcasting) {
        broadcastTimer += deltaTime;
        if (broadcastTimer >= 2.0f) { // Broadcast every 2 seconds
            broadcastTimer = 0.0f;
            // In a real implementation, this would send UDP broadcast
            Logger::Debug("[LANDiscovery] Broadcasting server presence");
        }
    }
    
    // Discover servers
    if (isDiscovering) {
        discoveryTimer += deltaTime;
        if (discoveryTimer >= 3.0f) { // Check every 3 seconds
            discoveryTimer = 0.0f;
            // In a real implementation, this would listen for UDP broadcasts
            Logger::Debug("[LANDiscovery] Scanning for servers");
        }
    }
}

} // namespace vge
