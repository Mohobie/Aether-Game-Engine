#include "npc_dialog.h"
#include "core/logger.h"

namespace vge {

NPCDialogSystem::NPCDialogSystem() : currentNode(nullptr) {
}

NPCDialogSystem::~NPCDialogSystem() {
}

void NPCDialogSystem::RegisterDialog(const DialogNode& node) {
    dialogs.push_back(node);
    Logger::Info("[NPCDialog] Registered dialog node: " + node.id);
}

void NPCDialogSystem::StartDialog(const std::string& nodeId) {
    DialogNode* node = FindNode(nodeId);
    if (!node) {
        Logger::Error("[NPCDialog] Dialog node not found: " + nodeId);
        return;
    }
    
    currentNode = node;
    
    if (node->onEnter) {
        node->onEnter();
    }
    
    if (onDialogStart) {
        onDialogStart(*node);
    }
    
    if (onTextDisplay) {
        onTextDisplay(node->text);
    }
    
    Logger::Info("[NPCDialog] Started dialog: " + node->speaker + " - " + node->text);
}

void NPCDialogSystem::SelectOption(int optionIndex) {
    if (!currentNode) {
        Logger::Error("[NPCDialog] No active dialog");
        return;
    }
    
    if (optionIndex < 0 || optionIndex >= (int)currentNode->options.size()) {
        Logger::Error("[NPCDialog] Invalid option index: " + std::to_string(optionIndex));
        return;
    }
    
    DialogOption& option = currentNode->options[optionIndex];
    
    if (!option.enabled) {
        Logger::Info("[NPCDialog] Option disabled: " + option.text);
        return;
    }
    
    if (option.onSelect) {
        option.onSelect();
    }
    
    if (option.nextDialogId.empty()) {
        EndDialog();
    } else {
        StartDialog(option.nextDialogId);
    }
}

void NPCDialogSystem::EndDialog() {
    if (!currentNode) return;
    
    Logger::Info("[NPCDialog] Ended dialog");
    currentNode = nullptr;
    
    if (onDialogEnd) {
        onDialogEnd();
    }
}

DialogNode* NPCDialogSystem::FindNode(const std::string& id) {
    for (auto& node : dialogs) {
        if (node.id == id) {
            return &node;
        }
    }
    return nullptr;
}

} // namespace vge
