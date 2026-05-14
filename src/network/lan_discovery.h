#pragma once
#include <string>
#include <vector>
#include <functional>

namespace vge {

// ============================================
// LAN Server Discovery
// ============================================
struct DiscoveredServer {
    std::string name;
    std::string address;
    uint16_t port;
    int playerCount;
    int maxPlayers;
    int ping;
    std::string version;
};

class LANDiscovery {
public:
    LANDiscovery();
    ~LANDiscovery();

    // Start broadcasting server presence
    bool StartBroadcasting(const std::string& serverName, uint16_t port, int maxPlayers);
    void StopBroadcasting();

    // Start discovering servers
    bool StartDiscovery();
    void StopDiscovery();

    // Get discovered servers
    std::vector<DiscoveredServer> GetDiscoveredServers();
    void ClearDiscoveredServers();

    // Callbacks
    void OnServerDiscovered(std::function<void(const DiscoveredServer&)> callback);

    // Update (call every frame)
    void Update(float deltaTime);

private:
    bool isBroadcasting;
    bool isDiscovering;
    std::string broadcastName;
    uint16_t broadcastPort;
    int broadcastMaxPlayers;

    std::vector<DiscoveredServer> discoveredServers;
    std::function<void(const DiscoveredServer&)> onServerDiscovered;

    float broadcastTimer;
    float discoveryTimer;
};

} // namespace vge
