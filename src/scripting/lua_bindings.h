#pragma once

// Forward declarations
struct lua_State;

namespace vge {

class ScriptEngine;

// ============================================
// Lua C API binding functions
// Each returns int (number of return values on Lua stack)
// ============================================

// World bindings
int lua_world_set_block(lua_State* L);
int lua_world_get_block(lua_State* L);
int lua_world_spawn_entity(lua_State* L);
int lua_world_get_seed(lua_State* L);
int lua_world_set_seed(lua_State* L);

// Entity bindings
int lua_entity_set_position(lua_State* L);
int lua_entity_get_position(lua_State* L);
int lua_entity_set_velocity(lua_State* L);
int lua_entity_get_velocity(lua_State* L);
int lua_entity_set_name(lua_State* L);
int lua_entity_get_name(lua_State* L);
int lua_entity_destroy(lua_State* L);

// Event bindings
int lua_event_on(lua_State* L);
int lua_event_emit(lua_State* L);

// Input bindings
int lua_input_on_key_press(lua_State* L);
int lua_input_on_key_release(lua_State* L);
int lua_input_is_key_pressed(lua_State* L);
int lua_input_get_mouse_delta(lua_State* L);

// UI bindings
int lua_ui_create_button(lua_State* L);
int lua_ui_create_label(lua_State* L);
int lua_ui_create_panel(lua_State* L);
int lua_ui_on_click(lua_State* L);
int lua_ui_set_text(lua_State* L);
int lua_ui_set_position(lua_State* L);
int lua_ui_set_size(lua_State* L);
int lua_ui_set_visible(lua_State* L);

// Audio bindings
int lua_audio_play_sound(lua_State* L);
int lua_audio_play_music(lua_State* L);
int lua_audio_stop_music(lua_State* L);
int lua_audio_set_volume(lua_State* L);

// Crafting bindings
int lua_crafting_register_recipe(lua_State* L);
int lua_crafting_get_recipe(lua_State* L);

// Registration helpers (called by ScriptEngine)
void RegisterWorldBindings(lua_State* L);
void RegisterEntityBindings(lua_State* L);
void RegisterEventBindings(lua_State* L);
void RegisterInputBindings(lua_State* L);
void RegisterUIBindings(lua_State* L);
void RegisterAudioBindings(lua_State* L);
void RegisterCraftingBindings(lua_State* L);

} // namespace vge
