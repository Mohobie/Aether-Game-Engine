#pragma once
#include <string>

namespace vge {

struct Packet {
    std::string data;
    std::string address;
    int port;
};

class NetworkManager {
private:
    bool initialized;
    int port;

public:
    NetworkManager();
    ~NetworkManager();

    bool Initialize(int listenPort = 7777);
    void Shutdown();

    void SendPacket(const Packet& packet, const std::string& address);
    void BroadcastPacket(const Packet& packet);
    void Update();

    bool IsInitialized() const;
};

} // namespace vge
