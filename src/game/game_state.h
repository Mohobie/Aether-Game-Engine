#pragma once
#include <string>
namespace aether {
enum class GameStateType {
    Menu,
    Playing,
    Paused,
    Settings,
    Loading
};
class GameState {
public:
    virtual ~GameState() = default;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual GameStateType getType() const = 0;
    virtual const char* getName() const = 0;
};
} // namespace aether
