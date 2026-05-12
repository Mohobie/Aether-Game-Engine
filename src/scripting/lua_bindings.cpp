#include "scripting/lua_bindings.h"
#include "scripting/script_engine.h"
#include "voxel/world.h"
#include "voxel/block_registry.h"
#include "entity/entity.h"
#include "platform/input_manager.h"
#include "ui/ui_system.h"
#include "audio/audio_system.h"
#include "core/crafting.h"
#include "math/vec3.h"
#include <iostream>
#include <cstring>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace vge {

// ============================================
// Helper macros and utilities
// ============================================

#define CHECK_ARG_COUNT(L, n) \
    if (lua_gettop(L) < n) { \
        luaL_error(L, "Expected at least %d arguments, got %d", n, lua_gettop(L)); \
        return 0; \
    }

// Get the ScriptEngine pointer from Lua registry
static ScriptEngine* GetScriptEngine(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_script_engine");
    ScriptEngine* engine = static_cast<ScriptEngine*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return engine;
}

// Helper to push a Vec3 as a Lua table
static void PushVec3(lua_State* L, const Vec3& v) {
    lua_newtable(L);
    lua_pushnumber(L, v.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, v.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, v.z);
    lua_setfield(L, -2, "z");
}

// Helper to read a Vec3 from Lua (accepts table or 3 numbers)
static Vec3 ReadVec3(lua_State* L, int index) {
    if (lua_istable(L, index)) {
        Vec3 v;
        lua_getfield(L, index, "x");
        v.x = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, index, "y");
        v.y = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, index, "z");
        v.z = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        return v;
    } else if (lua_gettop(L) >= index + 2) {
        return Vec3(
            static_cast<float>(lua_tonumber(L, index)),
            static_cast<float>(lua_tonumber(L, index + 1)),
            static_cast<float>(lua_tonumber(L, index + 2))
        );
    }
    return Vec3(0, 0, 0);
}

// ============================================
// World Bindings
// ============================================

int lua_world_set_block(lua_State* L) {
    CHECK_ARG_COUNT(L, 4);
    ScriptEngine* engine = GetScriptEngine(L);
    if (!engine) return 0;

    World* world = engine->GetLuaEngine() ? nullptr : nullptr; // We'll get world from engine
    // Actually, we need to access world through the engine pointer stored in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_world");
    World* w = static_cast<World*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!w) {
        lua_pushboolean(L, false);
        return 1;
    }

    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    int z = static_cast<int>(lua_tointeger(L, 3));

    BlockTypeID blockType = BLOCK_AIR;
    if (lua_isinteger(L, 4)) {
        blockType = static_cast<BlockTypeID>(lua_tointeger(L, 4));
    } else if (lua_isstring(L, 4)) {
        const char* blockName = lua_tostring(L, 4);
        BlockRegistry& reg = BlockRegistry::GetInstance();
        blockType = reg.GetBlockId(blockName);
    }

    w->SetBlock(x, y, z, blockType);
    lua_pushboolean(L, true);
    return 1;
}

int lua_world_get_block(lua_State* L) {
    CHECK_ARG_COUNT(L, 3);

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_world");
    World* w = static_cast<World*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!w) {
        lua_pushinteger(L, 0);
        return 1;
    }

    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    int z = static_cast<int>(lua_tointeger(L, 3));

    BlockTypeID block = w->GetBlock(x, y, z);
    lua_pushinteger(L, block);
    return 1;
}

int lua_world_spawn_entity(lua_State* L) {
    CHECK_ARG_COUNT(L, 4);

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_entity_manager");
    EntityManager* em = static_cast<EntityManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!em) {
        lua_pushnil(L);
        return 1;
    }

    const char* name = lua_tostring(L, 1);
    float x = static_cast<float>(lua_tonumber(L, 2));
    float y = static_cast<float>(lua_tonumber(L, 3));
    float z = static_cast<float>(lua_tonumber(L, 4));

    Entity* entity = em->CreateEntity(name ? name : "Entity");
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }

    // Create an entity table with methods that capture the entity ID
    lua_newtable(L);
    lua_pushinteger(L, entity->GetID());
    lua_setfield(L, -2, "__id");

    // SetPosition method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);

        lua_getfield(innerL, LUA_REGISTRYINDEX, "__vge_entity_manager");
        EntityManager* innerEm = static_cast<EntityManager*>(lua_touserdata(innerL, -1));
        lua_pop(innerL, 1);

        if (!innerEm) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        Entity* e = innerEm->GetEntity(id);
        if (!e) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        Vec3 pos = ReadVec3(innerL, 2);
        std::cout << "[Lua] Set position for entity " << e->GetName()
                  << " to (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        lua_pushboolean(innerL, true);
        return 1;
    });
    lua_setfield(L, -2, "SetPosition");

    // GetPosition method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);
        PushVec3(innerL, Vec3(0, 0, 0));
        return 1;
    });
    lua_setfield(L, -2, "GetPosition");

    // SetVelocity method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);

        lua_getfield(innerL, LUA_REGISTRYINDEX, "__vge_entity_manager");
        EntityManager* innerEm = static_cast<EntityManager*>(lua_touserdata(innerL, -1));
        lua_pop(innerL, 1);

        if (!innerEm) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        Entity* e = innerEm->GetEntity(id);
        if (!e) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        Vec3 vel = ReadVec3(innerL, 2);
        std::cout << "[Lua] Set velocity for entity " << e->GetName()
                  << " to (" << vel.x << ", " << vel.y << ", " << vel.z << ")" << std::endl;
        lua_pushboolean(innerL, true);
        return 1;
    });
    lua_setfield(L, -2, "SetVelocity");

    // GetVelocity method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        lua_pop(innerL, 1);
        PushVec3(innerL, Vec3(0, 0, 0));
        return 1;
    });
    lua_setfield(L, -2, "GetVelocity");

    // SetName method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);

        lua_getfield(innerL, LUA_REGISTRYINDEX, "__vge_entity_manager");
        EntityManager* innerEm = static_cast<EntityManager*>(lua_touserdata(innerL, -1));
        lua_pop(innerL, 1);

        if (!innerEm || !lua_isstring(innerL, 2)) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        Entity* e = innerEm->GetEntity(id);
        if (!e) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        e->SetName(lua_tostring(innerL, 2));
        lua_pushboolean(innerL, true);
        return 1;
    });
    lua_setfield(L, -2, "SetName");

    // GetName method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);

        lua_getfield(innerL, LUA_REGISTRYINDEX, "__vge_entity_manager");
        EntityManager* innerEm = static_cast<EntityManager*>(lua_touserdata(innerL, -1));
        lua_pop(innerL, 1);

        if (!innerEm) {
            lua_pushstring(innerL, "");
            return 1;
        }

        Entity* e = innerEm->GetEntity(id);
        if (!e) {
            lua_pushstring(innerL, "");
            return 1;
        }

        lua_pushstring(innerL, e->GetName().c_str());
        return 1;
    });
    lua_setfield(L, -2, "GetName");

    // Destroy method
    lua_pushcfunction(L, [](lua_State* innerL) -> int {
        lua_getfield(innerL, 1, "__id");
        EntityID id = static_cast<EntityID>(lua_tointeger(innerL, -1));
        lua_pop(innerL, 1);

        lua_getfield(innerL, LUA_REGISTRYINDEX, "__vge_entity_manager");
        EntityManager* innerEm = static_cast<EntityManager*>(lua_touserdata(innerL, -1));
        lua_pop(innerL, 1);

        if (!innerEm) {
            lua_pushboolean(innerL, false);
            return 1;
        }

        innerEm->DestroyEntity(id);
        lua_pushboolean(innerL, true);
        return 1;
    });
    lua_setfield(L, -2, "Destroy");

    return 1;
}

int lua_world_get_seed(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_world");
    World* w = static_cast<World*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (w) {
        lua_pushinteger(L, w->GetSeed());
    } else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

int lua_world_set_seed(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_world");
    World* w = static_cast<World*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (w) {
        w->SetSeed(static_cast<int>(lua_tointeger(L, 1)));
    }
    return 0;
}

// ============================================
// Entity Bindings
// ============================================

static Entity* GetEntityByID(lua_State* L, int index) {
    if (!lua_isinteger(L, index)) return nullptr;
    EntityID id = static_cast<EntityID>(lua_tointeger(L, index));

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_entity_manager");
    EntityManager* em = static_cast<EntityManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!em) return nullptr;
    return em->GetEntity(id);
}

// Entity methods are now embedded in the table returned by SpawnEntity
// These standalone functions are kept for backward compatibility but not registered
int lua_entity_set_position(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity) {
        lua_pushboolean(L, false);
        return 1;
    }

    Vec3 pos = ReadVec3(L, 2);
    std::cout << "[Lua] Set position for entity " << entity->GetName()
              << " to (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    lua_pushboolean(L, true);
    return 1;
}

int lua_entity_get_position(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }
    PushVec3(L, Vec3(0, 0, 0));
    return 1;
}

int lua_entity_set_velocity(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity) {
        lua_pushboolean(L, false);
        return 1;
    }

    Vec3 vel = ReadVec3(L, 2);
    std::cout << "[Lua] Set velocity for entity " << entity->GetName()
              << " to (" << vel.x << ", " << vel.y << ", " << vel.z << ")" << std::endl;
    lua_pushboolean(L, true);
    return 1;
}

int lua_entity_get_velocity(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }
    PushVec3(L, Vec3(0, 0, 0));
    return 1;
}

int lua_entity_set_name(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity || !lua_isstring(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    entity->SetName(lua_tostring(L, 2));
    lua_pushboolean(L, true);
    return 1;
}

int lua_entity_get_name(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    Entity* entity = GetEntityByID(L, 1);
    if (!entity) {
        lua_pushstring(L, "");
        return 1;
    }

    lua_pushstring(L, entity->GetName().c_str());
    return 1;
}

int lua_entity_destroy(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_entity_manager");
    EntityManager* em = static_cast<EntityManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!em || !lua_isinteger(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    EntityID id = static_cast<EntityID>(lua_tointeger(L, 1));
    em->DestroyEntity(id);
    lua_pushboolean(L, true);
    return 1;
}

// ============================================
// Event Bindings
// ============================================

// Simple event system using Lua tables
int lua_event_on(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* eventName = lua_tostring(L, 1);

    // Store callback in registry: __vge_events[eventName] = function
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_events");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "__vge_events");
        lua_getfield(L, LUA_REGISTRYINDEX, "__vge_events");
    }

    lua_pushstring(L, eventName);
    lua_pushvalue(L, 2); // Copy the function
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_pushboolean(L, true);
    return 1;
}

int lua_event_emit(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) return 0;

    const char* eventName = lua_tostring(L, 1);

    // Get events table
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_events");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    lua_getfield(L, -1, eventName);
    if (lua_isfunction(L, -1)) {
        // Copy any additional arguments
        int nargs = lua_gettop(L) - 3; // -3 for events table, function, and original stack
        if (nargs > 0) {
            for (int i = 0; i < nargs; ++i) {
                lua_pushvalue(L, 2 + i);
            }
        }
        lua_pcall(L, nargs, 0, 0);
    }
    lua_pop(L, 2); // pop function and events table

    return 0;
}

// ============================================
// Input Bindings
// ============================================

static KeyCode StringToKeyCode(const std::string& key) {
    static const std::unordered_map<std::string, KeyCode> keyMap = {
        {"W", KeyCode::W}, {"A", KeyCode::A}, {"S", KeyCode::S}, {"D", KeyCode::D},
        {"Space", KeyCode::Space}, {"Escape", KeyCode::Escape}, {"Enter", KeyCode::Enter},
        {"Up", KeyCode::Up}, {"Down", KeyCode::Down}, {"Left", KeyCode::Left}, {"Right", KeyCode::Right},
        {"E", KeyCode::E}, {"Q", KeyCode::Q}, {"Shift", KeyCode::Shift}, {"Ctrl", KeyCode::Ctrl},
        {"1", KeyCode::Key1}, {"2", KeyCode::Key2}, {"3", KeyCode::Key3},
        {"4", KeyCode::Key4}, {"5", KeyCode::Key5}, {"6", KeyCode::Key6},
        {"7", KeyCode::Key7}, {"8", KeyCode::Key8}, {"9", KeyCode::Key9},
    };

    auto it = keyMap.find(key);
    if (it != keyMap.end()) return it->second;
    return KeyCode::Count;
}

int lua_input_on_key_press(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* keyName = lua_tostring(L, 1);

    // Store in registry: __vge_input[keyName] = function
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input_press");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "__vge_input_press");
        lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input_press");
    }

    lua_pushstring(L, keyName);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_pushboolean(L, true);
    return 1;
}

int lua_input_on_key_release(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* keyName = lua_tostring(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input_release");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "__vge_input_release");
        lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input_release");
    }

    lua_pushstring(L, keyName);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_pushboolean(L, true);
    return 1;
}

int lua_input_is_key_pressed(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input");
    Input* input = static_cast<Input*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!input) {
        lua_pushboolean(L, false);
        return 1;
    }

    KeyCode key = StringToKeyCode(lua_tostring(L, 1));
    if (key != KeyCode::Count) {
        lua_pushboolean(L, input->IsKeyPressed(key));
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

int lua_input_get_mouse_delta(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_input");
    Input* input = static_cast<Input*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!input) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 2;
    }

    float dx, dy;
    input->GetMouseDelta(dx, dy);
    lua_pushnumber(L, dx);
    lua_pushnumber(L, dy);
    return 2;
}

// ============================================
// UI Bindings
// ============================================

int lua_ui_create_button(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* name = lua_tostring(L, 1);
    const char* text = lua_tostring(L, 2);
    UIButton* btn = ui->CreateButton(name, text);

    lua_pushboolean(L, btn != nullptr);
    return 1;
}

int lua_ui_create_label(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) {
        lua_pushboolean(L, false);
        return 1;
    }

    UILabel* label = ui->CreateLabel(lua_tostring(L, 1), lua_tostring(L, 2));
    lua_pushboolean(L, label != nullptr);
    return 1;
}

int lua_ui_create_panel(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) {
        lua_pushboolean(L, false);
        return 1;
    }

    UIPanel* panel = ui->CreatePanel(lua_tostring(L, 1));
    lua_pushboolean(L, panel != nullptr);
    return 1;
}

int lua_ui_on_click(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isfunction(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* elementName = lua_tostring(L, 1);
    UIElement* element = ui->FindElement(elementName);

    if (!element) {
        lua_pushboolean(L, false);
        return 1;
    }

    // Store callback in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui_callbacks");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "__vge_ui_callbacks");
        lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui_callbacks");
    }

    lua_pushstring(L, elementName);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    // Set C++ callback that will call Lua
    element->SetOnClick([L, elementName]() {
        lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui_callbacks");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, elementName);
            if (lua_isfunction(L, -1)) {
                lua_pcall(L, 0, 0, 0);
            } else {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    });

    lua_pushboolean(L, true);
    return 1;
}

int lua_ui_set_text(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) return 0;

    UIElement* element = ui->FindElement(lua_tostring(L, 1));
    if (!element) return 0;

    // Try to cast to label
    // Note: We'd need dynamic_cast or a type system for proper casting
    // For now, just log it
    std::cout << "[Lua] Set text for " << lua_tostring(L, 1)
              << " to: " << lua_tostring(L, 2) << std::endl;
    return 0;
}

int lua_ui_set_position(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) return 0;

    UIElement* element = ui->FindElement(lua_tostring(L, 1));
    if (!element) return 0;

    Vec2 pos;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        pos.x = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        pos.y = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    } else if (lua_gettop(L) >= 3) {
        pos.x = static_cast<float>(lua_tonumber(L, 2));
        pos.y = static_cast<float>(lua_tonumber(L, 3));
    }

    element->SetPosition(pos);
    return 0;
}

int lua_ui_set_size(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) return 0;

    UIElement* element = ui->FindElement(lua_tostring(L, 1));
    if (!element) return 0;

    Vec2 size;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        size.x = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        size.y = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    } else if (lua_gettop(L) >= 3) {
        size.x = static_cast<float>(lua_tonumber(L, 2));
        size.y = static_cast<float>(lua_tonumber(L, 3));
    }

    element->SetSize(size);
    return 0;
}

int lua_ui_set_visible(lua_State* L) {
    CHECK_ARG_COUNT(L, 2);
    if (!lua_isstring(L, 1) || !lua_isboolean(L, 2)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_ui");
    UIManager* ui = static_cast<UIManager*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ui) return 0;

    UIElement* element = ui->FindElement(lua_tostring(L, 1));
    if (!element) return 0;

    element->SetVisible(lua_toboolean(L, 2));
    return 0;
}

// ============================================
// Audio Bindings
// ============================================

int lua_audio_play_sound(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_audio");
    AudioSystem* audio = static_cast<AudioSystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!audio) {
        lua_pushboolean(L, false);
        return 1;
    }

    const char* soundName = lua_tostring(L, 1);
    Vec3 pos(0, 0, 0);
    if (lua_gettop(L) >= 4) {
        pos.x = static_cast<float>(lua_tonumber(L, 2));
        pos.y = static_cast<float>(lua_tonumber(L, 3));
        pos.z = static_cast<float>(lua_tonumber(L, 4));
    }

    audio->PlayOneShot(soundName, pos);
    lua_pushboolean(L, true);
    return 1;
}

int lua_audio_play_music(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    // Music playback would need a music system
    // For now, just log it
    std::cout << "[Lua] Play music: " << lua_tostring(L, 1) << std::endl;
    lua_pushboolean(L, true);
    return 1;
}

int lua_audio_stop_music(lua_State* L) {
    std::cout << "[Lua] Stop music" << std::endl;
    return 0;
}

int lua_audio_set_volume(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isnumber(L, 1)) return 0;

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_audio");
    AudioSystem* audio = static_cast<AudioSystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (audio) {
        audio->SetMasterVolume(static_cast<float>(lua_tonumber(L, 1)));
    }
    return 0;
}

// ============================================
// Crafting Bindings
// ============================================

int lua_crafting_register_recipe(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_istable(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_crafting");
    CraftingSystem* crafting = static_cast<CraftingSystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!crafting) {
        lua_pushboolean(L, false);
        return 1;
    }

    // Parse recipe from table
    CraftingRecipe recipe;

    lua_getfield(L, 1, "id");
    if (lua_isstring(L, -1)) recipe.recipeID = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "output");
    if (lua_isstring(L, -1)) recipe.outputItemID = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "outputCount");
    if (lua_isinteger(L, -1)) recipe.outputCount = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, 1, "shapeless");
    if (lua_isboolean(L, -1)) recipe.shapeless = lua_toboolean(L, -1);
    lua_pop(L, 1);

    // Parse ingredients
    lua_getfield(L, 1, "ingredients");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            if (lua_istable(L, -1)) {
                RecipeIngredient ing;
                lua_getfield(L, -1, "item");
                if (lua_isstring(L, -1)) ing.itemID = lua_tostring(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "count");
                if (lua_isinteger(L, -1)) ing.count = static_cast<int>(lua_tointeger(L, -1));
                lua_pop(L, 1);

                recipe.ingredients.push_back(ing);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    crafting->AddRecipe(recipe);
    lua_pushboolean(L, true);
    return 1;
}

int lua_crafting_get_recipe(lua_State* L) {
    CHECK_ARG_COUNT(L, 1);
    if (!lua_isstring(L, 1)) {
        lua_pushnil(L);
        return 1;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "__vge_crafting");
    CraftingSystem* crafting = static_cast<CraftingSystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!crafting) {
        lua_pushnil(L);
        return 1;
    }

    const CraftingRecipe* recipe = crafting->GetRecipe(lua_tostring(L, 1));
    if (!recipe) {
        lua_pushnil(L);
        return 1;
    }

    // Return recipe as table
    lua_newtable(L);
    lua_pushstring(L, recipe->recipeID.c_str());
    lua_setfield(L, -2, "id");
    lua_pushstring(L, recipe->outputItemID.c_str());
    lua_setfield(L, -2, "output");
    lua_pushinteger(L, recipe->outputCount);
    lua_setfield(L, -2, "outputCount");

    return 1;
}

// ============================================
// Registration Functions
// ============================================

void RegisterWorldBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_world_set_block);
    lua_setfield(L, -2, "SetBlock");
    lua_pushcfunction(L, lua_world_get_block);
    lua_setfield(L, -2, "GetBlock");
    lua_pushcfunction(L, lua_world_spawn_entity);
    lua_setfield(L, -2, "SpawnEntity");
    lua_pushcfunction(L, lua_world_get_seed);
    lua_setfield(L, -2, "GetSeed");
    lua_pushcfunction(L, lua_world_set_seed);
    lua_setfield(L, -2, "SetSeed");
    lua_setglobal(L, "world");
}

void RegisterEntityBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_entity_set_position);
    lua_setfield(L, -2, "SetPosition");
    lua_pushcfunction(L, lua_entity_get_position);
    lua_setfield(L, -2, "GetPosition");
    lua_pushcfunction(L, lua_entity_set_velocity);
    lua_setfield(L, -2, "SetVelocity");
    lua_pushcfunction(L, lua_entity_get_velocity);
    lua_setfield(L, -2, "GetVelocity");
    lua_pushcfunction(L, lua_entity_set_name);
    lua_setfield(L, -2, "SetName");
    lua_pushcfunction(L, lua_entity_get_name);
    lua_setfield(L, -2, "GetName");
    lua_pushcfunction(L, lua_entity_destroy);
    lua_setfield(L, -2, "Destroy");
    lua_setglobal(L, "entity");
}

void RegisterEventBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_event_on);
    lua_setfield(L, -2, "On");
    lua_pushcfunction(L, lua_event_emit);
    lua_setfield(L, -2, "Emit");
    lua_setglobal(L, "event");
}

void RegisterInputBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_input_on_key_press);
    lua_setfield(L, -2, "OnKeyPress");
    lua_pushcfunction(L, lua_input_on_key_release);
    lua_setfield(L, -2, "OnKeyRelease");
    lua_pushcfunction(L, lua_input_is_key_pressed);
    lua_setfield(L, -2, "IsKeyPressed");
    lua_pushcfunction(L, lua_input_get_mouse_delta);
    lua_setfield(L, -2, "GetMouseDelta");
    lua_setglobal(L, "input");
}

void RegisterUIBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_ui_create_button);
    lua_setfield(L, -2, "CreateButton");
    lua_pushcfunction(L, lua_ui_create_label);
    lua_setfield(L, -2, "CreateLabel");
    lua_pushcfunction(L, lua_ui_create_panel);
    lua_setfield(L, -2, "CreatePanel");
    lua_pushcfunction(L, lua_ui_on_click);
    lua_setfield(L, -2, "OnClick");
    lua_pushcfunction(L, lua_ui_set_text);
    lua_setfield(L, -2, "SetText");
    lua_pushcfunction(L, lua_ui_set_position);
    lua_setfield(L, -2, "SetPosition");
    lua_pushcfunction(L, lua_ui_set_size);
    lua_setfield(L, -2, "SetSize");
    lua_pushcfunction(L, lua_ui_set_visible);
    lua_setfield(L, -2, "SetVisible");
    lua_setglobal(L, "ui");
}

void RegisterAudioBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_audio_play_sound);
    lua_setfield(L, -2, "PlaySound");
    lua_pushcfunction(L, lua_audio_play_music);
    lua_setfield(L, -2, "PlayMusic");
    lua_pushcfunction(L, lua_audio_stop_music);
    lua_setfield(L, -2, "StopMusic");
    lua_pushcfunction(L, lua_audio_set_volume);
    lua_setfield(L, -2, "SetVolume");
    lua_setglobal(L, "audio");
}

void RegisterCraftingBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, lua_crafting_register_recipe);
    lua_setfield(L, -2, "RegisterRecipe");
    lua_pushcfunction(L, lua_crafting_get_recipe);
    lua_setfield(L, -2, "GetRecipe");
    lua_setglobal(L, "crafting");
}

} // namespace vge
