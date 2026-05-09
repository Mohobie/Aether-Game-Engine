# Aether Voxel Engine

A C++ voxel game engine with modern rendering, physics, networking, and scripting.

## Features

- **Voxel World**: Chunk-based 16³ block storage with procedural generation
- **Rendering**: OpenGL-based with shaders, textures, and camera system
- **Physics**: AABB collision detection and rigid body dynamics
- **Networking**: UDP client/server architecture (stub)
- **Scripting**: Lua integration ready (headers)
- **UI**: Dear ImGui wrapper for in-game interfaces
- **Audio**: Audio engine stub for sound playback
- **Platform**: GLFW window and input handling

## Building

```bash
# Clone the repository
git clone http://192.168.1.189:3100/aether/aether-game-engine.git
cd aether-game-engine

# Build with CMake
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./voxel_engine
```

## Project Structure

```
src/
├── core/          # Engine core (application, logger)
├── math/          # Math library (vec3, mat4)
├── voxel/         # Voxel systems (chunk, world, block)
├── rendering/     # Graphics (renderer, shader, camera, mesh, texture)
├── platform/      # Platform abstraction (window, input)
├── physics/       # Physics (collision, rigidbody)
├── network/       # Networking (client, network_manager)
├── audio/         # Audio (audio_engine, sound_manager)
├── scripting/     # Scripting (lua_engine, script_engine)
└── ui/            # UI (console, imgui_wrapper)
```

## Current Status

- ✅ 24 .cpp implementation files
- ✅ 30 header files
- ✅ CMake build system
- ✅ Git repository with Gitea integration
- 🔄 Active development

## License

MIT License - Aether AI
