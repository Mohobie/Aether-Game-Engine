cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================
# Platform Detection
# ============================================
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    add_definitions(-DPLATFORM_LINUX)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    add_definitions(-DPLATFORM_MACOS)
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
    src/physics/physics_constraints.cpp
    src/network/network_architecture.cpp
    src/network/packet.cpp
    src/network/network_manager.cpp
    src/network/server_authority.cpp
    src/network/entity_replication.cpp
    src/network/networked_game_mode.cpp
    src/network/game_server.cpp
    src/network/game_client.cpp
    src/network/multiplayer_game.cpp
    src/scripting/lua_engine.cpp
    src/scripting/script_engine.cpp
    src/scripting/lua_bindings.cpp
    src/scripting/mod_system.cpp
    src/ai/behavior_tree.cpp
    src/ai/enemy_ai.cpp
    src/ai/enemy_spawner.cpp
    src/ai/mob_spawner.cpp
    src/ai/entity_ai.cpp
    src/ai/entity_spawner.cpp
    src/animation/animation_state_machine.cpp
    src/rendering/sky/sky_renderer.cpp
    src/rendering/particles/particle_system.cpp
    src/rendering/weather/weather_effects_renderer.cpp
    src/core/weather_system.cpp
    src/rendering/post_processing.cpp
    src/rendering/camera.cpp
    src/rendering/renderer.cpp
    src/rendering/shader.cpp
    src/rendering/texture.cpp
    src/rendering/lighting.cpp
    src/rendering/light_system.cpp
    src/ai/ai_system.cpp
    src/animation/animation.cpp
    src/entity/entity.cpp
    src/entity/components.cpp
    src/rendering/mesh.cpp
    src/rendering/shadow_system.cpp
    src/rendering/pbr_material.cpp
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
    src/voxel/block_mesh_builder.cpp
    src/voxel/world_renderer.cpp
    src/voxel/world_serializer.cpp
    src/voxel/ore_generator.cpp
    src/voxel/chunk_manager.cpp
    src/debug/debug_system.cpp
    src/debug/debug_renderer.cpp
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
    src/core/entity.cpp
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
    src/game/sandbox_mode.cpp
    src/game/survival_mode.cpp
    src/game/player_stats.cpp
    src/game/day_night_cycle.cpp
    src/game/block_interaction.cpp
    src/game/tool_system.cpp
    src/game/mob_system.cpp
    src/game/block_drops.cpp
    src/game/combat_system.cpp
    src/editor/entity_spawner.cpp
    src/editor/in_game_editor.cpp
    src/editor/scene_viewport.cpp
    src/editor/asset_browser.cpp
    src/ui/console_commands.cpp
    src/rendering/frustum.cpp
    src/rendering/culling_system.cpp
    src/rendering/particles/particle_system.cpp
)

# ============================================
# Platform-Specific Sources
# ============================================
if(PLATFORM_LINUX)
    list(APPEND CORE_SOURCES src/rendering/framebuffer_renderer.cpp)
endif()

# ============================================
# Test Executables
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
list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party/imgui)
list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party/imgui/backends)
endif()

add_executable(voxel_test src/test_main.cpp ${CORE_SOURCES})
add_executable(world_test src/world_test.cpp ${CORE_SOURCES})
add_executable(save_test src/save_test.cpp ${CORE_SOURCES})

target_include_directories(voxel_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(world_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(save_test PRIVATE ${COMMON_INCLUDES})

# ============================================
# Platform-Specific Linking
# ============================================
if(LUA_FOUND)
    target_link_libraries(voxel_test PRIVATE ${LUA_LIBRARY})
    target_link_libraries(world_test PRIVATE ${LUA_LIBRARY})
    target_link_libraries(save_test PRIVATE ${LUA_LIBRARY})
endif()

if(PLATFORM_LINUX)
    target_link_libraries(voxel_test PRIVATE pthread dl)
    target_link_libraries(world_test PRIVATE pthread dl)
    target_link_libraries(save_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(voxel_test PRIVATE ws2_32 winmm)
    target_link_libraries(world_test PRIVATE ws2_32 winmm)
    target_link_libraries(save_test PRIVATE ws2_32 winmm)
endif()



set(ALL_SOURCES ${CORE_SOURCES})

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

set(OPTIONAL_SOURCES
    src/platform/window.cpp
    src/rendering/camera.cpp
    src/rendering/mesh.cpp
    src/rendering/renderer.cpp
    src/rendering/shader.cpp
    src/rendering/texture.cpp
    src/rendering/lighting.cpp
    src/ui/console.cpp
    src/ui/imgui_wrapper.cpp
    src/ui/menu_system.cpp
    src/voxel/world_renderer.cpp
    src/voxel/block_mesh_builder.cpp
    src/core/engine.cpp
    src/core/application.cpp
    src/main.cpp
    ${IMGUI_SOURCES}
)

set(COMPILABLE_OPTIONAL_SOURCES)
foreach(src ${OPTIONAL_SOURCES})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${src})
        list(APPEND COMPILABLE_OPTIONAL_SOURCES ${src})
    endif()
endforeach()

add_executable(voxel_engine ${ALL_SOURCES} ${COMPILABLE_OPTIONAL_SOURCES})

target_include_directories(voxel_engine PRIVATE ${COMMON_INCLUDES})

# Link libraries
if(OpenGL_FOUND)
    target_link_libraries(voxel_engine PRIVATE OpenGL::GL)
endif()

if(GLFW_FOUND)
    target_include_directories(voxel_engine PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(voxel_engine PRIVATE ${GLFW_LIBRARIES})
endif()

if(LUA_FOUND)
    target_link_libraries(voxel_engine PRIVATE ${LUA_LIBRARY})
endif()

if(PLATFORM_LINUX)
    target_link_libraries(voxel_engine PRIVATE pthread dl GL GLX)
    target_link_libraries(voxel_test PRIVATE pthread dl GL GLX)
    target_link_libraries(world_test PRIVATE pthread dl GL GLX)
    target_link_libraries(save_test PRIVATE pthread dl GL GLX)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(voxel_engine PRIVATE ws2_32 winmm opengl32)
    target_link_libraries(voxel_test PRIVATE ws2_32 winmm opengl32)
    target_link_libraries(world_test PRIVATE ws2_32 winmm opengl32)
    target_link_libraries(save_test PRIVATE ws2_32 winmm opengl32)
endif()

# ============================================
# Compiler Warnings
# ============================================
if(MSVC)
    target_compile_options(voxel_engine PRIVATE /W4)
    target_compile_options(voxel_test PRIVATE /W4)
else()
    target_compile_options(voxel_engine PRIVATE -Wall -Wextra)
    target_compile_options(voxel_test PRIVATE -Wall -Wextra)
endif()

# ============================================
# My Game Executable
# ============================================
if(EXISTS ${CMAKE_SOURCE_DIR}/my_game.cpp)
    # Build without main.cpp and engine.cpp (they have their own main)
    set(MY_GAME_SOURCES ${ALL_SOURCES})
    list(REMOVE_ITEM MY_GAME_SOURCES src/main.cpp src/core/engine.cpp src/core/application.cpp)
    
    # Also remove from optional sources
    set(MY_GAME_OPTIONAL ${COMPILABLE_OPTIONAL_SOURCES})
    list(REMOVE_ITEM MY_GAME_OPTIONAL src/main.cpp src/core/engine.cpp src/core/application.cpp)
    
    add_executable(my_game my_game.cpp ${MY_GAME_SOURCES} ${MY_GAME_OPTIONAL})
    target_include_directories(my_game PRIVATE ${COMMON_INCLUDES})
    
    if(OpenGL_FOUND)
        target_link_libraries(my_game PRIVATE OpenGL::GL)
    endif()
    if(GLFW_FOUND)
        target_include_directories(my_game PRIVATE ${GLFW_INCLUDE_DIRS})
        target_link_libraries(my_game PRIVATE ${GLFW_LIBRARIES})
    endif()
    if(LUA_FOUND)
        target_link_libraries(my_game PRIVATE ${LUA_LIBRARY})
    endif()
    if(PLATFORM_LINUX)
        target_link_libraries(my_game PRIVATE pthread dl GL GLU GLX)
    elseif(PLATFORM_WINDOWS)
        target_link_libraries(my_game PRIVATE ws2_32 winmm opengl32 glu32)
    endif()
    
    message(STATUS "My Game: my_game.cpp found — building my_game executable")
else()
    message(STATUS "My Game: my_game.cpp not found — skipping")
endif()

# ============================================
# Multiplayer Executable
# ============================================
if(EXISTS ${CMAKE_SOURCE_DIR}/multiplayer_main.cpp)
    set(MP_SOURCES ${ALL_SOURCES})
    list(REMOVE_ITEM MP_SOURCES src/main.cpp src/core/engine.cpp src/core/application.cpp)
    
    set(MP_OPTIONAL ${COMPILABLE_OPTIONAL_SOURCES})
    list(REMOVE_ITEM MP_OPTIONAL src/main.cpp src/core/engine.cpp src/core/application.cpp)
    
    add_executable(multiplayer multiplayer_main.cpp ${MP_SOURCES} ${MP_OPTIONAL})
    target_include_directories(multiplayer PRIVATE ${COMMON_INCLUDES})
    
    if(OpenGL_FOUND)
        target_link_libraries(multiplayer PRIVATE OpenGL::GL)
    endif()
    if(GLFW_FOUND)
        target_include_directories(multiplayer PRIVATE ${GLFW_INCLUDE_DIRS})
        target_link_libraries(multiplayer PRIVATE ${GLFW_LIBRARIES})
    endif()
    if(LUA_FOUND)
        target_link_libraries(multiplayer PRIVATE ${LUA_LIBRARY})
    endif()
    if(PLATFORM_LINUX)
        target_link_libraries(multiplayer PRIVATE pthread dl GL GLU GLX)
    elseif(PLATFORM_WINDOWS)
        target_link_libraries(multiplayer PRIVATE ws2_32 winmm opengl32 glu32)
    endif()
    
    message(STATUS "Multiplayer: multiplayer_main.cpp found — building multiplayer executable")
else()
    message(STATUS "Multiplayer: multiplayer_main.cpp not found — skipping")
endif()

message(STATUS "Building VoxelEngine v${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "OpenGL: ${OpenGL_FOUND}")
message(STATUS "GLFW: ${GLFW_FOUND}")
message(STATUS "Lua: ${LUA_FOUND}")
