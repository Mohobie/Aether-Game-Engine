#pragma once
#include "input_manager.h"
#include <string>
#include <unordered_map>
namespace vge {
struct KeyBinding {
    std::string action;
    Key key;
    std::string description;
};
class KeyBindings {
public:
    KeyBindings();
    void loadDefaults();
    void bind(const std::string& action, Key key);
    Key getBinding(const std::string& action) const;
    std::string getActionName(Key key) const;
    void resetToDefaults();
private:
    std::unordered_map<std::string, Key> bindings;
    std::unordered_map<std::string, Key> defaults;
};
} // namespace vge
