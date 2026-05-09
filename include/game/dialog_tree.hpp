#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace game {

struct DialogOption {
    std::string text;
    std::string nextNodeID;
    std::function<bool()> condition;
    std::function<void()> onSelect;
};

struct DialogNode {
    std::string id;
    std::string speaker;
    std::string text;
    std::vector<DialogOption> options;
};

class DialogTree {
public:
    void addNode(DialogNode node);
    const DialogNode* current() const;
    void start(const std::string& nodeID);
    void chooseOption(size_t idx);
    bool hasOptions() const;
private:
    std::unordered_map<std::string, DialogNode> nodes;
    std::string currentNode;
};

} // namespace game
