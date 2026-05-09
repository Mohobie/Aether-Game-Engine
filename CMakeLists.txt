cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages (optional)
find_package(OpenGL QUIET)
find_package(PkgConfig QUIET)

# Try to find GLFW
set(GLFW_FOUND FALSE)
if(PkgConfig_FOUND)
    pkg_check_modules(GLFW QUIET glfw3)
endif()

# Try to find Lua
set(LUA_FOUND FALSE)
find_library(LUA_LIBRARY lua5.4 lua54 lua5.3 lua53 lua PATHS /usr/lib /usr/local/lib)
if(LUA_LIBRARY)
    set(LUA_FOUND TRUE)
    message(STATUS "Found Lua library: ${LUA_LIBRARY}")
endif()

# Core files that don't need OpenGL/GLFW/Lua
set(CORE_SOURCES
    src/math/vec3.cpp
    src/math/mat4.cpp
    src/voxel/block.cpp
    src/voxel/chunk.cpp
    src/voxel/world.cpp
    src/voxel/block_registry.cpp
    src/voxel/biome.cpp
    src/voxel/world_generator.cpp
    src/physics/collision.cpp
    src/physics/rigidbody.cpp
    src/core/logger.cpp
    src/core/save_system.cpp
    src/core/config.cpp
    src/core/time_system.cpp
    src/core/achievements.cpp
    src/core/raycast.cpp
    src/core/inventory.cpp
    src/core/crafting.cpp
    src/core/entity.cpp
    src/network/packet.cpp
    src/audio/audio_engine.cpp
    src/audio/sound_manager.cpp
)

# Test executable (no OpenGL/GLFW required)
add_executable(voxel_test src/test_main.cpp ${CORE_SOURCES})

# World generation test
add_executable(world_test src/world_test.cpp ${CORE_SOURCES})

# Save/load test
add_executable(save_test src/save_test.cpp ${CORE_SOURCES})

target_include_directories(voxel_test PRIVATE 
    src
    /usr/include
    /usr/local/include
)

target_include_directories(world_test PRIVATE 
    src
    /usr/include
    /usr/local/include
)

# Platform specific
if(UNIX AND NOT APPLE)
    target_link_libraries(voxel_test PRIVATE pthread dl)
    target_link_libraries(world_test PRIVATE pthread dl)
    target_link_libraries(save_test PRIVATE pthread dl)
endif()

target_include_directories(save_test PRIVATE 
    src
    /usr/include
    /usr/local/include
)

# Build as many files as possible even without all dependencies
set(ALL_SOURCES ${CORE_SOURCES})

# Add files that might compile without OpenGL/GLFW headers
# (we'll stub out the OpenGL calls)
set(OPTIONAL_SOURCES
    src/platform/input.cpp
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

# Check which optional files can compile
set(COMPILABLE_OPTIONAL_SOURCES)
foreach(src ${OPTIONAL_SOURCES})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${src})
        list(APPEND COMPILABLE_OPTIONAL_SOURCES ${src})
    endif()
endforeach()

# Try to build full engine even without OpenGL/GLFW
# (we'll use stub implementations)
add_executable(voxel_engine ${ALL_SOURCES} ${COMPILABLE_OPTIONAL_SOURCES})

target_include_directories(voxel_engine PRIVATE 
    src
    /usr/include
    /usr/local/include
)

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

if(UNIX AND NOT APPLE)
    target_link_libraries(voxel_engine PRIVATE pthread dl)
endif()

message(STATUS "Building with ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
