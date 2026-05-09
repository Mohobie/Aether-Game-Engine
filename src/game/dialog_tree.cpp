#include "game/dialog_tree.hpp"

namespace game {

void DialogTree::addNode(DialogNode node) {
    nodes[node.id] = std::move(node);
}

const DialogNode* DialogTree::current() const {
    auto it = nodes.find(currentNode);
    if (it == nodes.end()) return nullptr;
    return &it->second;
}

void DialogTree::start(const std::string& nodeID) {
    currentNode = nodeID;
}

void DialogTree::chooseOption(size_t idx) {
    auto it = nodes.find(currentNode);
    if (it == nodes.end()) return;
    if (idx >= it->second.options.size()) return;
    auto& opt = it->second.options[idx];
    if (opt.condition && !opt.condition()) return;
    if (opt.onSelect) opt.onSelect();
    currentNode = opt.nextNodeID;
}

bool DialogTree::hasOptions() const {
    auto it = nodes.find(currentNode);
    if (it == nodes.end()) return false;
    return !it->second.options.empty();
}

} // namespace game
