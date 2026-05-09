#pragma once
#include <functional>
#include <vector>
#include <map>
#include <string>
namespace aether {
class EventSystem {
public:
    using Handler = std::function<void(const void*)>;
    template<typename T>
    void subscribe(const std::string& eventType, std::function<void(const T&)> handler) {
        handlers[eventType].push_back([handler](const void* data) {
            handler(*static_cast<const T*>(data));
        });
    }
    template<typename T>
    void emit(const std::string& eventType, const T& data) {
        auto it = handlers.find(eventType);
        if (it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(&data);
            }
        }
    }
private:
    std::map<std::string, std::vector<Handler>> handlers;
};
} // namespace aether
