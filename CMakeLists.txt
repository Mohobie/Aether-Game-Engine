cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages (optional)
find_package(OpenGL QUIET)

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

target_include_directories(voxel_test PRIVATE 
    src
    /usr/include
    /usr/local/include
)

# Platform specific
if(UNIX AND NOT APPLE)
    target_link_libraries(voxel_test PRIVATE pthread dl)
endif()

# Main executable (only if OpenGL is available)
if(OpenGL_FOUND)
    # Add GLFW/Lua dependent files
    set(PLATFORM_SOURCES
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
    
    file(GLOB_RECURSE ALL_SOURCES src/*.cpp)
    list(FILTER ALL_SOURCES EXCLUDE REGEX "test_main.cpp$")
    
    add_executable(voxel_engine ${ALL_SOURCES})
    
    target_include_directories(voxel_engine PRIVATE 
        src
        /usr/include
        /usr/local/include
    )
    
    target_link_libraries(voxel_engine PRIVATE
        OpenGL::GL
    )
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(voxel_engine PRIVATE pthread dl)
    endif()
    
    message(STATUS "OpenGL found - main engine target available")
else()
    message(WARNING "OpenGL not found - only test target available")
    message(STATUS "Install libgl1-mesa-dev to build full engine")
endif()
