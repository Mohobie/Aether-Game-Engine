#pragma once
#include <string>
#include <vector>
#include <functional>

namespace vge {

// ============================================
// Dialog Option
// ============================================
struct DialogOption {
    std::string text;
    std::string nextDialogId;
    std::function<void()> onSelect;
    bool enabled;
    
    DialogOption() : enabled(true) {}
    DialogOption(const std::string& t, const std::string& next = "")
        : text(t), nextDialogId(next), enabled(true) {}
};

// ============================================
// Dialog Node
// ============================================
struct DialogNode {
    std::string id;
    std::string speaker;
    std::string text;
    std::vector<DialogOption> options;
    std::function<void()> onEnter;
    bool isEnd;
    
    DialogNode() : isEnd(false) {}
};

// ============================================
// NPC Dialog System
// ============================================
class NPCDialogSystem {
public:
    NPCDialogSystem();
    ~NPCDialogSystem();

    // Dialog management
    void RegisterDialog(const DialogNode& node);
    void StartDialog(const std::string& nodeId);
    void SelectOption(int optionIndex);
    void EndDialog();
    
    // Current state
    bool IsInDialog() const { return currentNode != nullptr; }
    const DialogNode* GetCurrentNode() const { return currentNode; }
    
    // Callbacks
    std::function<void(const DialogNode&)> onDialogStart;
    std::function<void()> onDialogEnd;
    std::function<void(const std::string&)> onTextDisplay;

private:
    std::vector<DialogNode> dialogs;
    DialogNode* currentNode;
    
    DialogNode* FindNode(const std::string& id);
};

} // namespace vge
