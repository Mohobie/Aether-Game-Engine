#pragma once
#include <string>
#include <unordered_map>
#include <functional>

namespace vge {

// ============================================
// Key Bindings Configuration
// ============================================
class KeyBindings {
public:
    KeyBindings();
    ~KeyBindings();

    // Load/Save
    bool LoadFromFile(const std::string& filepath);
    bool SaveToFile(const std::string& filepath);
    
    // Get/Set bindings
    int GetKey(const std::string& action) const;
    void SetKey(const std::string& action, int keyCode);
    
    // Default bindings
    void ResetToDefaults();
    
    // Check if key is bound
    bool IsBound(const std::string& action) const;
    
    // Get all bindings
    std::unordered_map<std::string, int> GetAllBindings() const;

private:
    std::unordered_map<std::string, int> bindings;
    
    void SetDefaults();
};

} // namespace vge
