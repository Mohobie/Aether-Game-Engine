#pragma once
#include <string>
#include <unordered_map>
#include <variant>
namespace vge {
using ConfigValue = std::variant<std::string, int, float, bool>;
class ConfigManager {
public:
    ConfigManager();
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;
    void setValue(const std::string& key, const ConfigValue& value);
    ConfigValue getValue(const std::string& key) const;
    bool hasKey(const std::string& key) const;
    void setDefault(const std::string& key, const ConfigValue& value);
    void clear();
private:
    std::unordered_map<std::string, ConfigValue> values;
    std::unordered_map<std::string, ConfigValue> defaults;
};
} // namespace vge
