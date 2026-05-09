cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)

# Fetch dependencies
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)

FetchContent_MakeAvailable(glfw glm)

# glad is special - we need to generate it
# For now, assume glad is in third_party/

# Source files
file(GLOB_RECURSE SOURCES 
    src/*.cpp
)

# Create executable
add_executable(voxel_engine ${SOURCES})

target_include_directories(voxel_engine PRIVATE 
    src
    ${glad_SOURCE_DIR}/include
)

target_link_libraries(voxel_engine PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)

# Platform specific
if(WIN32)
    target_link_libraries(voxel_engine PRIVATE opengl32)
endif()

# Copy shaders
file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
