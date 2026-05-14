# Voxel Game Engine Specification

A high-performance, developer-friendly voxel game engine optimized for efficiency and modern multiplayer experiences.

---

## Core Philosophy

- **Performance First**: Every system designed with efficiency as the primary constraint
- **Developer Experience**: Simple APIs, clear documentation, rapid iteration
- **Voxel-Native**: Built from the ground up for voxel-based worlds, not adapted from polygonal engines
- **Production Ready**: Robust error handling, logging, and debugging tools

---

## 1. Architecture Overview

```
┌─────────────────────────────────────────┐
│         Game/Application Layer          │
│    (Mods, Game Logic, UI, Scripts)     │
├─────────────────────────────────────────┤
│         Gameplay Systems                │
│  (Entities, Physics, AI, Economy)       │
├─────────────────────────────────────────┤
│         Voxel Core                      │
│  (Chunk Manager, Block Registry,        │
│   Mesh Builder, World Generator)        │
├─────────────────────────────────────────┤
│         Rendering Engine                │
│  (Voxel Renderer, Shader Pipeline,     │
│   Post-Processing, UI Renderer)          │
├─────────────────────────────────────────┤
│         Multiplayer & Communication       │
│  (Netcode, Voice Chat, Text Chat,      │
│   Server Authority, Replication)          │
├─────────────────────────────────────────┤
│         Platform Abstraction            │
│  (Window, Input, Audio, File I/O,       │
│   Threading, Memory)                    │
├─────────────────────────────────────────┤
│         Foundation                      │
│  (Math, Containers, Logging, Profiling)  │
└─────────────────────────────────────────┘
```

---

## 2. Voxel Core Systems

### 2.1 Block Registry

```cpp
class BlockRegistry {
public:
    // Register a new block type
    BlockID registerBlock(const BlockDefinition& def);
    
    // Get block properties
    const BlockDefinition& getBlock(BlockID id) const;
    
    // Built-in properties
    bool isSolid(BlockID id) const;        // Can collide
    bool isOpaque(BlockID id) const;       // Blocks light
    bool isTransparent(BlockID id) const;  // Partial transparency
    bool isEmissive(BlockID id) const;     // Emits light
    bool isLiquid(BlockID id) const;       // Flows, no collision
    
    // Custom properties via components
    template<typename T>
    T* getComponent(BlockID id);
    
private:
    std::vector<BlockDefinition> blocks;
    std::unordered_map<std::string, BlockID> nameToId;
};
```

### 2.2 Chunk System

```cpp
// Chunk: 32x32x32 blocks (configurable)
constexpr int CHUNK_SIZE = 32;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

class Chunk {
public:
    // Block storage - palette-based compression
    BlockID getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockID block);
    
    // Block metadata (4 bits per block)
    uint8_t getMetadata(int x, int y, int z) const;
    void setMetadata(int x, int y, int z, uint8_t meta);
    
    // Lighting data
    uint8_t getLightLevel(int x, int y, int z) const;
    void setLightLevel(int x, int y, int z, uint8_t level);
    
    // State
    bool isEmpty() const;      // All air
    bool isUniform() const;    // All same block
    bool isModified() const;   // Needs save
    bool isDirty() const;      // Needs mesh rebuild
    
    // Memory: ~32KB per chunk with palette compression
    // vs 128KB for raw BlockID[32][32][32]
    
private:
    // Palette compression
    std::vector<BlockID> palette;  // Unique blocks in chunk
    std::vector<uint16_t> indices;   // Indices into palette
    
    // Metadata and lighting
    std::vector<uint8_t> metadata;   // 4 bits per block
    std::vector<uint8_t> light;    // 8 bits per block
};
```

### 2.3 Chunk Manager

```cpp
class ChunkManager {
public:
    // World coordinates to chunk coordinates
    static ChunkCoord worldToChunk(int x, int y, int z);
    static BlockCoord worldToBlock(int x, int y, int z);
    
    // Chunk access
    Chunk* getChunk(ChunkCoord coord);
    Chunk* getChunk(int cx, int cy, int cz);
    
    // Block access (automatic chunk loading)
    BlockID getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, BlockID block);
    
    // Chunk lifecycle
    void loadChunk(ChunkCoord coord);     // Async load from disk/network
    void unloadChunk(ChunkCoord coord);    // Save and free
    void generateChunk(ChunkCoord coord);  // Procedural generation
    
    // View distance management
    void setViewDistance(int chunks);      // Radius in chunks
    void updatePlayerPosition(Vec3 position);
    
    // Iteration
    void forEachChunkInView(std::function<void(Chunk*)> callback);
    void forEachBlockInChunk(Chunk* chunk, std::function<void(int,int,int,BlockID)> callback);
    
private:
    // Chunk storage - LRU cache with configurable limit
    LRUCache<ChunkCoord, std::unique_ptr<Chunk>> chunkCache;
    
    // Chunk loading queue - priority by distance to player
    std::priority_queue<ChunkLoadRequest> loadQueue;
    
    // Thread pool for async operations
    ThreadPool threadPool;
};
```

### 2.4 Voxel Mesh Builder

```cpp
class VoxelMeshBuilder {
public:
    // Build mesh for a chunk
    Mesh buildChunkMesh(const Chunk& chunk, const ChunkNeighbors& neighbors);
    
    // Optimization: Only build visible faces
    // Uses greedy meshing for same-block faces
    
    // Mesh data structure
    struct VoxelVertex {
        Vec3 position;      // 12 bytes
        Vec2 texCoord;      // 8 bytes
        uint32_t normal;    // Packed normal (4 bytes)
        uint32_t color;     // Packed color (4 bytes)
        uint32_t ao;        // Ambient occlusion (4 bytes)
    }; // 32 bytes per vertex
    
    // Greedy meshing - merge adjacent same-block faces
    void greedyMesh(std::vector<Quad>& quads);
    
    // Ambient occlusion calculation
    uint8_t calculateAO(const Chunk& chunk, int x, int y, int z, Face face);
    
private:
    // Face culling
    bool shouldRenderFace(BlockID block, BlockID neighbor);
    
    // Mesh caching
    std::unordered_map<ChunkCoord, Mesh> meshCache;
};
```

### 2.5 World Generation

```cpp
class WorldGenerator {
public:
    // Generate chunk at given coordinates
    void generateChunk(Chunk& chunk, ChunkCoord coord);
    
    // Biome system
    BiomeID getBiomeAt(int x, int z);
    
    // Terrain features
    float getHeightAt(int x, int z);  // Heightmap
    float getCaveDensity(int x, int y, int z);
    float getOreDensity(int x, int y, int z, OreType type);
    
    // Structure generation
    void generateStructure(StructureType type, int x, int y, int z);
    
private:
    // Noise generators
    FastNoiseSIMD terrainNoise;
    FastNoiseSIMD caveNoise;
    FastNoiseSIMD biomeNoise;
    
    // Seed-based generation
    uint64_t worldSeed;
};
```

---

## 3. Rendering Engine

### 3.1 Voxel Renderer

```cpp
class VoxelRenderer {
public:
    void initialize();
    void shutdown();
    
    // Render frame
    void beginFrame();
    void renderChunks(const std::vector<Chunk*>& chunks);
    void renderEntities(const std::vector<Entity*>& entities);
    void renderUI();
    void endFrame();
    
    // Chunk rendering
    void renderChunk(const Chunk& chunk, const Mesh& mesh);
    
    // Optimization: Frustum culling
    void setFrustum(const Frustum& frustum);
    bool isChunkVisible(ChunkCoord coord) const;
    
    // Optimization: Occlusion culling
    void buildOcclusionVolume();
    bool isOccluded(ChunkCoord coord) const;
    
    // LOD system
    void setLODLevel(int level);  // 0 = full detail, higher = less
    
private:
    // Shader programs
    Shader chunkShader;
    Shader entityShader;
    Shader uiShader;
    
    // GPU resources
    VertexArray chunkVAO;
    Buffer chunkVBO;
    Buffer chunkIBO;
    
    // Texture atlas
    Texture2D blockTextureAtlas;
    
    // Uniform buffers
    UniformBuffer viewProjUBO;
    UniformBuffer lightUBO;
};
```

### 3.2 Shader Pipeline

```glsl
// chunk.vert
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in uint aNormal;
layout(location = 3) in uint aColor;
layout(location = 4) in uint aAO;

layout(std140, binding = 0) uniform ViewProj {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
};

out vec2 vTexCoord;
out vec3 vNormal;
out vec4 vColor;
out float vAO;

void main() {
    gl_Position = projection * view * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
    vNormal = unpackNormal(aNormal);
    vColor = unpackColor(aColor);
    vAO = unpackAO(aAO);
}
```

```glsl
// chunk.frag
#version 450 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vColor;
in float vAO;

layout(binding = 0) uniform sampler2D uTextureAtlas;

layout(std140, binding = 1) uniform Lighting {
    vec3 sunDirection;
    vec3 sunColor;
    vec3 ambientColor;
    float sunIntensity;
};

out vec4 fragColor;

void main() {
    vec4 texColor = texture(uTextureAtlas, vTexCoord);
    
    // Apply vertex color (tinting)
    texColor *= vColor;
    
    // Ambient occlusion
    texColor.rgb *= vAO;
    
    // Simple lighting
    float diff = max(dot(vNormal, -sunDirection), 0.0);
    vec3 lighting = ambientColor + sunColor * diff * sunIntensity;
    
    fragColor = vec4(texColor.rgb * lighting, texColor.a);
}
```

### 3.3 Post-Processing

```cpp
class PostProcessor {
public:
    void initialize(int width, int height);
    
    // Effects
    void applyBloom(const Texture& input, Texture& output);
    void applySSAO(const Texture& depth, const Texture& normal, Texture& output);
    void applyFXAA(const Texture& input, Texture& output);
    void applyToneMapping(const Texture& input, Texture& output);
    
    // Combined pipeline
    void process(const Texture& scene, Texture& output);
    
private:
    // Framebuffers
    Framebuffer sceneFBO;
    Framebuffer bloomFBO[2];
    Framebuffer ssaoFBO;
    
    // Shaders
    Shader bloomShader;
    Shader ssaoShader;
    Shader fxaaShader;
    Shader toneMapShader;
};
```

---

## 4. Multiplayer & Communication

### 4.1 Network Architecture

```
┌─────────────┐         ┌─────────────┐         ┌─────────────┐
│   Client    │◄───────►│   Server    │◄───────►│   Client    │
│             │  UDP    │             │  UDP    │             │
│ - Predict   │         │ - Authoritative│        │ - Predict   │
│ - Interpolate│        │ - Replicate  │        │ - Interpolate│
│ - Voice Chat│         │ - Relay Voice│        │ - Voice Chat│
└─────────────┘         └─────────────┘         └─────────────┘
```

### 4.2 Network Manager

```cpp
class NetworkManager {
public:
    // Connection
    void connect(const std::string& address, uint16_t port);
    void disconnect();
    bool isConnected() const;
    
    // Server (if hosting)
    void startServer(uint16_t port, int maxPlayers);
    void stopServer();
    
    // Data transmission
    void sendReliable(const Message& msg);      // TCP-like
    void sendUnreliable(const Message& msg);    // UDP-like
    void sendOrdered(const Message& msg);     // Ordered unreliable
    
    // Entity replication
    void replicateEntity(Entity* entity);
    void setReplicationRate(Entity* entity, float hz);
    
    // Chunk synchronization
    void requestChunk(ChunkCoord coord);
    void sendChunkDelta(ChunkCoord coord, const ChunkDelta& delta);
    
    // Player state
    void sendPlayerState(const PlayerState& state);
    void receivePlayerState(PlayerID id, const PlayerState& state);
    
private:
    // Network socket
    UDPSocket socket;
    
    // Connection management
    std::vector<Connection> connections;
    Connection serverConnection;
    
    // Reliability layer
    ReliableUDP reliableLayer;
    
    // Entity interpolation
    EntityInterpolator interpolator;
    
    // Bandwidth optimization
    DeltaCompressor deltaCompressor;
};
```

### 4.3 Text Chat System

```cpp
class ChatSystem {
public:
    // Send message
    void sendMessage(const std::string& message, Channel channel);
    void sendPrivateMessage(const std::string& message, PlayerID recipient);
    
    // Receive message
    void onMessageReceived(const ChatMessage& msg);
    
    // Channels
    enum class Channel {
        Global,     // All players
        Local,      // Nearby players only
        Team,       // Team members
        Whisper,    // Private
        System      // Server announcements
    };
    
    // Message history
    std::vector<ChatMessage> getHistory(Channel channel, int count);
    
    // Moderation
    void mutePlayer(PlayerID id, Duration duration);
    void banPlayer(PlayerID id, Duration duration);
    
private:
    std::deque<ChatMessage> history;
    std::unordered_map<PlayerID, MuteInfo> mutedPlayers;
};
```

### 4.4 Voice Chat System

```cpp
class VoiceChatSystem {
public:
    // Initialize audio
    void initialize();
    void shutdown();
    
    // Voice capture
    void startRecording();
    void stopRecording();
    bool isRecording() const;
    
    // Voice playback
    void playVoice(PlayerID id, const AudioPacket& packet);
    void setPlayerVolume(PlayerID id, float volume);
    
    // Spatial audio
    void setListenerPosition(Vec3 position, Vec3 forward, Vec3 up);
    void setPlayerPosition(PlayerID id, Vec3 position);
    
    // Codec
    void setCodec(CodecType type);  // Opus recommended
    
    // Push-to-talk or voice activation
    void setPTT(bool enabled);
    void setVoiceActivation(bool enabled, float threshold);
    
private:
    // Audio I/O
    AudioDevice inputDevice;
    AudioDevice outputDevice;
    
    // Opus codec
    OpusEncoder encoder;
    std::unordered_map<PlayerID, OpusDecoder> decoders;
    
    // Spatial audio
    SpatialAudio spatialAudio;
    
    // Voice activity detection
    VAD voiceActivityDetector;
};
```

---

## 5. Input & Controls

### 5.1 Input System

```cpp
class InputSystem {
public:
    void update();
    
    // Keyboard
    bool isKeyPressed(KeyCode key);
    bool isKeyHeld(KeyCode key);
    bool isKeyReleased(KeyCode key);
    
    // Mouse
    Vec2 getMousePosition();
    Vec2 getMouseDelta();
    bool isMouseButtonPressed(MouseButton button);
    float getMouseWheelDelta();
    
    // Gamepad
    bool isGamepadConnected(int index);
    float getGamepadAxis(int index, GamepadAxis axis);
    bool isGamepadButtonPressed(int index, GamepadButton button);
    
    // Action mapping (developer-friendly)
    void bindAction(const std::string& action, const InputBinding& binding);
    bool isActionPressed(const std::string& action);
    float getActionValue(const std::string& action);
    
    // Voxel-specific actions
    bool isBreakBlockPressed();
    bool isPlaceBlockPressed();
    Vec3 getBlockTarget();  // Block player is looking at
    Vec3 getPlacePosition(); // Where to place block
    
private:
    // Raw input state
    KeyboardState keyboard;
    MouseState mouse;
    std::vector<GamepadState> gamepads;
    
    // Action mappings
    std::unordered_map<std::string, std::vector<InputBinding>> actions;
};
```

### 5.2 Action Map Example

```json
{
  "actions": {
    "move_forward": {
      "keyboard": ["W", "Up"],
      "gamepad": "LeftStickY+"
    },
    "move_backward": {
      "keyboard": ["S", "Down"],
      "gamepad": "LeftStickY-"
    },
    "move_left": {
      "keyboard": ["A", "Left"],
      "gamepad": "LeftStickX-"
    },
    "move_right": {
      "keyboard": ["D", "Right"],
      "gamepad": "LeftStickX+"
    },
    "jump": {
      "keyboard": ["Space"],
      "gamepad": "A"
    },
    "sneak": {
      "keyboard": ["LeftShift"],
      "gamepad": "RightStickPress"
    },
    "break_block": {
      "keyboard": ["MouseLeft"],
      "gamepad": "RightTrigger"
    },
    "place_block": {
      "keyboard": ["MouseRight"],
      "gamepad": "LeftTrigger"
    },
    "open_inventory": {
      "keyboard": ["E"],
      "gamepad": "Y"
    },
    "drop_item": {
      "keyboard": ["Q"],
      "gamepad": "B"
    },
    "chat": {
      "keyboard": ["T"],
      "gamepad": "DpadUp"
    },
    "push_to_talk": {
      "keyboard": ["V"],
      "gamepad": "LeftBumper"
    }
  }
}
```

---

## 6. Audio Engine

### 6.1 Audio System

```cpp
class AudioEngine {
public:
    void initialize();
    void shutdown();
    
    // Sound effects
    SoundHandle playSound(const std::string& path, Vec3 position);
    SoundHandle playSound2D(const std::string& path);
    
    // Music
    void playMusic(const std::string& path, bool loop);
    void stopMusic();
    void setMusicVolume(float volume);
    
    // Spatial audio
    void setListenerPosition(Vec3 position, Vec3 forward, Vec3 up);
    void setSourcePosition(SoundHandle handle, Vec3 position);
    void setSourceVelocity(SoundHandle handle, Vec3 velocity);
    
    // Categories
    void setCategoryVolume(AudioCategory category, float volume);
    
    enum class AudioCategory {
        Master,
        Music,
        SFX,
        UI,
        Voice,
        Ambient
    };
    
private:
    // Audio context
    AudioContext context;
    
    // Sources and buffers
    std::vector<AudioSource> sources;
    std::unordered_map<std::string, AudioBuffer> bufferCache;
    
    // Streaming
    AudioStream musicStream;
};
```

### 6.2 Voxel-Specific Audio

```cpp
class VoxelAudio {
public:
    // Block sounds
    void playBlockSound(BlockID block, SoundType type, Vec3 position);
    
    enum class SoundType {
        Break,      // Breaking block
        Place,      // Placing block
        Step,       // Walking on block
        Hit,        // Hitting block
        Ambient     // Block ambient sound
    };
    
    // Footstep system
    void updateFootsteps(Vec3 position, Vec3 velocity);
    
    // Ambient sounds
    void updateAmbientSounds(Vec3 playerPosition);
    
private:
    // Block sound mappings
    std::unordered_map<BlockID, BlockSounds> soundMap;
    
    // Footstep timing
    float footstepTimer;
    BlockID lastFootstepBlock;
};
```

---

## 7. Physics & Collision

### 7.1 Voxel Physics

```cpp
class VoxelPhysics {
public:
    // Ray casting
    RaycastResult raycast(Vec3 origin, Vec3 direction, float maxDistance);
    
    // AABB collision
    bool intersectAABB(const AABB& box, Vec3& resolution);
    
    // Entity physics
    void updateEntity(Entity* entity, float deltaTime);
    
    // Gravity and movement
    void applyGravity(Entity* entity, float deltaTime);
    void resolveCollision(Entity* entity, Vec3 velocity);
    
    // Block collision shapes
    AABB getBlockAABB(int x, int y, int z);
    AABB getBlockAABB(BlockID block, int x, int y, int z);
    
private:
    // Spatial hash for entity queries
    SpatialHash spatialHash;
    
    // Collision resolution
    void resolveAABB(const AABB& player, const AABB& block, Vec3& resolution);
};
```

### 7.2 Collision Detection

```cpp
// Optimized for voxel worlds
class VoxelCollision {
public:
    // Broad phase: Check which chunks/entities to test
    std::vector<AABB> broadPhase(const AABB& query);
    
    // Narrow phase: Precise collision tests
    bool narrowPhase(const AABB& a, const AABB& b, CollisionResult& result);
    
    // Sweep test for moving objects
    SweepResult sweepAABB(const AABB& box, Vec3 velocity, float deltaTime);
    
private:
    // Spatial partitioning
    UniformGrid uniformGrid;
};
```

---

## 8. Entity System

### 8.1 Entity Component System

```cpp
// Entity is just an ID
using EntityID = uint32_t;

// Components are pure data
struct Position { Vec3 value; };
struct Velocity { Vec3 value; };
struct Transform { Vec3 position; Quat rotation; Vec3 scale; };
struct MeshRenderer { MeshHandle mesh; MaterialHandle material; };
struct PhysicsBody { AABB bounds; float mass; bool isKinematic; };
struct PlayerController { float speed; float jumpForce; bool isGrounded; };
struct Health { int current; int max; };
struct Inventory { std::vector<ItemStack> items; };

// Systems process entities with specific components
class MovementSystem : public System {
public:
    void update(float deltaTime) override {
        for (auto [entity, pos, vel] : query<Position, Velocity>()) {
            pos.value += vel.value * deltaTime;
        }
    }
};

class PlayerControllerSystem : public System {
public:
    void update(float deltaTime) override {
        for (auto [entity, player, pos, vel] : query<PlayerController, Position, Velocity>()) {
            // Handle input
            Vec3 input = getMovementInput();
            vel.value.x = input.x * player.speed;
            vel.value.z = input.z * player.speed;
            
            // Jump
            if (isJumpPressed() && player.isGrounded) {
                vel.value.y = player.jumpForce;
                player.isGrounded = false;
            }
        }
    }
};
```

### 8.2 Entity Manager

```cpp
class EntityManager {
public:
    // Entity lifecycle
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    bool isAlive(EntityID entity) const;
    
    // Components
    template<typename T, typename... Args>
    T* addComponent(EntityID entity, Args&&... args);
    
    template<typename T>
    void removeComponent(EntityID entity);
    
    template<typename T>
    T* getComponent(EntityID entity);
    
    template<typename T>
    bool hasComponent(EntityID entity);
    
    // Queries
    template<typename... Components>
    QueryResult<Components...> query();
    
    // Systems
    void registerSystem(std::unique_ptr<System> system);
    void updateSystems(float deltaTime);
    
private:
    // Entity storage
    std::vector<Entity> entities;
    std::queue<EntityID> freeList;
    
    // Component storage - SOA for cache efficiency
    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;
    ComponentStorage<Transform> transforms;
    // ... etc
    
    // Systems
    std::vector<std::unique_ptr<System>> systems;
};
```

---

## 9. Scripting System

### 9.1 Lua Integration

```cpp
class ScriptEngine {
public:
    void initialize();
    void shutdown();
    
    // Load and run scripts
    void loadScript(const std::string& path);
    void reloadScript(const std::string& path);
    
    // Bind C++ functions to Lua
    template<typename Func>
    void bindFunction(const std::string& name, Func function);
    
    // Call Lua functions from C++
    void callFunction(const std::string& name, std::vector<Variant> args);
    
    // Entity scripting
    void attachScript(EntityID entity, const std::string& scriptPath);
    void detachScript(EntityID entity);
    
    // Event handling
    void onEvent(const std::string& eventName, std::vector<Variant> args);
    
private:
    // Lua state
    sol::state lua;
    
    // Script cache
    std::unordered_map<std::string, sol::load_result> scriptCache;
    
    // Entity scripts
    std::unordered_map<EntityID, sol::table> entityScripts;
};
```

### 9.2 Lua API Example

```lua
-- block_definitions.lua
-- Define custom blocks

local blocks = require("engine.blocks")

blocks.register({
    id = "custom:ruby_ore",
    name = "Ruby Ore",
    texture = "ruby_ore.png",
    hardness = 3.0,
    drops = "custom:ruby",
    light_level = 0,
    is_solid = true,
    is_opaque = true
})

blocks.register({
    id = "custom:glowing_stone",
    name = "Glowing Stone",
    texture = "glowing_stone.png",
    hardness = 2.0,
    light_level = 15,
    is_solid = true,
    is_opaque = true
})

-- events.lua
-- Handle game events

local events = require("engine.events")
local world = require("engine.world")

events.on("player_join", function(player)
    print("Welcome, " .. player.name .. "!")
    player:giveItem("custom:starter_kit")
end)

events.on("block_break", function(pos, block, player)
    if block.id == "custom:ruby_ore" then
        player:addXP("mining", 10)
        world.spawnParticle("sparkle", pos)
    end
end)
```

---

## 10. Memory Management

### 10.1 Allocators

```cpp
// Pool allocator for frequent allocations
class PoolAllocator {
public:
    PoolAllocator(size_t objectSize, size_t objectCount);
    void* allocate();
    void deallocate(void* ptr);
    
private:
    struct FreeNode { FreeNode* next; };
    FreeNode* freeList;
    void* memory;
};

// Arena allocator for temporary allocations
class ArenaAllocator {
public:
    ArenaAllocator(size_t size);
    void* allocate(size_t size);
    void reset();
    
private:
    std::vector<uint8_t> memory;
    size_t offset;
};

// Chunk allocator
class ChunkAllocator {
public:
    Chunk* allocateChunk();
    void deallocateChunk(Chunk* chunk);
    
private:
    PoolAllocator pool;
};
```

### 10.2 Memory Layout

```cpp
// Cache-friendly chunk storage
struct ChunkData {
    // Hot data - accessed every frame
    struct {
        BlockID palette[256];        // 512 bytes
        uint16_t blocks[CHUNK_VOLUME]; // 64KB
    } blocks;
    
    // Warm data - accessed frequently
    struct {
        uint8_t metadata[CHUNK_VOLUME / 2];  // 16KB (4 bits per block)
        uint8_t light[CHUNK_VOLUME];          // 32KB
    } data;
    
    // Cold data - accessed occasionally
    struct {
        uint8_t biome[CHUNK_SIZE * CHUNK_SIZE];  // 1KB
        uint32_t flags;                            // 4 bytes
    } meta;
}; // ~115KB total per chunk
```

---

## 11. Profiling & Debugging

### 11.1 Profiler

```cpp
class Profiler {
public:
    // Scoped profiling
    static void beginScope(const char* name);
    static void endScope();
    
    // Manual profiling
    static void recordTime(const char* name, double milliseconds);
    static void recordCounter(const char* name, uint64_t value);
    static void recordMemory(const char* name, size_t bytes);
    
    // Frame stats
    static void beginFrame();
    static void endFrame();
    
    // Output
    static void dumpToConsole();
    static void dumpToFile(const std::string& path);
    static void drawUI();  // ImGui overlay
    
private:
    struct ScopeData {
        const char* name;
        double startTime;
        double totalTime;
        uint32_t callCount;
    };
    
    std::vector<ScopeData> scopes;
    std::stack<ScopeData*> scopeStack;
};

// Usage
void updateChunks() {
    PROFILE_SCOPE("UpdateChunks");
    // ... chunk update code
}
```

### 11.2 Debug Tools

```cpp
class DebugRenderer {
public:
    // Wireframe rendering
    void drawChunkBounds(const Chunk& chunk);
    void drawEntityBounds(const Entity& entity);
    void drawRaycast(const Ray& ray, float distance);
    
    // Text overlay
    void drawText(Vec3 position, const std::string& text);
    void drawBlockInfo(int x, int y, int z, BlockID block);
    
    // Performance overlay
    void drawFPS(int fps);
    void drawFrameTime(double ms);
    void drawChunkCount(int count);
    void drawMemoryUsage(size_t bytes);
};

class Console {
public:
    // Logging
    static void log(const std::string& message, LogLevel level);
    static void trace(const std::string& msg);
    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    
    // Commands
    void registerCommand(const std::string& name, CommandFunc func);
    void execute(const std::string& command);
    
    // History
    std::vector<std::string> getHistory();
};
```

---

## 12. Build System

### 12.1 CMake Configuration

```cmake
cmake_minimum_required(VERSION 3.16)
project(VoxelEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_EDITOR "Build level editor" ON)
option(BUILD_SERVER "Build dedicated server" ON)
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_PROFILING "Enable profiling" OFF)

# Engine library
add_library(voxel_engine STATIC
    src/core/math.cpp
    src/core/memory.cpp
    src/voxel/block_registry.cpp
    src/voxel/chunk.cpp
    src/voxel/chunk_manager.cpp
    src/voxel/mesh_builder.cpp
    src/voxel/world_generator.cpp
    src/rendering/renderer.cpp
    src/rendering/shader.cpp
    src/rendering/texture.cpp
    src/rendering/post_processing.cpp
    src/network/network_manager.cpp
    src/network/replication.cpp
    src/audio/audio_engine.cpp
    src/audio/voice_chat.cpp
    src/platform/input_manager.cpp
    src/physics/voxel_physics.cpp
    src/entity/entity_manager.cpp
    src/script/script_engine.cpp
    src/platform/window.cpp
    src/platform/file_system.cpp
    src/platform/threading.cpp
)

---

## 13. Data Structures & Types

### 13.1 Core Math Types

```cpp
// 2D Vector
struct Vec2 {
    float x, y;
    
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    
    float length() const { return std::sqrt(x * x + y * y); }
    float dot(const Vec2& other) const { return x * other.x + y * other.y; }
    Vec2 normalized() const { float len = length(); return len > 0 ? Vec2(x / len, y / len) : Vec2(); }
};

// 3D Vector
struct Vec3 {
    float x, y, z;
    
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    Vec3 normalized() const { float len = length(); return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3(); }
};

// Quaternion
struct Quat {
    float x, y, z, w;
    
    Quat(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
    
    static Quat fromAxisAngle(const Vec3& axis, float angle);
    static Quat fromEuler(float pitch, float yaw, float roll);
    
    Quat operator*(const Quat& other) const;
    Vec3 operator*(const Vec3& vec) const;
    
    Quat normalized() const;
    Quat slerp(const Quat& other, float t) const;
};

// 4x4 Matrix
struct Mat4 {
    float m[16];
    
    static Mat4 identity();
    static Mat4 translate(const Vec3& translation);
    static Mat4 scale(const Vec3& scale);
    static Mat4 rotate(const Quat& rotation);
    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
    
    Mat4 operator*(const Mat4& other) const;
    Vec3 transformPoint(const Vec3& point) const;
    Vec3 transformVector(const Vec3& vector) const;
    
    Mat4 inverted() const;
    Mat4 transposed() const;
};
```

### 13.2 Bounding Volumes

```cpp
// Axis-Aligned Bounding Box
struct AABB {
    Vec3 min;
    Vec3 max;
    
    AABB() : min(Vec3()), max(Vec3()) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    
    Vec3 center() const { return (min + max) * 0.5f; }
    Vec3 size() const { return max - min; }
    float volume() const { Vec3 s = size(); return s.x * s.y * s.z; }
    
    bool contains(const Vec3& point) const;
    bool intersects(const AABB& other) const;
    AABB merge(const AABB& other) const;
    AABB expand(const Vec3& point) const;
    
    // Ray intersection
    bool intersectRay(const Vec3& origin, const Vec3& direction, float& tmin, float& tmax) const;
};

// Ray
struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalized()) {}
    
    Vec3 at(float t) const { return origin + direction * t; }
};

// Frustum (for view culling)
struct Frustum {
    // 6 planes: left, right, top, bottom, near, far
    struct Plane { Vec3 normal; float distance; };
    Plane planes[6];
    
    void update(const Mat4& viewProj);
    bool contains(const Vec3& point) const;
    bool contains(const AABB& box) const;
    bool intersects(const AABB& box) const;
};
```

### 13.3 Voxel Types

```cpp
// Block ID - 16-bit unsigned integer
using BlockID = uint16_t;
constexpr BlockID BLOCK_AIR = 0;
constexpr BlockID BLOCK_INVALID = 0xFFFF;

// Block definition
struct BlockDefinition {
    std::string name;           // "minecraft:stone"
    std::string displayName;    // "Stone"
    
    // Properties
    bool isSolid = true;
    bool isOpaque = true;
    bool isTransparent = false;
    bool isEmissive = false;
    bool isLiquid = false;
    bool isCollidable = true;
    
    // Visual
    uint32_t textureFaces[6];   // Texture atlas indices for each face
    uint32_t color = 0xFFFFFFFF; // Tint color
    float roughness = 0.8f;
    float metallic = 0.0f;
    
    // Lighting
    uint8_t lightEmission = 0;   // 0-15
    uint8_t lightOpacity = 15;   // 0-15 (how much light is blocked)
    
    // Physics
    float hardness = 1.0f;       // Mining hardness
    float friction = 0.6f;
    float restitution = 0.0f;
    
    // Audio
    std::string breakSound;
    std::string placeSound;
    std::string stepSound;
    
    // Custom properties
    std::unordered_map<std::string, Variant> properties;
};

// Chunk coordinates
struct ChunkCoord {
    int32_t x, y, z;
    
    bool operator==(const ChunkCoord& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

// Block coordinates within chunk (0-31)
struct BlockCoord {
    uint8_t x, y, z;
};

// World block coordinates
struct WorldBlockCoord {
    int32_t x, y, z;
    
    ChunkCoord toChunk() const {
        return ChunkCoord{
            x >> 5,  // Divide by 32
            y >> 5,
            z >> 5
        };
    }
    
    BlockCoord toBlock() const {
        return BlockCoord{
            static_cast<uint8_t>(x & 31),  // Modulo 32
            static_cast<uint8_t>(y & 31),
            static_cast<uint8_t>(z & 31)
        };
    }
};

// Chunk face direction
enum class Face : uint8_t {
    Left = 0,   // -X
    Right = 1,  // +X
    Bottom = 2, // -Y
    Top = 3,    // +Y
    Back = 4,   // -Z
    Front = 5   // +Z
};

// Mesh data
struct Mesh {
    std::vector<VoxelVertex> vertices;
    std::vector<uint32_t> indices;
    
    void clear() {
        vertices.clear();
        indices.clear();
    }
    
    bool isEmpty() const { return vertices.empty(); }
};
```

---

## 14. Platform Abstraction Layer

### 14.1 Window System

```cpp
class Window {
public:
    // Creation
    bool create(const std::string& title, int width, int height, bool fullscreen = false);
    void destroy();
    bool isOpen() const;
    
    // Properties
    void setTitle(const std::string& title);
    void setSize(int width, int height);
    void getSize(int& width, int& height) const;
    void setFullscreen(bool fullscreen);
    void setVSync(bool enabled);
    
    // Input focus
    void setCursorVisible(bool visible);
    void setCursorLocked(bool locked);
    bool hasFocus() const;
    
    // Events
    void pollEvents();
    bool shouldClose() const;
    
    // Platform-specific
    void* getNativeHandle() const;  // HWND, X11 Window, etc.
    void* getNativeDisplay() const; // HINSTANCE, Display*, etc.
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```

### 14.2 File System

```cpp
class FileSystem {
public:
    // Path operations
    static std::string getExecutablePath();
    static std::string getWorkingDirectory();
    static std::string getUserDataDirectory();
    static std::string getAppDataDirectory(const std::string& appName);
    
    // File operations
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool remove(const std::string& path);
    static bool copy(const std::string& source, const std::string& dest);
    
    // Reading
    static std::vector<uint8_t> readBinary(const std::string& path);
    static std::string readText(const std::string& path);
    
    // Writing
    static bool writeBinary(const std::string& path, const std::vector<uint8_t>& data);
    static bool writeText(const std::string& path, const std::string& text);
    
    // Directory listing
    static std::vector<std::string> listFiles(const std::string& directory);
    static std::vector<std::string> listFilesRecursive(const std::string& directory);
    
    // Path manipulation
    static std::string join(const std::string& a, const std::string& b);
    static std::string getDirectory(const std::string& path);
    static std::string getFilename(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string removeExtension(const std::string& path);
};
```

### 14.3 Threading

```cpp
class ThreadPool {
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();
    
    // Task submission
    template<typename Func, typename... Args>
    auto enqueue(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;
    
    // Synchronization
    void waitAll();
    size_t getPendingTasks() const;
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
};

// Spinlock for low-contention scenarios
class SpinLock {
public:
    void lock();
    void unlock();
    bool tryLock();
    
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// Read-Write Lock
class RWLock {
public:
    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();
    
private:
    std::shared_mutex mutex;
};
```

---

## 15. Resource Management

### 15.1 Asset Types

```cpp
// Texture
struct Texture {
    uint32_t id;           // OpenGL texture ID
    int width, height;
    int channels;
    bool hasAlpha;
    
    void bind(uint32_t slot = 0) const;
    void unbind() const;
};

// Material
struct Material {
    Texture* albedoMap = nullptr;
    Texture* normalMap = nullptr;
    Texture* roughnessMap = nullptr;
    Texture* metallicMap = nullptr;
    Texture* aoMap = nullptr;
    
    Vec4 albedoColor = Vec4(1, 1, 1, 1);
    float roughness = 0.5f;
    float metallic = 0.0f;
    float ao = 1.0f;
};

// Shader
struct Shader {
    uint32_t programId;
    
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    void reload();
    
    void bind() const;
    void unbind() const;
    
    // Uniforms
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const Vec2& value);
    void setVec3(const std::string& name, const Vec3& value);
    void setVec4(const std::string& name, const Vec4& value);
    void setMat4(const std::string& name, const Mat4& value);
};

// Model/Mesh
struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    
    bool loadFromFile(const std::string& path);  // .obj, .fbx, .gltf
    void render() const;
};
```

### 15.2 Resource Manager

```cpp
class ResourceManager {
public:
    // Generic resource loading
    template<typename T>
    T* load(const std::string& path);
    
    template<typename T>
    T* get(const std::string& path);
    
    template<typename T>
    void unload(const std::string& path);
    
    // Specific loaders
    Texture* loadTexture(const std::string& path);
    Shader* loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    Model* loadModel(const std::string& path);
    
    // Hot reloading
    void enableHotReloading(bool enabled);
    void checkForChanges();
    
    // Cache management
    void clearCache();
    size_t getCacheSize() const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textureCache;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaderCache;
    std::unordered_map<std::string, std::unique_ptr<Model>> modelCache;
    
    std::unordered_map<std::string, std::filesystem::file_time_type> lastModified;
};
```

### 15.3 Texture Atlas

```cpp
class TextureAtlas {
public:
    // Initialize atlas with maximum size
    void initialize(int maxWidth, int maxHeight);
    
    // Add texture to atlas
    bool addTexture(const std::string& name, const std::vector<uint8_t>& data, int width, int height);
    bool addTexture(const std::string& name, const std::string& filePath);
    
    // Get UV coordinates for a texture
    Vec2 getUV(const std::string& name) const;
    Vec2 getUVSize(const std::string& name) const;
    
    // Pack all textures into atlas
    bool pack();
    
    // Get final atlas texture
    Texture* getAtlasTexture() const;
    
    // Block texture helper (6 faces)
    void setBlockTextures(BlockID block, const std::string& faces[6]);
    void setBlockTextures(BlockID block, const std::string& allFaces); // Same texture on all faces
    
private:
    struct AtlasEntry {
        std::string name;
        int x, y, width, height;
        std::vector<uint8_t> data;
    };
    
    std::vector<AtlasEntry> entries;
    std::unique_ptr<Texture> atlasTexture;
    int atlasWidth, atlasHeight;
    bool needsRepack = false;
};
```

---

## 16. UI System

### 16.1 Immediate Mode GUI

```cpp
class UI {
public:
    void initialize(Window* window);
    void shutdown();
    
    // Frame management
    void beginFrame();
    void endFrame();
    
    // Basic widgets
    void text(const std::string& text);
    bool button(const std::string& label);
    bool checkbox(const std::string& label, bool& checked);
    void sliderFloat(const std::string& label, float& value, float min, float max);
    void sliderInt(const std::string& label, int& value, int min, int max);
    void inputText(const std::string& label, std::string& text);
    
    // Layout
    void beginWindow(const std::string& title, bool* open = nullptr);
    void endWindow();
    void sameLine();
    void newline();
    void spacing();
    void separator();
    
    // Panels
    void beginPanel(const std::string& title);
    void endPanel();
    void beginTabBar(const std::string& title);
    bool beginTab(const std::string& label);
    void endTab();
    void endTabBar();
    
    // Styling
    void pushColor(UIColor type, uint32_t color);
    void popColor();
    void pushFont(Font* font);
    void popFont();
    
    // Voxel-specific UI
    void blockSelector(const BlockRegistry& registry, BlockID& selected);
    void inventoryGrid(const Inventory& inventory, int& selectedSlot);
    void hotbar(const Inventory& inventory, int& selectedSlot);
    void crosshair();
    void debugOverlay(const DebugInfo& info);
};
```

### 16.2 HUD Elements

```cpp
class HUD {
public:
    void render(const PlayerState& player);
    
    // Health bar
    void healthBar(int current, int max);
    
    // Hotbar
    void hotbar(const Inventory& inventory, int selectedSlot);
    
    // Crosshair
    void crosshair();
    
    // Chat overlay
    void chatOverlay(const ChatSystem& chat);
    
    // Debug info
    void debugInfo(const DebugInfo& info);
    
    // Block highlight
    void blockHighlight(const Vec3& position, const BlockID& block);
};
```

---

## 17. Game Loop & Timing

### 17.1 Application

```cpp
class Application {
public:
    // Lifecycle
    bool initialize();
    void run();
    void shutdown();
    
    // Configuration
    void setTargetFPS(int fps);
    void setFixedTimestep(float timestep);  // For physics
    
    // State
    bool isRunning() const;
    float getDeltaTime() const;
    float getTotalTime() const;
    int getFPS() const;
    
    // Subsystems
    Window* getWindow();
    Renderer* getRenderer();
    InputSystem* getInput();
    AudioEngine* getAudio();
    
private:
    // Game loop
    void processEvents();
    void update(float deltaTime);
    void fixedUpdate(float fixedDeltaTime);
    void render();
    
    // Timing
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float deltaTime = 0.0f;
    float fixedTimestep = 1.0f / 60.0f;
    float accumulator = 0.0f;
    int targetFPS = 60;
    
    // FPS counting
    int frameCount = 0;
    float fpsTimer = 0.0f;
    int currentFPS = 0;
};
```

### 17.2 Timer

```cpp
class Timer {
public:
    void start();
    void stop();
    void reset();
    
    float elapsedSeconds() const;
    float elapsedMilliseconds() const;
    float elapsedMicroseconds() const;
    
    bool isRunning() const;
    
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    bool running = false;
};

// High precision sleep
void preciseSleep(float seconds);
```

---

## 18. Configuration & Settings

### 18.1 Config System

```cpp
class Config {
public:
    // Load/Save
    bool load(const std::string& path);
    bool save(const std::string& path);
    
    // Get values
    bool getBool(const std::string& key, bool defaultValue = false);
    int getInt(const std::string& key, int defaultValue = 0);
    float getFloat(const std::string& key, float defaultValue = 0.0f);
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    Vec2 getVec2(const std::string& key, const Vec2& defaultValue = Vec2());
    Vec3 getVec3(const std::string& key, const Vec3& defaultValue = Vec3());
    
    // Set values
    void setBool(const std::string& key, bool value);
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setString(const std::string& key, const std::string& value);
    void setVec2(const std::string& key, const Vec2& value);
    void setVec3(const std::string& key, const Vec3& value);
    
    // Sections
    Config* getSection(const std::string& name);
    
private:
    std::unordered_map<std::string, std::variant<bool, int, float, std::string, Vec2, Vec3>> values;
    std::unordered_map<std::string, std::unique_ptr<Config>> sections;
};
```

### 18.2 Command Line Arguments

```cpp
struct CommandLineArgs {
    // Parse arguments
    static CommandLineArgs parse(int argc, char** argv);
    
    // Access
    bool has(const std::string& name);
    std::string get(const std::string& name, const std::string& defaultValue = "");
    int getInt(const std::string& name, int defaultValue = 0);
    float getFloat(const std::string& name, float defaultValue = 0.0f);
    bool getBool(const std::string& name, bool defaultValue = false);
    
    // Common options
    bool isServerMode() const;
    bool isEditorMode() const;
    bool isHeadless() const;
    std::string getWorldPath() const;
    int getPort() const;
    
private:
    std::unordered_map<std::string, std::string> args;
    std::vector<std::string> positional;
};
```

---

## 19. Error Handling

### 19.1 Error System

```cpp
// Error codes
enum class ErrorCode {
    Success = 0,
    FileNotFound,
    InvalidFormat,
    OutOfMemory,
    GraphicsError,
    NetworkError,
    InvalidArgument,
    NotImplemented,
    Unknown
};

// Result type
template<typename T>
class Result {
public:
    bool isSuccess() const { return error == ErrorCode::Success; }
    bool isError() const { return error != ErrorCode::Success; }
    
    T& getValue() { return value; }
    const T& getValue() const { return value; }
    ErrorCode getError() const { return error; }
    const std::string& getErrorMessage() const { return message; }
    
    static Result<T> success(T value) { return Result(ErrorCode::Success, "", std::move(value)); }
    static Result<T> failure(ErrorCode code, const std::string& message) { return Result(code, message, T()); }
    
private:
    Result(ErrorCode error, const std::string& message, T value) 
        : error(error), message(message), value(std::move(value)) {}
    
    ErrorCode error;
    std::string message;
    T value;
};

// Assert system
#ifdef DEBUG
    #define VE_ASSERT(condition, message) \
        if (!(condition)) { \
            ErrorHandler::handleAssert(#condition, message, __FILE__, __LINE__); \
        }
#else
    #define VE_ASSERT(condition, message)
#endif

class ErrorHandler {
public:
    // Error handling modes
    enum class Mode {
        Log,        // Just log and continue
        Exception,  // Throw exception
        Abort,      // Abort immediately
        Callback    // Call custom handler
    };
    
    static void setMode(Mode mode);
    static void setCallback(std::function<void(ErrorCode, const std::string&)> callback);
    
    // Handle errors
    static void handleError(ErrorCode code, const std::string& message);
    static void handleAssert(const std::string& condition, const std::string& message, const char* file, int line);
    
    // Logging
    static void logError(const std::string& message);
    static void logWarning(const std::string& message);
    static void logInfo(const std::string& message);
    
private:
    static Mode currentMode;
    static std::function<void(ErrorCode, const std::string&)> errorCallback;
};
```

### 19.2 Exception Safety

```cpp
// Base exception
class VoxelEngineException : public std::exception {
public:
    VoxelEngineException(const std::string& message) : message(message) {}
    const char* what() const noexcept override { return message.c_str(); }
    
private:
    std::string message;
};

// Specific exceptions
class FileNotFoundException : public VoxelEngineException {
public:
    FileNotFoundException(const std::string& path) 
        : VoxelEngineException("File not found: " + path), path(path) {}
    
    const std::string& getPath() const { return path; }
    
private:
    std::string path;
};

class GraphicsException : public VoxelEngineException {
public:
    GraphicsException(const std::string& message) : VoxelEngineException("Graphics error: " + message) {}
};

class NetworkException : public VoxelEngineException {
public:
    NetworkException(const std::string& message) : VoxelEngineException("Network error: " + message) {}
};
```

---

## 20. Serialization

### 20.1 World Serialization

```cpp
class WorldSerializer {
public:
    // Save/Load entire world
    bool saveWorld(const std::string& path, const World& world);
    bool loadWorld(const std::string& path, World& world);
    
    // Chunk serialization
    bool saveChunk(const std::string& path, const Chunk& chunk);
    bool loadChunk(const std::string& path, Chunk& chunk);
    
    // Entity serialization
    bool saveEntity(const std::string& path, const Entity& entity);
    bool loadEntity(const std::string& path, Entity& entity);
    
    // Format
    enum class Format {
        Binary,     // Fast, compact
        JSON,       // Human-readable
        Compressed  // Binary + LZ4 compression
    };
    
    void setFormat(Format format);
    
private:
    Format currentFormat = Format::Compressed;
};
```

### 20.2 Binary Format

```cpp
// Chunk binary format (little-endian)
struct ChunkHeader {
    uint32_t magic = 0x56454B43;  // "VEKC" 
    uint32_t version = 1;
    uint32_t chunkX, chunkY, chunkZ;
    uint32_t paletteSize;
    uint32_t dataSize;
    uint32_t flags;
};

// Palette entry
struct PaletteEntry {
    uint16_t blockID;
    uint16_t metadata;
};

// Chunk data (after header and palette)
// - Block indices (uint16_t[CHUNK_VOLUME]) 
// - Metadata (uint8_t[CHUNK_VOLUME / 2])
// - Light (uint8_t[CHUNK_VOLUME])
// - Biome (uint8_t[CHUNK_SIZE * CHUNK_SIZE])
```

### 20.3 Network Messages

```cpp
// Message types
enum class MessageType : uint8_t {
    // Connection
    Handshake = 0,
    Disconnect,
    Ping,
    Pong,
    
    // World
    ChunkRequest,
    ChunkData,
    ChunkUpdate,
    BlockUpdate,
    
    // Entity
    EntityCreate,
    EntityDestroy,
    EntityUpdate,
    EntityReplication,
    
    // Player
    PlayerJoin,
    PlayerLeave,
    PlayerState,
    PlayerAction,
    
    // Chat
    ChatMessage,
    VoiceData,
    
    // Server
    ServerStatus,
    ServerConfig,
    Kick
};

// Base message
struct Message {
    MessageType type;
    uint32_t sequence;
    uint32_t timestamp;
    std::vector<uint8_t> payload;
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    bool deserialize(const std::vector<uint8_t>& data);
};

// Message handlers
class MessageHandler {
public:
    using Handler = std::function<void(const Message&)>;
    
    void registerHandler(MessageType type, Handler handler);
    void handleMessage(const Message& message);
    
private:
    std::unordered_map<MessageType, Handler> handlers;
};
```

---

## 21. Testing Framework

### 21.1 Unit Testing

```cpp
// Test framework (custom lightweight)
class TestFramework {
public:
    // Test registration
    using TestFunc = std::function<bool()>;
    
    static void registerTest(const std::string& name, TestFunc test);
    static void registerBenchmark(const std::string& name, std::function<void()> bench, int iterations);
    
    // Running
    static bool runAllTests();
    static bool runTest(const std::string& name);
    static void runAllBenchmarks();
    
    // Results
    struct TestResult {
        std::string name;
        bool passed;
        std::string errorMessage;
        float duration;
    };
    
    static std::vector<TestResult> getResults();
    static int getPassedCount();
    static int getFailedCount();
    
private:
    struct Test {
        std::string name;
        TestFunc func;
    };
    
    static std::vector<Test> tests;
    static std::vector<TestResult> results;
};

// Test macros
#define TEST(name) \
    static bool test_##name(); \
    static struct test_##name##_registrar { \
        test_##name##_registrar() { \
            TestFramework::registerTest(#name, test_##name); \
        } \
    } test_##name##_instance; \
    static bool test_##name()

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        TestFramework::fail("Assertion failed: " #condition); \
        return false; \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        TestFramework::fail("Assertion failed: " #a " == " #b); \
        return false; \
    }

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        TestFramework::fail("Assertion failed: " #a " ~= " #b); \
        return false; \
    }
```

### 21.2 Integration Testing

```cpp
class IntegrationTest {
public:
    // Setup
    virtual bool setup();
    virtual void teardown();
    
    // Test scenarios
    virtual bool testChunkLoading();
    virtual bool testWorldGeneration();
    virtual bool testNetworkConnection();
    virtual bool testEntityReplication();
    virtual bool testSerialization();
    
    // Run all
    bool runAll();
    
protected:
    std::unique_ptr<Application> app;
    std::unique_ptr<World> world;
    std::unique_ptr<NetworkManager> network;
};
```

### 21.3 Performance Benchmarks

```cpp
class Benchmark {
public:
    // Benchmark chunk operations
    static void benchmarkChunkGeneration(int numChunks);
    static void benchmarkChunkMeshing(int numChunks);
    static void benchmarkChunkSerialization(int numChunks);
    
    // Benchmark rendering
    static void benchmarkRendering(int numChunks);
    static void benchmarkCulling(int numChunks);
    
    // Benchmark networking
    static void benchmarkNetworkSerialization(int numMessages);
    static void benchmarkNetworkThroughput(int dataSize);
    
    // Results
    struct BenchmarkResult {
        std::string name;
        float avgTimeMs;
        float minTimeMs;
        float maxTimeMs;
        int iterations;
    };
    
    static std::vector<BenchmarkResult> getResults();
};
```

---

## 22. Third-Party Dependencies

### 22.1 Required Libraries

| Library | Version | Purpose | License |
|---------|---------|---------|---------|
| **GLFW** | 3.3+ | Window creation, input | zlib |
| **GLAD** | Latest | OpenGL loading | MIT |
| **glm** | 0.9.9+ | Math library (optional, can use custom) | MIT |
| **stb_image** | Latest | Image loading | Public Domain |
| **stb_image_write** | Latest | Image writing | Public Domain |
| **FastNoiseSIMD** | Latest | Noise generation | MIT |
| **sol2** | 3.3+ | Lua C++ binding | MIT |
| **Lua** | 5.4+ | Scripting | MIT |
| **enet** | 1.3+ | UDP networking | MIT |
| **opus** | 1.3+ | Voice codec | BSD |
| **OpenAL Soft** | 1.21+ | Audio | LGPL |
| **miniaudio** | 0.11+ | Audio (alternative to OpenAL) | Public Domain |
| **Dear ImGui** | 1.89+ | Immediate mode GUI | MIT |
| **nlohmann/json** | 3.11+ | JSON serialization | MIT |
| **LZ4** | 1.9+ | Compression | BSD |
| **spdlog** | 1.12+ | Logging | MIT |
| **fmt** | 10.0+ | Formatting (spdlog dependency) | MIT |

### 22.2 CMake FetchContent

```cmake
# FetchContent for dependencies
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)

# Make available
FetchContent_MakeAvailable(glfw glad glm imgui json spdlog)

# Link dependencies
target_link_libraries(voxel_engine
    glfw
    glad
    glm
    imgui
    nlohmann_json
    spdlog
)
```

---

## 23. Implementation Roadmap

### Phase 1: Foundation (Weeks 1-4)
- [ ] Math library (Vec2, Vec3, Mat4, Quat)
- [ ] Platform abstraction (Window, FileSystem, ThreadPool)
- [ ] Memory allocators (Pool, Arena, Chunk)
- [ ] Logging system (spdlog integration)
- [ ] Error handling framework

### Phase 2: Voxel Core (Weeks 5-8)
- [ ] Block registry and definitions
- [ ] Chunk data structure with palette compression
- [ ] Chunk manager with async loading
- [ ] World generator with noise
- [ ] Mesh builder with greedy meshing

### Phase 3: Rendering (Weeks 9-12)
- [ ] OpenGL renderer setup
- [ ] Shader pipeline
- [ ] Texture atlas system
- [ ] Voxel mesh rendering
- [ ] Frustum and occlusion culling
- [ ] Post-processing effects

### Phase 4: Gameplay (Weeks 13-16)
- [ ] Entity component system
- [ ] Physics and collision
- [ ] Input system with action mapping
- [ ] Lua scripting integration
- [ ] Audio engine

### Phase 5: Multiplayer (Weeks 17-20)
- [ ] Network manager (ENet)
- [ ] Entity replication
- [ ] Chunk synchronization
- [ ] Text chat system
- [ ] Voice chat (Opus)

### Phase 6: Polish (Weeks 21-24)
- [ ] UI system (Dear ImGui)
- [ ] Resource management
- [ ] Serialization (world save/load)
- [ ] Configuration system
- [ ] Profiling and debugging tools
- [ ] Testing framework

---

## 24. File Structure

```
voxel-engine/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── docs/
│   ├── architecture.md
│   ├── api-reference.md
│   └── tutorials/
├── src/
│   ├── core/
│   │   ├── math.h / .cpp
│   │   ├── memory.h / .cpp
│   │   ├── types.h
│   │   └── utils.h / .cpp
│   ├── platform/
│   │   ├── window.h / .cpp
│   │   ├── file_system.h / .cpp
│   │   ├── threading.h / .cpp
│   │   └── timer.h / .cpp
│   ├── voxel/
│   │   ├── block_registry.h / .cpp
│   │   ├── chunk.h / .cpp
│   │   ├── chunk_manager.h / .cpp
│   │   ├── mesh_builder.h / .cpp
│   │   ├── world_generator.h / .cpp
│   │   └── world.h / .cpp
│   ├── render/
│   │   ├── renderer.h / .cpp
│   │   ├── shader.h / .cpp
│   │   ├── texture.h / .cpp
│   │   ├── mesh.h / .cpp
│   │   ├── post_processor.h / .cpp
│   │   └── camera.h / .cpp
│   ├── network/
│   │   ├── network_manager.h / .cpp
│   │   ├── replication.h / .cpp
│   │   ├── message.h / .cpp
│   │   └── voice_chat.h / .cpp
│   ├── audio/
│   │   ├── audio_engine.h / .cpp
│   │   ├── spatial_audio.h / .cpp
│   │   └── voxel_audio.h / .cpp
│   ├── input/
│   │   ├── input_system.h / .cpp
│   │   ├── key_codes.h
│   │   └── gamepad.h / .cpp
│   ├── physics/
│   │   ├── voxel_physics.h / .cpp
│   │   ├── collision.h / .cpp
│   │   └── spatial_hash.h / .cpp
│   ├── entity/
│   │   ├── entity_manager.h / .cpp
│   │   ├── component_storage.h / .cpp
│   │   └── systems.h / .cpp
│   ├── script/
│   │   ├── script_engine.h / .cpp
│   │   └── lua_bindings.h / .cpp
│   ├── ui/
│   │   ├── ui.h / .cpp
│   │   ├── hud.h / .cpp
│   │   └── widgets.h / .cpp
│   ├── resource/
│   │   ├── resource_manager.h / .cpp
│   │   ├── texture_atlas.h / .cpp
│   │   └── asset_loader.h / .cpp
│   └── game/
│       ├── application.h / .cpp
│       ├── game_loop.h / .cpp
│       ├── config.h / .cpp
│       └── serialization.h / .cpp
├── include/
│   └── voxel_engine/
│       └── voxel_engine.h          # Main public header
├── tests/
│   ├── unit/
│   │   ├── test_math.cpp
│   │   ├── test_chunk.cpp
│   │   ├── test_entity.cpp
│   │   └── test_serialization.cpp
│   ├── integration/
│   │   ├── test_world.cpp
│   │   └── test_network.cpp
│   └── benchmarks/
│       ├── bench_chunk.cpp
│       └── bench_render.cpp
├── examples/
│   ├── simple_voxel_world/
│   └── multiplayer_demo/
├── assets/
│   ├── textures/
│   ├── shaders/
│   └── sounds/
├── scripts/
│   ├── block_definitions.lua
│   └── game_logic.lua
├── tools/
│   └── texture_packer/
└── third_party/
    └── CMakeLists.txt
```

---

## 25. API Usage Examples

### 25.1 Minimal Example

```cpp
#include <voxel_engine.h>

int main() {
    // Create application
    Application app;
    if (!app.initialize()) {
        return -1;
    }
    
    // Create window
    Window* window = app.getWindow();
    window->create("My Voxel Game", 1280, 720);
    
    // Create world
    World world;
    world.setSeed(12345);
    world.setViewDistance(16);
    
    // Main loop
    while (app.isRunning()) {
        // Update
        float dt = app.getDeltaTime();
        world.update(dt);
        
        // Render
        Renderer* renderer = app.getRenderer();
        renderer->beginFrame();
        renderer->renderWorld(world);
        renderer->endFrame();
        
        // Present
        window->swapBuffers();
        window->pollEvents();
    }
    
    app.shutdown();
    return 0;
}
```

### 25.2 Custom Block Registration

```cpp
#include <voxel_engine.h>

void registerCustomBlocks(BlockRegistry& registry) {
    // Register a simple block
    BlockDefinition stone;
    stone.name = "mygame:stone";
    stone.displayName = "Stone";
    stone.isSolid = true;
    stone.isOpaque = true;
    stone.hardness = 2.0f;
    // stone.textureFaces[0..5] = textureAtlas.get("stone.png");
    registry.registerBlock(stone);
    
    // Register a glowing block
    BlockDefinition lamp;
    lamp.name = "mygame:lamp";
    lamp.displayName = "Lamp";
    lamp.isSolid = true;
    lamp.isEmissive = true;
    lamp.lightEmission = 15;
    registry.registerBlock(lamp);
    
    // Register a transparent block
    BlockDefinition glass;
    glass.name = "mygame:glass";
    glass.displayName = "Glass";
    glass.isSolid = true;
    glass.isTransparent = true;
    glass.isOpaque = false;
    glass.lightOpacity = 0; // Light passes through
    registry.registerBlock(glass);
}
```

### 25.3 Multiplayer Server

```cpp
#include <voxel_engine.h>

int main(int argc, char** argv) {
    // Parse command line
    auto args = CommandLineArgs::parse(argc, argv);
    int port = args.getInt("port", 7777);
    int maxPlayers = args.getInt("max-players", 32);
    
    // Create server
    NetworkManager network;
    if (!network.startServer(port, maxPlayers)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return -1;
    }
    
    // Create world
    World world;
    world.setSeed(12345);
    
    std::cout << "Server running on port " << port << std::endl;
    
    // Server loop
    Timer timer;
    timer.start();
    
    while (true) {
        float dt = timer.elapsedSeconds();
        timer.reset();
        
        // Update network
        network.update(dt);
        
        // Update world
        world.update(dt);
        
        // Process player inputs
        for (auto& player : network.getConnectedPlayers()) {
            if (player.hasInput()) {
                auto input = player.getInput();
                world.applyPlayerInput(player.getEntity(), input);
            }
        }
        
        // Replicate entities
        network.replicateEntities(world.getEntities());
        
        // Send chunk updates
        for (auto& player : network.getConnectedPlayers()) {
            auto chunks = world.getChunksForPlayer(player.getPosition());
            for (auto& chunk : chunks) {
                if (chunk.isModified()) {
                    network.sendChunkUpdate(player, chunk);
                }
            }
        }
        
        // Sleep to maintain tick rate
        preciseSleep(0.016f); // ~60 TPS
    }
    
    network.stopServer();
    return 0;
}
```

### 25.4 Lua Modding

```lua
-- mods/my_mod/init.lua

-- Register custom blocks
local blocks = require("engine.blocks")
local events = require("engine.events")
local world = require("engine.world")

-- Register new block
blocks.register({
    id = "my_mod:ruby_ore",
    name = "Ruby Ore",
    texture = "ruby_ore.png",
    hardness = 3.0,
    drops = "my_mod:ruby",
    light_level = 0,
    is_solid = true,
    is_opaque = true
})

-- Register crafting recipe
local crafting = require("engine.crafting")
crafting.registerRecipe({
    input = {
        {"my_mod:ruby", "my_mod:ruby", "my_mod:ruby"},
        {"my_mod:ruby", "my_mod:ruby", "my_mod:ruby"},
        {"my_mod:ruby", "my_mod:ruby", "my_mod:ruby"}
    },
    output = "my_mod:ruby_block",
    count = 1
})

-- Event handlers
events.on("player_join", function(player)
    player:sendMessage("Welcome! This server is running my mod.")
end)

events.on("block_break", function(pos, block, player)
    if block.id == "my_mod:ruby_ore" then
        -- 50% chance to drop extra ruby
        if math.random() < 0.5 then
            world.spawnItem("my_mod:ruby", pos)
        end
    end
end)

-- Custom command
local commands = require("engine.commands")
commands.register("ruby", function(player, args)
    local amount = tonumber(args[1]) or 1
    player:giveItem("my_mod:ruby", amount)
    player:sendMessage("Gave you " .. amount .. " rubies!")
end)
```

---

## 26. Performance Targets

### 26.1 Rendering

| Metric | Target | Notes |
|--------|--------|-------|
| Chunk mesh generation | < 5ms | For 32x32x32 chunk |
| Frame time (60 FPS) | < 16.6ms | Full scene with 1000 chunks |
| Draw calls | < 100 | Via instancing/batching |
| GPU memory | < 2GB | For 16 chunk view distance |
| CPU memory | < 4GB | Including chunk cache |

### 26.2 Networking

| Metric | Target | Notes |
|--------|--------|-------|
| Server tick rate | 60 TPS | Fixed timestep |
| Client update rate | 20 Hz | Entity replication |
| Chunk sync | 10 chunks/sec | Priority-based |
| Latency | < 100ms | For local network |
| Bandwidth | < 1 MB/s | Per client |
| Voice latency | < 200ms | Opus compression |

### 26.3 Memory

| Component | Budget | Notes |
|-----------|--------|-------|
| Chunk storage | ~115KB | Per chunk (32³) |
| Mesh data | ~50KB | Per chunk (average) |
| Texture atlas | 256MB | 4096x4096 RGBA |
| Entity data | 1KB | Per entity (average) |
| Audio buffers | 64MB | Mixed sounds |
| Total per player | 2-4GB | With 16 chunk view |

---

## 27. Security Considerations

### 27.1 Server Authority

```cpp
class ServerAuthority {
public:
    // Validate client inputs
    bool validatePlayerMovement(PlayerID id, const Vec3& position, float deltaTime);
    bool validateBlockBreak(PlayerID id, const WorldBlockCoord& pos);
    bool validateBlockPlace(PlayerID id, const WorldBlockCoord& pos, BlockID block);
    
    // Anti-cheat
    bool checkSpeedHack(PlayerID id, const Vec3& velocity);
    bool checkFlyHack(PlayerID id, bool isGrounded);
    bool checkReachHack(PlayerID id, const Vec3& target);
    
    // Rate limiting
    bool checkBlockBreakRate(PlayerID id);
    bool checkChatRate(PlayerID id);
    bool checkCommandRate(PlayerID id);
    
private:
    std::unordered_map<PlayerID, PlayerState> playerStates;
    std::unordered_map<PlayerID, RateLimiter> rateLimiters;
};
```

### 27.2 Input Validation

```cpp
class InputValidator {
public:
    // Sanitize chat messages
    std::string sanitizeChat(const std::string& message);
    
    // Validate player names
    bool isValidPlayerName(const std::string& name);
    
    // Check for invalid packets
    bool isValidPacketSize(size_t size);
    bool isValidChunkCoord(const ChunkCoord& coord);
    bool isValidBlockID(BlockID id);
    
    // Prevent command injection in console
    std::string sanitizeCommand(const std::string& command);
};
```

---

## 28. Documentation Standards

### 28.1 Code Documentation

```cpp
/**
 * @brief Generates mesh data for a chunk
 * @param chunk The chunk to generate mesh for
 * @param neighbors Adjacent chunks (for face culling)
 * @return Mesh data containing vertices and indices
 * 
 * @details This function performs the following steps:
 * 1. Iterates through all blocks in the chunk
 * 2. Checks each of the 6 faces for visibility
 * 3. Generates vertices for visible faces
 * 4. Applies ambient occlusion
 * 5. Performs greedy meshing optimization
 * 
 * @thread_safety Not thread-safe. Should be called from main thread
 * or with chunk lock held.
 * 
 * @performance O(CHUNK_VOLUME) - linear in chunk size
 * 
 * @see VoxelMeshBuilder::greedyMesh
 * @see VoxelMeshBuilder::calculateAO
 */
Mesh buildChunkMesh(const Chunk& chunk, const ChunkNeighbors& neighbors);
```

### 28.2 API Documentation Template

```markdown
## Function: `ClassName::functionName`

**Signature:** `ReturnType functionName(ParamType param)`

**Description:** Brief description of what the function does.

**Parameters:**
- `param` - Description of parameter

**Returns:** Description of return value

**Exceptions:**
- `ExceptionType` - When/why this exception is thrown

**Example:**
```cpp
// Code example showing usage
```

**See Also:**
- [RelatedFunction](#)
- [RelatedClass](#)
```

---

## 29. Versioning & Compatibility

### 29.1 Version Format

```
MAJOR.MINOR.PATCH-BUILD

Example: 1.2.3-20240509
```

- **MAJOR**: Breaking API changes
- **MINOR**: New features, backwards compatible
- **PATCH**: Bug fixes, backwards compatible
- **BUILD**: Build date/number

### 29.2 Network Protocol Versioning

```cpp
struct ProtocolVersion {
    uint16_t major;
    uint16_t minor;
    
    bool isCompatible(const ProtocolVersion& other) const {
        return major == other.major;
    }
};

constexpr ProtocolVersion CURRENT_PROTOCOL = {1, 0};
```

### 29.3 Save Format Versioning

```cpp
struct SaveVersion {
    uint32_t version;
    std::string engineVersion;
    
    bool canLoad(uint32_t fileVersion) const {
        // Can load same version and older compatible versions
        return fileVersion <= version && fileVersion >= MIN_SUPPORTED_VERSION;
    }
};

constexpr uint32_t CURRENT_SAVE_VERSION = 1;
constexpr uint32_t MIN_SUPPORTED_VERSION = 1;
```

---

## 30. Complete Class Index

### Core
- `Vec2`, `Vec3`, `Vec4`, `Quat`, `Mat4`
- `AABB`, `Ray`, `Frustum`
- `BlockID`, `BlockDefinition`, `BlockCoord`, `ChunkCoord`, `WorldBlockCoord`
- `PoolAllocator`, `ArenaAllocator`, `ChunkAllocator`

### Platform
- `Window`, `FileSystem`, `ThreadPool`, `Timer`, `SpinLock`, `RWLock`

### Voxel
- `BlockRegistry`, `Chunk`, `ChunkManager`, `ChunkNeighbors`
- `VoxelMeshBuilder`, `WorldGenerator`, `World`
- `ChunkData`, `ChunkHeader`, `PaletteEntry`

### Rendering
- `Renderer`, `VoxelRenderer`, `PostProcessor`
- `Shader`, `Texture`, `Material`, `Model`, `Mesh`
- `VertexArray`, `Buffer`, `Framebuffer`, `UniformBuffer`
- `Camera`, `TextureAtlas`

### Network
- `NetworkManager`, `Connection`, `ServerConnection`
- `ReliableUDP`, `EntityInterpolator`, `DeltaCompressor`
- `Message`, `MessageHandler`, `MessageType`
- `ChatSystem`, `VoiceChatSystem`, `AudioPacket`

### Audio
- `AudioEngine`, `AudioContext`, `AudioSource`, `AudioBuffer`, `AudioStream`
- `VoxelAudio`, `SpatialAudio`, `VAD`
- `OpusEncoder`, `OpusDecoder`

### Input
- `InputSystem`, `KeyboardState`, `MouseState`, `GamepadState`
- `InputBinding`, `ActionMap`

### Physics
- `VoxelPhysics`, `VoxelCollision`, `SpatialHash`, `UniformGrid`
- `RaycastResult`, `CollisionResult`, `SweepResult`

### Entity
- `EntityManager`, `EntityID`, `ComponentStorage`
- `Position`, `Velocity`, `Transform`, `MeshRenderer`
- `PhysicsBody`, `PlayerController`, `Health`, `Inventory`
- `System`, `MovementSystem`, `PlayerControllerSystem`
- `QueryResult`

### Script
- `ScriptEngine`, `sol::state`

### UI
- `UI`, `HUD`, `DebugRenderer`
- `Console`, `Profiler`, `ScopeData`

### Resource
- `ResourceManager`, `AssetLoader`

### Game
- `Application`, `GameLoop`, `Config`, `CommandLineArgs`
- `WorldSerializer`, `ChunkSerializer`, `EntitySerializer`
- `TestFramework`, `IntegrationTest`, `Benchmark`

### Security
- `ServerAuthority`, `InputValidator`, `RateLimiter`

---

*End of Voxel Game Engine Specification*

---

## 31. C# Scripting System

### 31.1 Architecture

```
┌─────────────────────────────────────────┐
│         C# Scripting Layer              │
│  (User Scripts, Mods, Game Logic)       │
├─────────────────────────────────────────┤
│         C# API Bridge                   │
│  (Safe Wrappers, Type Bindings)         │
├─────────────────────────────────────────┤
│         .NET Runtime                    │
│  (Mono / .NET 8+ / CoreCLR)             │
├─────────────────────────────────────────┤
│         Native Engine                   │
│  (C++ Core via C API / P/Invoke)        │
└─────────────────────────────────────────┘
```

### 31.2 Script Types

```csharp
// Base script - attach to entities
public abstract class VoxelScript : MonoBehaviour
{
    // Lifecycle
    public virtual void OnStart() { }
    public virtual void OnUpdate(float deltaTime) { }
    public virtual void OnFixedUpdate(float fixedDeltaTime) { }
    public virtual void OnDestroy() { }
    
    // Entity access
    protected Entity Entity { get; private set; }
    protected Transform Transform { get; private set; }
    
    // Component access
    protected T GetComponent<T>() where T : Component;
    protected T AddComponent<T>() where T : Component, new();
    protected void RemoveComponent<T>() where T : Component;
    
    // World access
    protected World World => VoxelEngine.World;
    protected Player LocalPlayer => VoxelEngine.LocalPlayer;
    
    // Input
    protected Input Input => VoxelEngine.Input;
    
    // Networking
    protected bool IsServer => VoxelEngine.IsServer;
    protected bool IsClient => VoxelEngine.IsClient;
    protected NetworkManager Network => VoxelEngine.Network;
    
    // Logging
    protected void Log(string message) => VoxelEngine.Log($"[{GetType().Name}] {message}");
    protected void LogWarning(string message) => VoxelEngine.LogWarning($"[{GetType().Name}] {message}");
    protected void LogError(string message) => VoxelEngine.LogError($"[{GetType().Name}] {message}");
}

// Global script - runs without entity (singleton)
public abstract class GlobalScript
{
    public virtual void OnStart() { }
    public virtual void OnUpdate(float deltaTime) { }
    public virtual void OnFixedUpdate(float fixedDeltaTime) { }
    public virtual void OnDestroy() { }
}

// Block behavior script
public abstract class BlockScript
{
    public Block Block { get; private set; }
    public WorldBlockCoord Position { get; private set; }
    
    // Called when block is placed
    public virtual void OnPlace(Player byPlayer) { }
    
    // Called when block is broken
    public virtual void OnBreak(Player byPlayer) { }
    
    // Called when player interacts (right-click)
    public virtual void OnInteract(Player byPlayer) { }
    
    // Called every tick
    public virtual void OnTick(float deltaTime) { }
    
    // Called when neighbor block changes
    public virtual void OnNeighborChanged(WorldBlockCoord neighborPos, BlockID newBlock) { }
    
    // Called when entity collides with this block
    public virtual void OnEntityCollision(Entity entity) { }
}

// Item script
public abstract class ItemScript
{
    public Item Item { get; private set; }
    
    // Called when item is used
    public virtual void OnUse(Player byPlayer, Vec3 targetPos) { }
    
    // Called when item is equipped
    public virtual void OnEquip(Player byPlayer) { }
    
    // Called when item is unequipped
    public virtual void OnUnequip(Player byPlayer) { }
}
```

### 31.3 Engine API (C# Side)

```csharp
// Main engine access
public static class VoxelEngine
{
    // Subsystems
    public static World World { get; }
    public static Renderer Renderer { get; }
    public static Input Input { get; }
    public static Audio Audio { get; }
    public static NetworkManager Network { get; }
    public static Physics Physics { get; }
    public static UIManager UI { get; }
    
    // State
    public static bool IsRunning { get; }
    public static bool IsServer { get; }
    public static bool IsClient { get; }
    public static bool IsEditor { get; }
    public static float DeltaTime { get; }
    public static float FixedDeltaTime { get; }
    public static float TotalTime { get; }
    public static int FPS { get; }
    
    // Player
    public static Player LocalPlayer { get; }
    public static IReadOnlyList<Player> AllPlayers { get; }
    
    // Events
    public static event Action OnStart;
    public static event Action<float> OnUpdate;
    public static event Action<float> OnFixedUpdate;
    public static event Action OnShutdown;
    public static event Action<Player> OnPlayerJoin;
    public static event Action<Player> OnPlayerLeave;
    
    // Logging
    public static void Log(string message);
    public static void LogWarning(string message);
    public static void LogError(string message);
    public static void LogDebug(string message);
    
    // Script management
    public static void LoadScript(string path);
    public static void UnloadScript(string path);
    public static void ReloadScript(string path);
    public static void ReloadAllScripts();
    
    // Console commands
    public static void RegisterCommand(string name, Action<string[]> handler);
    public static void ExecuteCommand(string command);
}
```

### 31.4 Block Definition (C#)

```csharp
// Attribute-based block registration
[Block("my_mod:ruby_ore")]
public class RubyOreBlock : BlockScript
{
    // Static properties (set at registration time)
    public static string DisplayName => "Ruby Ore";
    public static float Hardness => 3.0f;
    public static int LightEmission => 0;
    public static bool IsOpaque => true;
    public static string BreakSound => "stone_break";
    public static string PlaceSound => "stone_place";
    public static string Texture => "ruby_ore.png";
    
    // Instance behavior
    public override void OnBreak(Player byPlayer)
    {
        // Drop items
        World.SpawnItem("my_mod:ruby", Position, count: 1 + Random.Range(0, 2));
        
        // XP
        byPlayer.Experience += 10;
        
        // Effects
        World.SpawnParticle("block_break", Position, color: Color.Red);
        Audio.PlaySound("stone_break", Position);
    }
    
    public override void OnPlace(Player byPlayer)
    {
        Audio.PlaySound("stone_place", Position);
    }
}
```

### 31.5 Entity Script Example

```csharp
// Attach to entities for custom behavior
public class Projectile : VoxelScript
{
    public float Speed { get; set; } = 50f;
    public float Damage { get; set; } = 10f;
    public float Lifetime { get; set; } = 5f;
    public string ImpactEffect { get; set; } = "explosion";
    
    private float _age = 0f;
    private Vec3 _velocity;
    
    public override void OnStart()
    {
        _velocity = Transform.Forward * Speed;
    }
    
    public override void OnUpdate(float deltaTime)
    {
        _age += deltaTime;
        
        if (_age >= Lifetime)
        {
            Entity.Destroy();
            return;
        }
        
        // Move
        var newPos = Transform.Position + _velocity * deltaTime;
        
        // Collision check
        var hit = Physics.Raycast(Transform.Position, _velocity.Normalized, _velocity.Length * deltaTime);
        
        if (hit.IsValid)
        {
            // Hit something
            OnImpact(hit);
            return;
        }
        
        Transform.Position = newPos;
    }
    
    private void OnImpact(RaycastHit hit)
    {
        // Spawn effect
        World.SpawnParticle(ImpactEffect, hit.Point);
        Audio.PlaySound("explosion", hit.Point);
        
        // Damage entity
        if (hit.Entity != null && hit.Entity.HasComponent<HealthComponent>())
        {
            var health = hit.Entity.GetComponent<HealthComponent>();
            health.TakeDamage(Damage);
        }
        
        // Destroy projectile
        Entity.Destroy();
    }
}
```

### 31.6 Global Game Script

```csharp
// Global script for game rules, events, etc.
public class GameRules : GlobalScript
{
    public float DayLength { get; set; } = 600f; // 10 minutes
    public bool PVP { get; set; } = true;
    public bool KeepInventory { get; set; } = false;
    public int MaxPlayers { get; set; } = 32;
    
    public override void OnStart()
    {
        // Register console commands
        VoxelEngine.RegisterCommand("time", args =>
        {
            if (args.Length > 0 && float.TryParse(args[0], out var time))
            {
                VoxelEngine.World.Time = new TimeOfDay(time);
                VoxelEngine.Log($"Time set to {time}");
            }
        });
        
        // Subscribe to events
        VoxelEngine.OnPlayerJoin += OnPlayerJoin;
        VoxelEngine.OnPlayerLeave += OnPlayerLeave;
    }
    
    private void OnPlayerJoin(Player player)
    {
        player.SendMessage("Welcome to the server!");
        player.Teleport(VoxelEngine.World.SpawnPoint);
        
        // Give starter items
        player.GiveItem("wooden_pickaxe", 1);
        player.GiveItem("wooden_axe", 1);
        player.GiveItem("torch", 16);
    }
    
    private void OnPlayerLeave(Player player)
    {
        VoxelEngine.Log($"Player {player.Username} left the game");
    }
    
    public override void OnUpdate(float deltaTime)
    {
        // Update day/night cycle
        VoxelEngine.World.Time += deltaTime / DayLength * 24f;
    }
}
```

### 31.7 Mod System

```csharp
// Mod entry point
public class MyMod : IMod
{
    public string Name => "My Awesome Mod";
    public string Version => "1.0.0";
    public string Author => "Your Name";
    public string Description => "Adds cool stuff to the game";
    
    public void OnLoad()
    {
        // Register blocks
        BlockRegistry.Register<RubyOreBlock>();
        
        // Register items
        ItemRegistry.Register("my_mod:ruby", new ItemDefinition
        {
            DisplayName = "Ruby",
            Texture = "ruby.png",
            MaxStack = 64,
            IsMaterial = true
        });
        
        // Register recipes
        Crafting.RegisterRecipe(new CraftingRecipe
        {
            Pattern = new[,]
            {
                { "my_mod:ruby", "my_mod:ruby", "my_mod:ruby" },
                { "my_mod:ruby", "stick", "my_mod:ruby" },
                { null, "stick", null }
            },
            Result = "my_mod:ruby_sword",
            Count = 1
        });
        
        VoxelEngine.Log($"[{Name}] Mod loaded successfully!");
    }
    
    public void OnUnload()
    {
        VoxelEngine.Log($"[{Name}] Mod unloaded");
    }
}
```

### 31.8 Hot Reloading

```csharp
// Scripts support hot reloading during development
public class HotReloadWatcher : GlobalScript
{
    private FileSystemWatcher _watcher;
    private float _reloadDelay = 1f;
    private float _reloadTimer = 0f;
    private bool _needsReload = false;
    
    public override void OnStart()
    {
        if (!VoxelEngine.IsEditor) return;
        
        _watcher = new FileSystemWatcher("scripts/");
        _watcher.Changed += (sender, e) =>
        {
            if (e.FullPath.EndsWith(".cs"))
            {
                _needsReload = true;
                _reloadTimer = _reloadDelay;
            }
        };
        _watcher.EnableRaisingEvents = true;
        
        VoxelEngine.Log("Hot reload watcher started");
    }
    
    public override void OnUpdate(float deltaTime)
    {
        if (!_needsReload) return;
        
        _reloadTimer -= deltaTime;
        if (_reloadTimer <= 0)
        {
            _needsReload = false;
            ReloadScripts();
        }
    }
    
    private void ReloadScripts()
    {
        try
        {
            VoxelEngine.ReloadAllScripts();
            VoxelEngine.Log("Scripts reloaded successfully");
        }
        catch (Exception ex)
        {
            VoxelEngine.LogError($"Failed to reload scripts: {ex.Message}");
        }
    }
}
```

### 31.9 C++ to C# Bridge

```cpp
// C++ side - exposes engine API to C#
class ScriptingBridge {
public:
    // Initialize .NET runtime
    bool initialize(const std::string& assemblyPath);
    void shutdown();
    
    // Load/unload assemblies
    bool loadAssembly(const std::string& path);
    void unloadAssembly(const std::string& path);
    
    // Call C# methods from C++
    template<typename... Args>
    void callMethod(const std::string& typeName, const std::string& methodName, Args... args);
    
    // Register C++ callbacks for C# to call
    void registerCallback(const std::string& name, std::function<void(void*)> callback);
    
    // Event forwarding
    void onUpdate(float deltaTime);
    void onFixedUpdate(float fixedDeltaTime);
    void onPlayerJoin(PlayerID player);
    void onPlayerLeave(PlayerID player);
    void onBlockChanged(WorldBlockCoord pos, BlockID oldBlock, BlockID newBlock);
    
private:
    // .NET runtime host
    void* runtimeHost;
    void* appDomain;
    
    // Loaded assemblies
    std::unordered_map<std::string, void*> assemblies;
    
    // Callbacks
    std::unordered_map<std::string, std::function<void(void*)>> callbacks;
};
```

### 31.10 Security Sandboxing

```csharp
// C# scripts run in sandboxed AppDomain with restrictions
public class ScriptSecurity
{
    // Allowed APIs
    public static readonly HashSet<string> AllowedNamespaces = new()
    {
        "System",
        "System.Collections",
        "System.Collections.Generic",
        "System.Linq",
        "System.Math",
        "System.Text",
        "VoxelEngine",
        "VoxelEngine.API",
        "UnityEngine" // For familiar API
    };
    
    // Forbidden APIs
    public static readonly HashSet<string> ForbiddenNamespaces = new()
    {
        "System.IO",
        "System.Net",
        "System.Reflection",
        "System.Threading",
        "System.Diagnostics",
        "System.Environment"
    };
    
    // File access (only within mod folder)
    public static bool CanAccessPath(string path)
    {
        var fullPath = Path.GetFullPath(path);
        var modPath = Path.GetFullPath("mods/");
        return fullPath.StartsWith(modPath);
    }
    
    // Network access (only through game API)
    public static bool CanMakeNetworkRequest(string url)
    {
        return false; // Scripts cannot make direct network requests
    }
}
```

---

## 32. C# Scripting Implementation Notes

### 32.1 Runtime Options

| Runtime | Pros | Cons | Recommendation |
|---------|------|------|----------------|
| **Mono** | Mature, embeddable, small footprint | Slower, older C# version | Good for compatibility |
| **.NET 8+** | Fast, modern C#, AOT compilation | Larger, newer | **Recommended** |
| **CoreCLR** | Performance, GC improvements | Complex embedding | Use via .NET 8+ |

### 32.2 Embedding .NET 8+

```cpp
// Using nethost + hostfxr
#include <nethost.h>
#include <hostfxr.h>

class DotNetRuntime {
public:
    bool initialize(const std::string& runtimeConfigPath) {
        // Load hostfxr
        hostfxr_handle cxt = nullptr;
        
        // Initialize runtime
        int rc = hostfxr_initialize_for_runtime_config(
            runtimeConfigPath.c_str(),
            nullptr,
            &cxt
        );
        
        if (rc != 0) {
            Log::error("Failed to initialize .NET runtime: {}", rc);
            return false;
        }
        
        // Get function pointers
        load_assembly = get_load_assembly_function(cxt);
        get_function_pointer = get_function_pointer_function(cxt);
        
        return true;
    }
    
    void* loadAssembly(const std::string& path) {
        // Load C# assembly
        component_entry_point_fn entryPoint = nullptr;
        int rc = load_assembly(
            path.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            (void**)&entryPoint
        );
        
        return entryPoint;
    }
    
    template<typename... Args>
    void callMethod(void* assembly, const std::string& typeName, 
                  const std::string& methodName, Args... args) {
        // Get method pointer
        void* method = nullptr;
        get_function_pointer(
            typeName.c_str(),
            methodName.c_str(),
            nullptr,
            nullptr,
            nullptr,
            &method
        );
        
        // Call method
        auto func = (void(*)(Args...))method;
        func(args...);
    }
    
private:
    load_assembly_fn load_assembly;
    get_function_pointer_fn get_function_pointer;
};
```

### 32.3 C# Project Structure for Mods

```
MyMod/
├── MyMod.csproj
├── MyMod.sln
├── src/
│   ├── MyMod.cs              # Entry point
│   ├── Blocks/
│   │   ├── RubyOreBlock.cs
│   │   └── FurnaceBlock.cs
│   ├── Items/
│   │   └── RubyItem.cs
│   ├── Entities/
│   │   └── Projectile.cs
│   └── Systems/
│       └── GameRules.cs
├── assets/
│   ├── textures/
│   │   ├── ruby_ore.png
│   │   └── ruby.png
│   ├── sounds/
│   │   └── custom_sound.ogg
│   └── models/
│       └── custom_model.obj
└── manifest.json
```

### 32.4 Mod Manifest

```json
{
  "name": "My Awesome Mod",
  "id": "my_mod",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "Adds cool stuff to the game",
  "website": "https://example.com/mymod",
  "dependencies": [
    {
      "id": "core",
      "version": ">=1.0.0"
    }
  ],
  "entryPoint": "MyMod.MyMod",
  "assets": {
    "textures": ["textures/*.png"],
    "sounds": ["sounds/*.ogg"],
    "models": ["models/*.obj"]
  },
  "scripts": {
    "global": ["GameRules"],
    "blocks": {
      "my_mod:ruby_ore": "RubyOreBlock",
      "my_mod:furnace": "FurnaceBlock"
    },
    "items": {
      "my_mod:ruby": "RubyItem"
    },
    "entities": {
      "my_mod:projectile": "Projectile"
    }
  }
}
```

---

*End of Voxel Game Engine Specification*
