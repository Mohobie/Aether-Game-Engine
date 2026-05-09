#include "config_manager.h"
#include <fstream>
#include <sstream>
namespace aether {
ConfigManager::ConfigManager() {}
bool ConfigManager::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return false;
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // Try to parse as different types
            if (value == "true" || value == "false") {
                values[key] = (value == "true");
            } else {
                try {
                    size_t idx;
                    int intVal = std::stoi(value, &idx);
                    if (idx == value.size()) values[key] = intVal;
                    else {
                        float floatVal = std::stof(value);
                        values[key] = floatVal;
                    }
                } catch (...) {
                    values[key] = value;
                }
            }
        }
    }
    return true;
}
bool ConfigManager::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file) return false;
    for (const auto& [key, value] : values) {
        file << key << "=";
        std::visit([&file](const auto& v) { file << v; }, value);
        file << "
";
    }
    return true;
}
void ConfigManager::setValue(const std::string& key, const ConfigValue& value) { values[key] = value; }
ConfigValue ConfigManager::getValue(const std::string& key) const {
    auto it = values.find(key);
    if (it != values.end()) return it->second;
    auto def = defaults.find(key);
    if (def != defaults.end()) return def->second;
    return ConfigValue{};
}
bool ConfigManager::hasKey(const std::string& key) const { return values.find(key) != values.end(); }
void ConfigManager::setDefault(const std::string& key, const ConfigValue& value) { defaults[key] = value; }
void ConfigManager::clear() { values.clear(); }
} // namespace aether
