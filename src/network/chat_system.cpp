#include "chat_system.h"
#include "core/logger.h"
#include <algorithm>

namespace vge {

ChatSystem::ChatSystem() {
}

ChatSystem::~ChatSystem() {
}

void ChatSystem::SendMessage(const std::string& message, uint32_t senderId, const std::string& senderName) {
    ChatMessage msg;
    msg.senderId = senderId;
    msg.senderName = senderName;
    msg.message = message;
    msg.timestamp = 0.0f; // Would use actual time
    msg.isSystem = false;
    
    ReceiveMessage(msg);
    
    Logger::Info("[Chat] " + senderName + ": " + message);
}

void ChatSystem::SendSystemMessage(const std::string& message) {
    ChatMessage msg;
    msg.senderId = 0;
    msg.senderName = "System";
    msg.message = message;
    msg.timestamp = 0.0f;
    msg.isSystem = true;
    
    ReceiveMessage(msg);
    
    Logger::Info("[Chat] [System] " + message);
}

void ChatSystem::ReceiveMessage(const ChatMessage& msg) {
    messages.push_back(msg);
    
    // Keep only last 100 messages
    if (messages.size() > 100) {
        messages.erase(messages.begin());
    }
    
    if (onMessageReceived) {
        onMessageReceived(msg);
    }
}

std::vector<ChatMessage> ChatSystem::GetMessages() const {
    return messages;
}

std::vector<ChatMessage> ChatSystem::GetRecentMessages(int count) const {
    int start = static_cast<int>(messages.size()) - count;
    if (start < 0) start = 0;
    
    std::vector<ChatMessage> result;
    for (int i = start; i < static_cast<int>(messages.size()); i++) {
        result.push_back(messages[i]);
    }
    return result;
}

void ChatSystem::ClearMessages() {
    messages.clear();
}

void ChatSystem::OnMessageReceived(std::function<void(const ChatMessage&)> callback) {
    onMessageReceived = callback;
}

} // namespace vge
