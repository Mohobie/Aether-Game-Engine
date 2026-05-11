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
    # Windows: Check common install locations
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
    # Linux/macOS: Try pkg-config first
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW QUIET glfw3)
    endif()
    
    # Fallback to find_library
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
    src/ai/behavior_tree.cpp
    src/animation/animation_state_machine.cpp
    src/rendering/sky/day_night_cycle.cpp
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
    src/ai/ai_system.cpp
    src/animation/animation.cpp
    src/entity/entity.cpp
    src/entity/components.cpp
    src/rendering/mesh.cpp
    src/rendering/texture.cpp
    src/rendering/shader.cpp
    src/rendering/shadow_system.cpp
    src/rendering/pbr_material.cpp
    src/audio/audio_system.cpp
    src/physics/physics_system.cpp
    src/voxel/terrain_system.cpp
    src/voxel/block.cpp
    src/voxel/block_types.cpp
    src/voxel/chunk.cpp
    src/voxel/world.cpp
    src/voxel/block_registry.cpp
    src/voxel/biome.cpp
    src/voxel/world_generator.cpp
    src/voxel/block_mesh_builder.cpp
    src/voxel/world_renderer.cpp
    src/debug/debug_system.cpp
    src/physics/collision.cpp
    src/physics/rigidbody.cpp
    src/core/logger.cpp
    src/core/save_system.cpp
    src/core/config.cpp
    src/core/time_system.cpp
    src/core/achievements.cpp
    src/core/raycast.cpp
    src/core/item_system.cpp
    src/core/crafting_inventory_bridge.cpp
    src/ui/ui_inventory.cpp
    src/ui/ui_element.cpp
    src/core/entity.cpp
    src/network/packet.cpp
    src/audio/audio_engine.cpp
    src/audio/sound_manager.cpp
    src/platform/platform_common.cpp
    src/platform/file_system.cpp
    src/platform/input_manager.cpp
    src/platform/gamepad.cpp
    src/core/crafting.cpp
    src/core/player_controller.cpp
    src/voxel/voxel_editor.cpp
    src/game/sandbox_mode.cpp
    src/editor/entity_spawner.cpp
    src/editor/in_game_editor.cpp
    src/ui/console_commands.cpp
    src/rendering/frustum.cpp
    src/rendering/culling_system.cpp
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
    list(APPEND COMMON_INCLUDES /usr/include /usr/local/include)
endif()

add_executable(voxel_test src/test_main.cpp ${CORE_SOURCES})
add_executable(world_test src/world_test.cpp ${CORE_SOURCES})
add_executable(save_test src/save_test.cpp ${CORE_SOURCES})
add_executable(weather_test tests/test_weather_system.cpp ${CORE_SOURCES})
add_executable(day_night_test tests/test_day_night.cpp ${CORE_SOURCES})
add_executable(inventory_test tests/test_inventory_system.cpp ${CORE_SOURCES})

target_include_directories(voxel_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(world_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(save_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(weather_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(day_night_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(inventory_test PRIVATE ${COMMON_INCLUDES})

# ============================================
# Platform-Specific Linking
# ============================================
if(PLATFORM_LINUX)
    target_link_libraries(voxel_test PRIVATE pthread dl)
    target_link_libraries(world_test PRIVATE pthread dl)
    target_link_libraries(save_test PRIVATE pthread dl)
    target_link_libraries(day_night_test PRIVATE pthread dl)
    target_link_libraries(inventory_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    # Windows: Link Windows-specific libraries
    target_link_libraries(voxel_test PRIVATE ws2_32 winmm)
    target_link_libraries(world_test PRIVATE ws2_32 winmm)
    target_link_libraries(save_test PRIVATE ws2_32 winmm)
    target_link_libraries(day_night_test PRIVATE ws2_32 winmm)
    target_link_libraries(inventory_test PRIVATE ws2_32 winmm)
endif()

add_executable(editor_test tests/test_editor.cpp ${CORE_SOURCES})
target_include_directories(editor_test PRIVATE ${COMMON_INCLUDES})
if(PLATFORM_LINUX)
    target_link_libraries(editor_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(editor_test PRIVATE ws2_32 winmm)
endif()

add_executable(console_test tests/test_console_commands.cpp ${CORE_SOURCES})
target_include_directories(console_test PRIVATE ${COMMON_INCLUDES})
if(PLATFORM_LINUX)
    target_link_libraries(console_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(console_test PRIVATE ws2_32 winmm)
endif()

add_executable(culling_test tests/test_culling.cpp ${CORE_SOURCES})
target_include_directories(culling_test PRIVATE ${COMMON_INCLUDES})
if(PLATFORM_LINUX)
    target_link_libraries(culling_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(culling_test PRIVATE ws2_32 winmm)
endif()

add_executable(gamepad_test tests/test_gamepad_standalone.cpp src/platform/gamepad.cpp)
target_include_directories(gamepad_test PRIVATE ${COMMON_INCLUDES})

add_executable(renderer_integration_test tests/test_renderer_integration.cpp ${CORE_SOURCES})
target_include_directories(renderer_integration_test PRIVATE ${COMMON_INCLUDES})
if(PLATFORM_LINUX)
    target_link_libraries(renderer_integration_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(renderer_integration_test PRIVATE ws2_32 winmm)
endif()

set(ALL_SOURCES ${CORE_SOURCES})

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
    src/voxel/chunk_manager.cpp
    src/core/player_controller.cpp
    src/core/engine.cpp
    src/core/application.cpp
    src/scripting/lua_engine.cpp
    src/scripting/script_engine.cpp
    src/main.cpp
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
    target_link_libraries(voxel_engine PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    target_link_libraries(voxel_engine PRIVATE ws2_32 winmm)
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

message(STATUS "Building VoxelEngine v${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "OpenGL: ${OpenGL_FOUND}")
message(STATUS "GLFW: ${GLFW_FOUND}")
message(STATUS "Lua: ${LUA_FOUND}")