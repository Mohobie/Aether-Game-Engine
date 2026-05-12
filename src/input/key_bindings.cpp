#include "key_bindings.h"
namespace vge {
KeyBindings::KeyBindings() {}
void KeyBindings::loadDefaults() {
    defaults["move_forward"] = Key::W;
    defaults["move_backward"] = Key::S;
    defaults["move_left"] = Key::A;
    defaults["move_right"] = Key::D;
    defaults["jump"] = Key::Space;
    defaults["crouch"] = Key::Control;
    defaults["interact"] = Key::E;
    defaults["inventory"] = Key::I;
    defaults["pause"] = Key::Escape;
    resetToDefaults();
}
void KeyBindings::bind(const std::string& action, Key key) { bindings[action] = key; }
Key KeyBindings::getBinding(const std::string& action) const {
    auto it = bindings.find(action);
    return it != bindings.end() ? it->second : Key::Unknown;
}
std::string KeyBindings::getActionName(Key key) const {
    for (const auto& [action, boundKey] : bindings) {
        if (boundKey == key) return action;
    }
    return "";
}
void KeyBindings::resetToDefaults() { bindings = defaults; }
} // namespace vge
