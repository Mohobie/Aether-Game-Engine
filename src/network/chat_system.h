#pragma once
#include <string>
#include <vector>
#include <functional>

namespace vge {

// ============================================
// Chat Message
// ============================================
struct ChatMessage {
    uint32_t senderId;
    std::string senderName;
    std::string message;
    float timestamp;
    bool isSystem;
    
    ChatMessage() : senderId(0), timestamp(0.0f), isSystem(false) {}
};

// ============================================
// Chat System
// ============================================
class ChatSystem {
public:
    ChatSystem();
    ~ChatSystem();

    // Send message
    void SendMessage(const std::string& message, uint32_t senderId = 0, const std::string& senderName = "");
    void SendSystemMessage(const std::string& message);
    
    // Receive message (called from network)
    void ReceiveMessage(const ChatMessage& msg);
    
    // Get messages
    std::vector<ChatMessage> GetMessages() const;
    std::vector<ChatMessage> GetRecentMessages(int count) const;
    void ClearMessages();
    
    // Callback
    void OnMessageReceived(std::function<void(const ChatMessage&)> callback);

private:
    std::vector<ChatMessage> messages;
    std::function<void(const ChatMessage&)> onMessageReceived;
};

} // namespace vge
