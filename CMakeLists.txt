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
    src/entity/entity.cpp
    src/entity/components.cpp
    src/rendering/mesh.cpp
    src/rendering/texture.cpp
    src/rendering/shader.cpp
    src/audio/audio_system.cpp
    src/physics/physics_system.cpp
    src/voxel/block.cpp
    src/voxel/block_types.cpp
    src/voxel/chunk.cpp
    src/voxel/world.cpp
    src/voxel/block_registry.cpp
    src/voxel/biome.cpp
    src/voxel/world_generator.cpp
    src/voxel/block_mesh_builder.cpp
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
    src/platform/platform_common.cpp
    src/platform/file_system.cpp
    src/platform/input_manager.cpp
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
add_executable(voxel_test src/test_main.cpp ${CORE_SOURCES})
add_executable(world_test src/world_test.cpp ${CORE_SOURCES})
add_executable(save_test src/save_test.cpp ${CORE_SOURCES})

# ============================================
# Include Directories
# ============================================
set(COMMON_INCLUDES 
    src
    ${CMAKE_SOURCE_DIR}/include
)

if(WIN32)
    # Windows: Add third_party if it exists
    if(EXISTS ${CMAKE_SOURCE_DIR}/third_party)
        list(APPEND COMMON_INCLUDES ${CMAKE_SOURCE_DIR}/third_party)
    endif()
    if(LUA_INCLUDE_DIR)
        list(APPEND COMMON_INCLUDES ${LUA_INCLUDE_DIR})
    endif()
else()
    list(APPEND COMMON_INCLUDES /usr/include /usr/local/include)
endif()

target_include_directories(voxel_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(world_test PRIVATE ${COMMON_INCLUDES})
target_include_directories(save_test PRIVATE ${COMMON_INCLUDES})

# ============================================
# Platform-Specific Linking
# ============================================
if(PLATFORM_LINUX)
    target_link_libraries(voxel_test PRIVATE pthread dl)
    target_link_libraries(world_test PRIVATE pthread dl)
    target_link_libraries(save_test PRIVATE pthread dl)
elseif(PLATFORM_WINDOWS)
    # Windows: Link Windows-specific libraries
    target_link_libraries(voxel_test PRIVATE ws2_32 winmm)
    target_link_libraries(world_test PRIVATE ws2_32 winmm)
    target_link_libraries(save_test PRIVATE ws2_32 winmm)
endif()

# ============================================
# Full Engine Build
# ============================================
set(ALL_SOURCES ${CORE_SOURCES})

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