cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================
# Platform Detection
# ============================================
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
endif()

# ============================================
# Find Packages (Cross-Platform)
# ============================================
find_package(OpenGL QUIET)

# Find GLFW - try multiple methods
set(GLFW_FOUND FALSE)
if(WIN32)
    find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
        PATHS 
            ${CMAKE_SOURCE_DIR}/third_party/glfw/include
            $ENV{GLFW_DIR}/include
            C:/glfw/include
            "C:/Program Files/GLFW/include"
    )
    find_library(GLFW_LIBRARY glfw3 glfw
        PATHS
            ${CMAKE_SOURCE_DIR}/third_party/glfw/lib
            $ENV{GLFW_DIR}/lib
            C:/glfw/lib
            "C:/Program Files/GLFW/lib"
    )
    if(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
        set(GLFW_FOUND TRUE)
        set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
        set(GLFW_LIBRARIES ${GLFW_LIBRARY})
        message(STATUS "Found GLFW (Windows): ${GLFW_LIBRARY}")
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW QUIET glfw3)
    endif()
    
    if(NOT GLFW_FOUND)
        find_library(GLFW_LIBRARY glfw PATHS /usr/lib /usr/local/lib)
        find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h PATHS /usr/include /usr/local/include)
        if(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)
            set(GLFW_FOUND TRUE)
            set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
            set(GLFW_LIBRARIES ${GLFW_LIBRARY})
        endif()
    endif()
endif()

# Find Lua
set(LUA_FOUND FALSE)
if(WIN32)
    find_library(LUA_LIBRARY lua54 lua53 lua5.4 lua5.3 lua
        PATHS 
            ${CMAKE_SOURCE_DIR}/third_party/lua
            $ENV{LUA_DIR}/lib
            C:/lua/lib
    )
    find_path(LUA_INCLUDE_DIR lua.h
        PATHS
            ${CMAKE_SOURCE_DIR}/third_party/lua
            $ENV{LUA_DIR}/include
            C:/lua/include
    )
    if(LUA_LIBRARY AND LUA_INCLUDE_DIR)
        set(LUA_FOUND TRUE)
        message(STATUS "Found Lua (Windows): ${LUA_LIBRARY}")
    endif()
else()
    find_library(LUA_LIBRARY lua5.4 lua54 lua5.3 lua53 lua PATHS /usr/lib /usr/local/lib)
    if(LUA_LIBRARY)
        set(LUA_FOUND TRUE)
        message(STATUS "Found Lua: ${LUA_LIBRARY}")
    endif()
endif()

# ============================================
# Core Sources (Cross-Platform)
# ============================================
set(CORE_SOURCES
    src/math/vec3.cpp
    src/math/mat4.cpp
    src/ui/ui_system.cpp
    src/ui/settings_menu.cpp
    src/ui/hud_system.cpp
    src/ui/main_menu.cpp
    src/physics/physics_constraints.cpp
    src/network/network_architecture.cpp
    src/network/network_manager.cpp
    src/network/server_authority.cpp
    src/network/entity_replication.cpp
    src/network/networked_game_mode.cpp
    src/network/game_server.cpp
    src/network/game_client.cpp
    src/network/lan_discovery.cpp
    src/network/chat_system.cpp
    src/network/player_skin.cpp
    src/ai/behavior_tree.cpp
    src/ai/enemy_ai.cpp
    src/ai/enemy_spawner.cpp
    src/ai/mob_spawner.cpp
    src/ai/entity_ai.cpp
    src/ai/mob_ai.cpp
    src/ai/entity_spawner.cpp
    src/animation/animation_state_machine.cpp
    src/core/weather_system.cpp
    src/ai/ai_system.cpp
    src/animation/animation.cpp
    src/entity/entity.cpp
    src/entity/components.cpp
    src/audio/audio_system.cpp
    src/audio/audio_events.cpp
    src/audio/sound_manager.cpp
    src/audio/audio_engine.cpp
    src/physics/physics_system.cpp
    src/voxel/terrain_system.cpp
    src/voxel/block.cpp
    src/voxel/block_types.cpp
    src/voxel/chunk.cpp
    src/voxel/world.cpp
    src/voxel/block_registry.cpp
    src/voxel/biome.cpp
    src/voxel/world_generator.cpp
    src/voxel/cave_generator.cpp
    src/voxel/ore_generator.cpp
    src/voxel/water_generator.cpp
    src/voxel/structure_generator.cpp
    src/voxel/flora_generator.cpp
    src/voxel/mineshaft_generator.cpp
    src/voxel/world_serializer.cpp
    src/voxel/chunk_manager.cpp
    src/physics/collision.cpp
    src/physics/rigidbody.cpp
    src/core/logger.cpp
    src/core/config.cpp
    src/core/time_system.cpp
    src/core/achievements.cpp
    src/core/raycast.cpp
    src/core/item_system.cpp
    src/core/crafting_inventory_bridge.cpp
    src/ui/ui_inventory.cpp
    src/ui/ui_element.cpp
    src/platform/platform_common.cpp
    src/platform/file_system.cpp
    src/platform/input_manager.cpp
    src/platform/gamepad.cpp
    src/core/crafting.cpp
    src/core/player_controller.cpp
    src/core/asset_manager.cpp
    src/core/asset_importer.cpp
    src/core/resource_pack.cpp
    src/core/save_system.cpp
    src/core/save_game.cpp
    src/core/survival_system.cpp
    src/voxel/voxel_editor.cpp
    src/game/player_stats.cpp
    src/game/day_night_cycle.cpp
    src/game/block_interaction.cpp
    src/game/tool_system.cpp
    src/game/mob_system.cpp
    src/game/bed_system.cpp
    src/game/furnace_system.cpp
    src/game/block_drops.cpp
    src/game/combat_system.cpp
    src/game/quest_system.cpp
    src/game/npc_dialog.cpp
    src/game/fall_damage.cpp
    src/game/lava_damage.cpp
    src/game/inventory_persistence.cpp
    src/game/scene.cpp
    src/resource/resource_manager.cpp
)

set(GLFW_SOURCES
    src/platform/window.cpp
    src/game/application.cpp
    src/network/multiplayer_game.cpp
)

set(RENDER_RUNTIME_SOURCES
    src/rendering/camera.cpp
    src/rendering/renderer.cpp
    src/rendering/shader.cpp
    src/rendering/texture.cpp
    src/rendering/lighting.cpp
    src/rendering/light_system.cpp
    src/rendering/mesh.cpp
    src/rendering/shadow_system.cpp
    src/rendering/pbr_material.cpp
    src/rendering/frustum.cpp
    src/rendering/culling_system.cpp
    src/rendering/sky/sky_renderer.cpp
    src/rendering/particles/particle_system.cpp
    src/rendering/weather/weather_effects_renderer.cpp
    src/rendering/modern_renderer.cpp
    src/rendering/post_processing.cpp
    src/voxel/block_mesh_builder.cpp
    src/voxel/world_renderer.cpp
)

set(EDITOR_RUNTIME_SOURCES
    src/debug/debug_system.cpp
    src/editor/entity_spawner.cpp
    src/editor/in_game_editor.cpp
    src/editor/scene_viewport.cpp
    src/editor/asset_browser.cpp
    src/ui/console_commands.cpp
)

set(LUA_SOURCES
    src/scripting/lua_engine.cpp
    src/scripting/script_engine.cpp
    src/scripting/lua_bindings.cpp
    src/scripting/mod_system.cpp
)

set(OPENGL_OPTIONAL_SOURCES)

set(EXPERIMENTAL_SOURCES
    src/scripting/visual_script.cpp
)

# ============================================
# Platform-Specific Sources
# ============================================
if(PLATFORM_LINUX)
    list(APPEND CORE_SOURCES src/rendering/framebuffer_renderer.cpp)
endif()

# ============================================
# Common Includes
# ============================================
set(COMMON_INCLUDES
    src
    ${CMAKE_SOURCE_DIR}/include
)

if(WIN32)
    if(EXISTS ${CMAKE_SOURCE_DIR}/third_party)
        list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party)
    endif()
    if(LUA_INCLUDE_DIR)
        list(APPEND COMMON_INCLUDES ${LUA_INCLUDE_DIR})
    endif()
else()
    list(APPEND COMMON_INCLUDES /usr/include /usr/local/include /usr/include/lua5.4)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/imgui)
    list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party/imgui)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/imgui/backends)
    list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party/imgui/backends)
endif()

set(ALL_SOURCES ${CORE_SOURCES})

if(GLFW_FOUND)
    list(APPEND ALL_SOURCES ${GLFW_SOURCES})
else()
    message(STATUS "GLFW not found; excluding windowed application sources from voxel_engine_lib")
endif()

if(GLFW_FOUND AND OpenGL_FOUND)
    list(APPEND ALL_SOURCES ${RENDER_RUNTIME_SOURCES} ${EDITOR_RUNTIME_SOURCES})
else()
    message(STATUS "Windowed render/editor runtime dependencies unavailable; excluding render runtime and editor sources from voxel_engine_lib")
endif()

if(LUA_FOUND)
    list(APPEND ALL_SOURCES ${LUA_SOURCES})
else()
    message(STATUS "Lua not found; excluding Lua-backed scripting sources from voxel_engine_lib")
endif()

if(OpenGL_FOUND)
    list(APPEND ALL_SOURCES ${OPENGL_OPTIONAL_SOURCES})
endif()

message(STATUS "Excluding experimental visual scripting sources from voxel_engine_lib until the API surface is reconciled")
message(STATUS "Excluding occlusion_culling from voxel_engine_lib until rendering dependencies are reconciled")
message(STATUS "Excluding legacy parallel Packet/UIManager implementations from voxel_engine_lib in favor of network_architecture.* and ui_system.*")

# ============================================
# Dear ImGui Sources
# ============================================
set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

set(VOXEL_ENGINE_SOURCES)

if(GLFW_FOUND)
    list(APPEND VOXEL_ENGINE_SOURCES src/main.cpp)
else()
    message(STATUS "GLFW not found; skipping voxel_engine executable")
endif()

set(IMGUI_OPTIONAL_SOURCES
    src/ui/console.cpp
    src/ui/imgui_wrapper.cpp
    src/ui/menu_system.cpp
)

set(HAVE_BUNDLED_IMGUI TRUE)
set(IMGUI_REQUIRED_FILES
    third_party/imgui/imgui.h
    third_party/imgui/imgui_internal.h
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imconfig.h
    third_party/imgui/imstb_rectpack.h
    third_party/imgui/imstb_textedit.h
    third_party/imgui/imstb_truetype.h
    third_party/imgui/backends/imgui_impl_glfw.h
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.h
    third_party/imgui/backends/imgui_impl_opengl3.cpp
    third_party/imgui/backends/imgui_impl_opengl3_loader.h
)

foreach(imgui_file ${IMGUI_REQUIRED_FILES})
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${imgui_file})
        set(HAVE_BUNDLED_IMGUI FALSE)
        break()
    endif()
endforeach()

if(HAVE_BUNDLED_IMGUI AND GLFW_FOUND)
    list(APPEND VOXEL_ENGINE_SOURCES ${IMGUI_OPTIONAL_SOURCES} ${IMGUI_SOURCES})
elseif(HAVE_BUNDLED_IMGUI)
    message(STATUS "Bundled ImGui sources found, but GLFW is unavailable; skipping optional editor/menu stack")
else()
    message(STATUS "Bundled ImGui sources not found; building voxel_engine without optional editor/menu stack")
endif()

# ============================================
# Engine Library Target
# ============================================
# Create a static library for the engine
add_library(voxel_engine_lib STATIC ${ALL_SOURCES})
target_include_directories(voxel_engine_lib PUBLIC ${COMMON_INCLUDES})
target_compile_definitions(voxel_engine_lib PUBLIC
    $<$<BOOL:${PLATFORM_WINDOWS}>:PLATFORM_WINDOWS>
    $<$<BOOL:${PLATFORM_LINUX}>:PLATFORM_LINUX>
    $<$<BOOL:${PLATFORM_MACOS}>:PLATFORM_MACOS>
)

if(OpenGL_FOUND)
    target_link_libraries(voxel_engine_lib PUBLIC OpenGL::GL)
endif()

if(GLFW_FOUND)
    target_include_directories(voxel_engine_lib PUBLIC ${GLFW_INCLUDE_DIRS})
    target_link_libraries(voxel_engine_lib PUBLIC ${GLFW_LIBRARIES})
endif()

if(LUA_FOUND)
    target_link_libraries(voxel_engine_lib PUBLIC ${LUA_LIBRARY})
endif()

if(PLATFORM_LINUX)
    target_link_libraries(voxel_engine_lib PUBLIC pthread dl GL GLX)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(voxel_engine_lib PUBLIC ws2_32 winmm opengl32)
endif()

# ============================================
# Engine Executable (for testing the engine itself)
# ============================================
if(VOXEL_ENGINE_SOURCES)
    add_executable(voxel_engine ${VOXEL_ENGINE_SOURCES})
    target_include_directories(voxel_engine PRIVATE ${COMMON_INCLUDES})
    target_link_libraries(voxel_engine PRIVATE voxel_engine_lib)
endif()

# Optional sample/test targets that the docs reference.
if(EXISTS ${CMAKE_SOURCE_DIR}/src/save_test.cpp)
    add_executable(save_test src/save_test.cpp)
    target_include_directories(save_test PRIVATE ${COMMON_INCLUDES})
    target_link_libraries(save_test PRIVATE voxel_engine_lib)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/src/world_test.cpp)
    add_executable(world_test src/world_test.cpp)
    target_include_directories(world_test PRIVATE ${COMMON_INCLUDES})
    target_link_libraries(world_test PRIVATE voxel_engine_lib)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/src/voxel_test.cpp)
    add_executable(voxel_test src/voxel_test.cpp)
    target_include_directories(voxel_test PRIVATE ${COMMON_INCLUDES})
    target_link_libraries(voxel_test PRIVATE voxel_engine_lib)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/my_game.cpp)
    add_executable(my_game my_game.cpp)
    target_include_directories(my_game PRIVATE ${COMMON_INCLUDES})
    target_link_libraries(my_game PRIVATE voxel_engine_lib)
endif()

# ============================================
# Compiler Warnings
# ============================================
if(MSVC)
    target_compile_options(voxel_engine_lib PRIVATE /W4)
    if(TARGET voxel_engine)
        target_compile_options(voxel_engine PRIVATE /W4)
    endif()
else()
    target_compile_options(voxel_engine_lib PRIVATE -Wall -Wextra)
    if(TARGET voxel_engine)
        target_compile_options(voxel_engine PRIVATE -Wall -Wextra)
    endif()
endif()

# ============================================
# Demo Game Note
# ============================================
# NOTE: Demo games have been moved to a separate repository:
#   aether-game-demo (https://github.com/Mohobie/Aether-Game-Engine)
#
# This engine repo now contains ONLY reusable engine code.
# Games that use the engine should be in their own repo.
#
# To create a game using this engine:
#   1. Create a new repo
#   2. Add this engine as a submodule or dependency
#   3. Create your own main.cpp/game files
#   4. Link against voxel_engine_lib
#
# Example CMake for a game:
#   add_subdirectory(aether-game-engine)
#   add_executable(my_game main.cpp)
#   target_link_libraries(my_game PRIVATE voxel_engine_lib)

message(STATUS "Building VoxelEngine v${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "OpenGL: ${OpenGL_FOUND}")
message(STATUS "GLFW: ${GLFW_FOUND}")
message(STATUS "Lua: ${LUA_FOUND}")
message(STATUS "Demo games: See aether-game-demo repo for examples")
