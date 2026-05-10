#pragma once
#include "math/vec3.h"
#include "math/vec2.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace vge {

// Forward declarations
class Chunk;
class World;
class Camera;

// ============================================
// Terrain LOD Level
// ============================================
enum class LODLevel {
    LOD0 = 0,   // Full detail (16x16 blocks)
    LOD1 = 1,   // Half detail (8x8 blocks)
    LOD2 = 2,   // Quarter detail (4x4 blocks)
    LOD3 = 3,   // Eighth detail (2x2 blocks)
    LOD4 = 4    // Single block
};

// ============================================
// Terrain Patch
// ============================================
struct TerrainPatch {
    int chunkX, chunkZ;
    LODLevel lod;
    float distanceFromCamera;
    bool visible;
    bool needsUpdate;
    
    // Mesh data for this LOD
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    
    TerrainPatch() : chunkX(0), chunkZ(0), lod(LODLevel::LOD0), 
                     distanceFromCamera(0), visible(false), needsUpdate(true) {}
};

// ============================================
// Heightmap
// ============================================
class Heightmap {
private:
    int width, height;
    std::vector<float> heights;
    float minHeight, maxHeight;
    
    // Noise parameters
    int octaves;
    float persistence;
    float lacunarity;
    float scale;
    int seed;
    
public:
    Heightmap(int width = 256, int height = 256);
    ~Heightmap();
    
    // Generation
    void Generate(int seed = 0);
    void GenerateFromNoise(int seed, float scale, int octaves, 
                          float persistence, float lacunarity);
    void LoadFromImage(const std::string& filename);
    void LoadFromData(const std::vector<float>& data);
    
    // Sampling
    float GetHeight(int x, int z) const;
    float GetHeightBilinear(float x, float z) const;
    float GetHeightNearest(float x, float z) const;
    
    // Normal calculation
    Vec3 GetNormal(int x, int z) const;
    Vec3 GetNormal(float x, float z) const;
    
    // Modification
    void SetHeight(int x, int z, float height);
    void AddHeight(int x, int z, float delta);
    void Smooth(float strength = 0.5f);
    void Erode(float strength = 0.1f, int iterations = 10);
    
    // Utility
    float GetMinHeight() const { return minHeight; }
    float GetMaxHeight() const { return maxHeight; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    // Export
    void SaveToImage(const std::string& filename) const;
    std::vector<float> GetData() const { return heights; }
};

// ============================================
// Terrain Generator
// ============================================
enum class TerrainType {
    Flat,
    Hills,
    Mountains,
    Islands,
    Canyons,
    Custom
};

class TerrainGenerator {
private:
    TerrainType type;
    int seed;
    
    // Noise parameters
    float baseScale;
    float heightScale;
    int octaves;
    float persistence;
    float lacunarity;
    
    // Feature parameters
    float waterLevel;
    float snowLevel;
    float treeLine;
    
    // Biome blending
    float biomeBlendDistance;
    
public:
    TerrainGenerator(TerrainType type = TerrainType::Hills, int seed = 0);
    ~TerrainGenerator();
    
    // Configuration
    void SetType(TerrainType type) { this->type = type; }
    void SetSeed(int seed) { this->seed = seed; }
    void SetNoiseParams(float scale, float height, int octaves, 
                       float persistence, float lacunarity);
    void SetWaterLevel(float level) { waterLevel = level; }
    void SetSnowLevel(float level) { snowLevel = level; }
    
    // Generation
    void GenerateChunk(Chunk* chunk, int chunkX, int chunkZ);
    void GenerateHeightmap(Heightmap& heightmap, int offsetX, int offsetZ);
    
    // Feature generation
    void GenerateRivers(Heightmap& heightmap, int count = 5);
    void GenerateCaves(Chunk* chunk);
    void GenerateOres(Chunk* chunk);
    
    // Biome determination
    int GetBiomeAt(float height, float moisture, float temperature) const;
    
    // Height at position
    float GetHeightAt(float worldX, float worldZ) const;
    
private:
    float SampleNoise(float x, float z) const;
    float SampleFractalNoise(float x, float z) const;
    float GetTerrainHeight(float x, float z) const;
};

// ============================================
// Terrain Streaming
// ============================================
struct StreamedChunk {
    int chunkX, chunkZ;
    float priority;
    bool loading;
    bool loaded;
    bool unloading;
    
    StreamedChunk() : chunkX(0), chunkZ(0), priority(0), 
                      loading(false), loaded(false), unloading(false) {}
};

class TerrainStreamer {
private:
    World* world;
    
    // Streaming parameters
    float loadDistance;
    float unloadDistance;
    float lodDistances[5]; // Distance thresholds for each LOD
    
    // Chunk management
    std::unordered_map<uint64_t, StreamedChunk> streamedChunks;
    std::vector<StreamedChunk*> loadQueue;
    std::vector<StreamedChunk*> unloadQueue;
    
    // Async loading
    int maxConcurrentLoads;
    int currentLoads;
    
    // Camera
    Vec3 cameraPosition;
    Vec3 cameraDirection;
    
public:
    TerrainStreamer(World* world);
    ~TerrainStreamer();
    
    // Configuration
    void SetLoadDistance(float distance) { loadDistance = distance; }
    void SetUnloadDistance(float distance) { unloadDistance = distance; }
    void SetLODDistances(float lod0, float lod1, float lod2, float lod3, float lod4);
    void SetMaxConcurrentLoads(int max) { maxConcurrentLoads = max; }
    
    // Update
    void Update(const Vec3& cameraPos, const Vec3& cameraDir);
    void UpdateLODs(const Vec3& cameraPos);
    
    // Chunk management
    void RequestChunk(int chunkX, int chunkZ);
    void ReleaseChunk(int chunkX, int chunkZ);
    bool IsChunkLoaded(int chunkX, int chunkZ) const;
    bool IsChunkLoading(int chunkX, int chunkZ) const;
    
    // Priority
    float CalculateChunkPriority(int chunkX, int chunkZ) const;
    void SortLoadQueue();
    
    // Processing
    void ProcessLoadQueue(int maxPerFrame = 1);
    void ProcessUnloadQueue(int maxPerFrame = 2);
    
    // Stats
    int GetLoadedChunkCount() const;
    int GetLoadingChunkCount() const;
    int GetQueueSize() const { return loadQueue.size(); }
    
private:
    uint64_t GetChunkKey(int x, int z) const;
    void UpdateChunkLOD(StreamedChunk& chunk, float distance);
};

// ============================================
// Terrain LOD Manager
// ============================================
class TerrainLODManager {
private:
    // LOD distances
    float lodDistances[5];
    
    // Transition settings
    float transitionWidth;
    bool useMorphing;
    
    // Chunk LOD cache
    std::unordered_map<uint64_t, LODLevel> chunkLODs;
    
public:
    TerrainLODManager();
    ~TerrainLODManager();
    
    // Configuration
    void SetLODDistance(LODLevel lod, float distance);
    void SetTransitionWidth(float width) { transitionWidth = width; }
    void EnableMorphing(bool enable) { useMorphing = enable; }
    
    // LOD calculation
    LODLevel CalculateLOD(float distance) const;
    float GetLODFactor(float distance, LODLevel lod) const;
    bool ShouldTransition(float distance, LODLevel currentLOD) const;
    
    // Chunk LOD
    void UpdateChunkLOD(int chunkX, int chunkZ, float distance);
    LODLevel GetChunkLOD(int chunkX, int chunkZ) const;
    
    // Morphing
    float CalculateMorphFactor(float distance, LODLevel lod) const;
};

// ============================================
// Terrain Mesh Builder
// ============================================
class TerrainMeshBuilder {
public:
    // Build mesh for a chunk at specific LOD
    static void BuildChunkMesh(Chunk* chunk, LODLevel lod,
                               std::vector<float>& vertices,
                               std::vector<uint32_t>& indices);
    
    // Build mesh from heightmap
    static void BuildHeightmapMesh(const Heightmap& heightmap,
                                   float worldScale,
                                   std::vector<float>& vertices,
                                   std::vector<uint32_t>& indices);
    
    // LOD mesh generation
    static void BuildLODMesh(const Heightmap& heightmap, int chunkX, int chunkZ,
                             LODLevel lod, std::vector<float>& vertices,
                             std::vector<uint32_t>& indices);
    
    // Stitching between LOD levels
    static void StitchLODEdges(std::vector<float>& vertices,
                               std::vector<uint32_t>& indices,
                               LODLevel centerLOD, LODLevel neighborLOD,
                               int edge); // 0=top, 1=right, 2=bottom, 3=left
    
private:
    static int GetVertexIndex(int x, int z, int resolution);
    static float GetHeightAtVertex(const Heightmap& heightmap, int x, int z,
                                   int chunkX, int chunkZ, int resolution);
};

// ============================================
// Terrain Decoration
// ============================================
struct TerrainDecoration {
    std::string type;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    float probability;
    int minHeight;
    int maxHeight;
    std::vector<std::string> biomes;
};

class TerrainDecorator {
private:
    std::vector<TerrainDecoration> decorations;
    int seed;
    
public:
    TerrainDecorator(int seed = 0);
    ~TerrainDecorator();
    
    // Decoration types
    void AddTree(const std::string& type, float probability, 
                 int minHeight, int maxHeight);
    void AddRock(const std::string& type, float probability,
                 int minHeight, int maxHeight);
    void AddGrass(float probability, int minHeight, int maxHeight);
    void AddFlower(const std::string& type, float probability,
                   int minHeight, int maxHeight);
    void AddBush(const std::string& type, float probability,
                 int minHeight, int maxHeight);
    
    // Decoration
    void DecorateChunk(Chunk* chunk, const Heightmap& heightmap,
                       int chunkX, int chunkZ);
    void DecorateTerrain(World* world, const Heightmap& heightmap,
                         int startX, int startZ, int width, int height);
    
    // Placement
    bool CanPlaceDecoration(const Vec3& position, const std::string& type) const;
    Vec3 FindValidPosition(const Heightmap& heightmap, int chunkX, int chunkZ) const;
    
private:
    float RandomFloat() const;
    int RandomInt(int min, int max) const;
};

// ============================================
// Terrain System (Main Controller)
// ============================================
class TerrainSystem {
private:
    World* world;
    std::unique_ptr<TerrainGenerator> generator;
    std::unique_ptr<TerrainStreamer> streamer;
    std::unique_ptr<TerrainLODManager> lodManager;
    std::unique_ptr<TerrainDecorator> decorator;
    
    // Global heightmap for far terrain
    std::unique_ptr<Heightmap> globalHeightmap;
    float globalHeightmapScale;
    
    // Settings
    bool useStreaming;
    bool useLOD;
    bool useDecoration;
    bool useHeightmap;
    
    // Stats
    int totalChunksGenerated;
    int totalDecorationsPlaced;
    
public:
    TerrainSystem(World* world);
    ~TerrainSystem();
    
    // Initialization
    void Initialize(TerrainType type = TerrainType::Hills, int seed = 0);
    void SetHeightmap(const std::string& filename);
    void SetHeightmap(std::unique_ptr<Heightmap> heightmap);
    
    // Configuration
    void EnableStreaming(bool enable) { useStreaming = enable; }
    void EnableLOD(bool enable) { useLOD = enable; }
    void EnableDecoration(bool enable) { useDecoration = enable; }
    
    // Update
    void Update(const Vec3& cameraPos, const Vec3& cameraDir, float deltaTime);
    
    // Generation
    void GenerateChunk(int chunkX, int chunkZ);
    void GenerateRegion(int startX, int startZ, int width, int height);
    void RegenerateChunk(int chunkX, int chunkZ);
    
    // Height queries
    float GetHeightAt(float worldX, float worldZ) const;
    Vec3 GetNormalAt(float worldX, float worldZ) const;
    
    // Decoration
    void DecorateChunk(int chunkX, int chunkZ);
    void DecorateRegion(int startX, int startZ, int width, int height);
    
    // Streaming
    bool IsChunkLoaded(int chunkX, int chunkZ) const;
    void ForceLoadChunk(int chunkX, int chunkZ);
    void ForceUnloadChunk(int chunkX, int chunkZ);
    
    // Utility
    void SaveTerrain(const std::string& filename) const;
    void LoadTerrain(const std::string& filename);
    void ClearTerrain();
    
    // Stats
    int GetTotalChunksGenerated() const { return totalChunksGenerated; }
    int GetTotalDecorationsPlaced() const { return totalDecorationsPlaced; }
    int GetLoadedChunkCount() const;
    
    // Access
    TerrainGenerator* GetGenerator() const { return generator.get(); }
    TerrainStreamer* GetStreamer() const { return streamer.get(); }
    TerrainLODManager* GetLODManager() const { return lodManager.get(); }
    TerrainDecorator* GetDecorator() const { return decorator.get(); }
    Heightmap* GetGlobalHeightmap() const { return globalHeightmap.get(); }
};

} // namespace vge