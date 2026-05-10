#include "terrain_system.h"
#include "world.h"
#include "chunk.h"
#include "block.h"
#include "block_types.h"
#include "block_registry.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>

namespace vge {

// ============================================
// Heightmap Implementation
// ============================================

Heightmap::Heightmap(int width, int height)
    : width(width), height(height), minHeight(0), maxHeight(0),
      octaves(4), persistence(0.5f), lacunarity(2.0f), scale(0.01f), seed(0) {
    heights.resize(width * height, 0.0f);
}

Heightmap::~Heightmap() {}

void Heightmap::Generate(int seed) {
    this->seed = seed;
    GenerateFromNoise(seed, scale, octaves, persistence, lacunarity);
}

void Heightmap::GenerateFromNoise(int seed, float scale, int octaves,
                                  float persistence, float lacunarity) {
    this->seed = seed;
    this->scale = scale;
    this->octaves = octaves;
    this->persistence = persistence;
    this->lacunarity = lacunarity;
    
    minHeight = std::numeric_limits<float>::max();
    maxHeight = std::numeric_limits<float>::lowest();
    
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float amplitude = 1.0f;
            float frequency = scale;
            float noiseHeight = 0.0f;
            
            for (int i = 0; i < octaves; ++i) {
                float sampleX = x * frequency;
                float sampleZ = z * frequency;
                float perlinValue = std::sin(sampleX + seed) * std::cos(sampleZ + seed * 0.5f);
                noiseHeight += perlinValue * amplitude;
                amplitude *= persistence;
                frequency *= lacunarity;
            }
            
            heights[z * width + x] = noiseHeight;
            if (noiseHeight < minHeight) minHeight = noiseHeight;
            if (noiseHeight > maxHeight) maxHeight = noiseHeight;
        }
    }
}

void Heightmap::LoadFromImage(const std::string& filename) {
    (void)filename;
}

void Heightmap::LoadFromData(const std::vector<float>& data) {
    if (data.size() == static_cast<size_t>(width * height)) {
        heights = data;
        minHeight = std::numeric_limits<float>::max();
        maxHeight = std::numeric_limits<float>::lowest();
        for (float h : heights) {
            if (h < minHeight) minHeight = h;
            if (h > maxHeight) maxHeight = h;
        }
    }
}

float Heightmap::GetHeight(int x, int z) const {
    x = std::max(0, std::min(width - 1, x));
    z = std::max(0, std::min(height - 1, z));
    return heights[z * width + x];
}

float Heightmap::GetHeightBilinear(float x, float z) const {
    int x0 = static_cast<int>(std::floor(x));
    int z0 = static_cast<int>(std::floor(z));
    float fx = x - x0;
    float fz = z - z0;
    float h00 = GetHeight(x0, z0);
    float h10 = GetHeight(x0 + 1, z0);
    float h01 = GetHeight(x0, z0 + 1);
    float h11 = GetHeight(x0 + 1, z0 + 1);
    return h00 * (1 - fx) * (1 - fz) + h10 * fx * (1 - fz) + h01 * (1 - fx) * fz + h11 * fx * fz;
}

float Heightmap::GetHeightNearest(float x, float z) const {
    return GetHeight(static_cast<int>(std::round(x)), static_cast<int>(std::round(z)));
}

Vec3 Heightmap::GetNormal(int x, int z) const {
    float hL = GetHeight(x - 1, z);
    float hR = GetHeight(x + 1, z);
    float hD = GetHeight(x, z - 1);
    float hU = GetHeight(x, z + 1);
    Vec3 normal(hL - hR, 2.0f, hD - hU);
    return normal.normalize();
}

Vec3 Heightmap::GetNormal(float x, float z) const {
    return GetNormal(static_cast<int>(std::round(x)), static_cast<int>(std::round(z)));
}

void Heightmap::SetHeight(int x, int z, float height) {
    if (x >= 0 && x < width && z >= 0 && z < height) {
        heights[z * width + x] = height;
        if (height < minHeight) minHeight = height;
        if (height > maxHeight) maxHeight = height;
    }
}

void Heightmap::AddHeight(int x, int z, float delta) {
    SetHeight(x, z, GetHeight(x, z) + delta);
}

void Heightmap::Smooth(float strength) {
    std::vector<float> smoothed = heights;
    for (int z = 1; z < height - 1; ++z) {
        for (int x = 1; x < width - 1; ++x) {
            float sum = 0.0f;
            for (int dz = -1; dz <= 1; ++dz) {
                for (int dx = -1; dx <= 1; ++dx) {
                    sum += GetHeight(x + dx, z + dz);
                }
            }
            smoothed[z * width + x] = GetHeight(x, z) * (1 - strength) + (sum / 9.0f) * strength;
        }
    }
    heights = smoothed;
}

void Heightmap::Erode(float strength, int iterations) {
    (void)strength;
    (void)iterations;
}

void Heightmap::SaveToImage(const std::string& filename) const {
    (void)filename;
}

// ============================================
// Terrain Generator Implementation
// ============================================

TerrainGenerator::TerrainGenerator(TerrainType type, int seed)
    : type(type), seed(seed), baseScale(0.01f), heightScale(64.0f),
      octaves(4), persistence(0.5f), lacunarity(2.0f),
      waterLevel(0.3f), snowLevel(0.8f), treeLine(0.6f), biomeBlendDistance(32.0f) {}

TerrainGenerator::~TerrainGenerator() {}

void TerrainGenerator::SetNoiseParams(float scale, float height, int octaves,
                                       float persistence, float lacunarity) {
    baseScale = scale;
    heightScale = height;
    this->octaves = octaves;
    this->persistence = persistence;
    this->lacunarity = lacunarity;
}

float TerrainGenerator::SampleNoise(float x, float z) const {
    return std::sin(x + seed) * std::cos(z + seed * 0.5f);
}

float TerrainGenerator::SampleFractalNoise(float x, float z) const {
    float amplitude = 1.0f;
    float frequency = baseScale;
    float noiseHeight = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        noiseHeight += std::sin(x * frequency + seed) * std::cos(z * frequency + seed * 0.5f) * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return noiseHeight;
}

float TerrainGenerator::GetTerrainHeight(float x, float z) const {
    switch (type) {
        case TerrainType::Flat: return 0.5f;
        case TerrainType::Hills: return SampleFractalNoise(x, z) * 0.5f + 0.5f;
        case TerrainType::Mountains: {
            float base = SampleFractalNoise(x, z);
            float detail = SampleFractalNoise(x * 2, z * 2) * 0.5f;
            return std::pow(base, 2.0f) * 0.7f + detail * 0.3f + 0.3f;
        }
        case TerrainType::Islands: {
            float noise = SampleFractalNoise(x, z);
            float distance = std::sqrt(x * x + z * z) * 0.001f;
            float falloff = std::max(0.0f, 1.0f - distance);
            return noise * falloff * 0.5f + 0.3f;
        }
        case TerrainType::Canyons: {
            float noise1 = SampleFractalNoise(x, z);
            float noise2 = SampleFractalNoise(x + 1000, z + 1000);
            return std::abs(noise1 - noise2) * 0.5f + 0.3f;
        }
        case TerrainType::Custom: return SampleFractalNoise(x, z);
        default: return 0.5f;
    }
}

void TerrainGenerator::GenerateChunk(Chunk* chunk, int chunkX, int chunkZ) {
    if (!chunk) return;
    
    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            float worldX = static_cast<float>(chunkX * CHUNK_SIZE + x);
            float worldZ = static_cast<float>(chunkZ * CHUNK_SIZE + z);
            float height = GetTerrainHeight(worldX, worldZ);
            int groundHeight = static_cast<int>(height * heightScale);
            
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                int worldY = chunk->GetChunkY() * CHUNK_SIZE + y;
                if (worldY < groundHeight - 3) {
                    chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("stone"));
                } else if (worldY < groundHeight) {
                    chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("dirt"));
                } else if (worldY == groundHeight) {
                    float moisture = std::sin(worldX * 0.005f + seed) * std::cos(worldZ * 0.005f + seed);
                    float temperature = std::sin(worldX * 0.003f + seed + 1000) * std::cos(worldZ * 0.003f + seed);
                    int biome = GetBiomeAt(height, moisture, temperature);
                    switch (biome) {
                        case 0: chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("sand")); break;
                        case 1: chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("snow")); break;
                        default: chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("grass")); break;
                    }
                } else if (worldY < waterLevel * heightScale) {
                    chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("water"));
                } else {
                    chunk->SetBlock(x, y, z, BlockRegistry::GetInstance().GetBlockId("air"));
                }
            }
        }
    }
    chunk->SetDirty(true);
}

void TerrainGenerator::GenerateHeightmap(Heightmap& heightmap, int offsetX, int offsetZ) {
    for (int z = 0; z < heightmap.GetHeight(); ++z) {
        for (int x = 0; x < heightmap.GetWidth(); ++x) {
            float worldX = static_cast<float>(offsetX + x);
            float worldZ = static_cast<float>(offsetZ + z);
            heightmap.SetHeight(x, z, GetTerrainHeight(worldX, worldZ));
        }
    }
}

void TerrainGenerator::GenerateRivers(Heightmap& heightmap, int count) {
    (void)heightmap;
    (void)count;
}

void TerrainGenerator::GenerateCaves(Chunk* chunk) {
    (void)chunk;
}

void TerrainGenerator::GenerateOres(Chunk* chunk) {
    (void)chunk;
}

int TerrainGenerator::GetBiomeAt(float height, float moisture, float temperature) const {
    (void)height;
    if (temperature > 0.7f && moisture < 0.3f) return 0;
    else if (temperature < 0.2f) return 1;
    else if (moisture > 0.6f && temperature > 0.5f) return 2;
    else if (moisture > 0.4f) return 3;
    else return 4;
}

float TerrainGenerator::GetHeightAt(float worldX, float worldZ) const {
    return GetTerrainHeight(worldX, worldZ) * heightScale;
}

// ============================================
// Terrain Streamer Implementation
// ============================================

TerrainStreamer::TerrainStreamer(World* world)
    : world(world), loadDistance(256.0f), unloadDistance(384.0f),
      maxConcurrentLoads(4), currentLoads(0) {
    lodDistances[0] = 64.0f;
    lodDistances[1] = 128.0f;
    lodDistances[2] = 256.0f;
    lodDistances[3] = 512.0f;
    lodDistances[4] = 1024.0f;
}

TerrainStreamer::~TerrainStreamer() {}

void TerrainStreamer::SetLODDistances(float lod0, float lod1, float lod2, float lod3, float lod4) {
    lodDistances[0] = lod0;
    lodDistances[1] = lod1;
    lodDistances[2] = lod2;
    lodDistances[3] = lod3;
    lodDistances[4] = lod4;
}

void TerrainStreamer::Update(const Vec3& cameraPos, const Vec3& cameraDir) {
    cameraPosition = cameraPos;
    cameraDirection = cameraDir;
    
    int cameraChunkX = static_cast<int>(cameraPos.x) / CHUNK_SIZE;
    int cameraChunkZ = static_cast<int>(cameraPos.z) / CHUNK_SIZE;
    int loadRadius = static_cast<int>(loadDistance / CHUNK_SIZE);
    
    for (int dz = -loadRadius; dz <= loadRadius; ++dz) {
        for (int dx = -loadRadius; dx <= loadRadius; ++dx) {
            int chunkX = cameraChunkX + dx;
            int chunkZ = cameraChunkZ + dz;
            float distance = std::sqrt(dx * dx + dz * dz) * CHUNK_SIZE;
            
            if (distance <= loadDistance) {
                if (!IsChunkLoaded(chunkX, chunkZ) && !IsChunkLoading(chunkX, chunkZ)) {
                    RequestChunk(chunkX, chunkZ);
                }
            }
            if (distance <= unloadDistance) {
                auto it = streamedChunks.find(GetChunkKey(chunkX, chunkZ));
                if (it != streamedChunks.end()) {
                    UpdateChunkLOD(it->second, distance);
                }
            }
        }
    }
    
    for (auto& [key, chunk] : streamedChunks) {
        if (chunk.loaded && !chunk.unloading) {
            float dx = chunk.chunkX - cameraChunkX;
            float dz = chunk.chunkZ - cameraChunkZ;
            float distance = std::sqrt(dx * dx + dz * dz) * CHUNK_SIZE;
            if (distance > unloadDistance) {
                ReleaseChunk(chunk.chunkX, chunk.chunkZ);
            }
        }
    }
    
    SortLoadQueue();
    ProcessLoadQueue(2);
    ProcessUnloadQueue(4);
}

void TerrainStreamer::UpdateLODs(const Vec3& cameraPos) {
    int cameraChunkX = static_cast<int>(cameraPos.x) / CHUNK_SIZE;
    int cameraChunkZ = static_cast<int>(cameraPos.z) / CHUNK_SIZE;
    
    for (auto& [key, chunk] : streamedChunks) {
        if (chunk.loaded) {
            float dx = chunk.chunkX - cameraChunkX;
            float dz = chunk.chunkZ - cameraChunkZ;
            float distance = std::sqrt(dx * dx + dz * dz) * CHUNK_SIZE;
            UpdateChunkLOD(chunk, distance);
        }
    }
}

void TerrainStreamer::RequestChunk(int chunkX, int chunkZ) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    auto it = streamedChunks.find(key);
    if (it == streamedChunks.end()) {
        StreamedChunk chunk;
        chunk.chunkX = chunkX;
        chunk.chunkZ = chunkZ;
        chunk.priority = CalculateChunkPriority(chunkX, chunkZ);
        chunk.loading = false;
        chunk.loaded = false;
        chunk.unloading = false;
        streamedChunks[key] = chunk;
    }
    
    StreamedChunk& chunk = streamedChunks[key];
    if (!chunk.loaded && !chunk.loading) {
        chunk.loading = true;
        loadQueue.push_back(&chunk);
    }
}

void TerrainStreamer::ReleaseChunk(int chunkX, int chunkZ) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    auto it = streamedChunks.find(key);
    if (it != streamedChunks.end() && it->second.loaded && !it->second.unloading) {
        it->second.unloading = true;
        unloadQueue.push_back(&it->second);
    }
}

bool TerrainStreamer::IsChunkLoaded(int chunkX, int chunkZ) const {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    auto it = streamedChunks.find(key);
    return it != streamedChunks.end() && it->second.loaded;
}

bool TerrainStreamer::IsChunkLoading(int chunkX, int chunkZ) const {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    auto it = streamedChunks.find(key);
    return it != streamedChunks.end() && it->second.loading;
}

float TerrainStreamer::CalculateChunkPriority(int chunkX, int chunkZ) const {
    int cameraChunkX = static_cast<int>(cameraPosition.x) / CHUNK_SIZE;
    int cameraChunkZ = static_cast<int>(cameraPosition.z) / CHUNK_SIZE;
    float dx = chunkX - cameraChunkX;
    float dz = chunkZ - cameraChunkZ;
    float distance = std::sqrt(dx * dx + dz * dz);
    float priority = 1.0f / (distance + 1.0f);
    
    Vec3 chunkDir(dx * CHUNK_SIZE, 0, dz * CHUNK_SIZE);
    chunkDir.normalize();
    float viewAlignment = chunkDir.dot(cameraDirection);
    if (viewAlignment > 0.5f) priority *= 1.5f;
    
    return priority;
}

void TerrainStreamer::SortLoadQueue() {
    std::sort(loadQueue.begin(), loadQueue.end(),
              [](StreamedChunk* a, StreamedChunk* b) { return a->priority > b->priority; });
}

void TerrainStreamer::ProcessLoadQueue(int maxPerFrame) {
    int processed = 0;
    while (!loadQueue.empty() && processed < maxPerFrame && currentLoads < maxConcurrentLoads) {
        StreamedChunk* chunk = loadQueue.front();
        loadQueue.erase(loadQueue.begin());
        
        if (chunk->loading && !chunk->loaded) {
            chunk->loaded = true;
            chunk->loading = false;
            ++currentLoads;
        }
        ++processed;
    }
}

void TerrainStreamer::ProcessUnloadQueue(int maxPerFrame) {
    int processed = 0;
    while (!unloadQueue.empty() && processed < maxPerFrame) {
        StreamedChunk* chunk = unloadQueue.front();
        unloadQueue.erase(unloadQueue.begin());
        
        if (chunk->unloading && chunk->loaded) {
            chunk->loaded = false;
            chunk->unloading = false;
            --currentLoads;
        }
        ++processed;
    }
}

int TerrainStreamer::GetLoadedChunkCount() const {
    int count = 0;
    for (const auto& [key, chunk] : streamedChunks) {
        if (chunk.loaded) ++count;
    }
    return count;
}

int TerrainStreamer::GetLoadingChunkCount() const {
    int count = 0;
    for (const auto& [key, chunk] : streamedChunks) {
        if (chunk.loading) ++count;
    }
    return count;
}

uint64_t TerrainStreamer::GetChunkKey(int x, int z) const {
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(z);
}

void TerrainStreamer::UpdateChunkLOD(StreamedChunk& chunk, float distance) {
    if (distance < lodDistances[0]) chunk.priority = 1.0f;
    else if (distance < lodDistances[1]) chunk.priority = 0.8f;
    else if (distance < lodDistances[2]) chunk.priority = 0.6f;
    else if (distance < lodDistances[3]) chunk.priority = 0.4f;
    else chunk.priority = 0.2f;
}

// ============================================
// Terrain LOD Manager Implementation
// ============================================

TerrainLODManager::TerrainLODManager()
    : transitionWidth(16.0f), useMorphing(true) {
    lodDistances[0] = 64.0f;
    lodDistances[1] = 128.0f;
    lodDistances[2] = 256.0f;
    lodDistances[3] = 512.0f;
    lodDistances[4] = 1024.0f;
}

TerrainLODManager::~TerrainLODManager() {}

void TerrainLODManager::SetLODDistance(LODLevel lod, float distance) {
    lodDistances[static_cast<int>(lod)] = distance;
}

LODLevel TerrainLODManager::CalculateLOD(float distance) const {
    for (int i = 4; i >= 0; --i) {
        if (distance >= lodDistances[i]) return static_cast<LODLevel>(i);
    }
    return LODLevel::LOD0;
}

float TerrainLODManager::GetLODFactor(float distance, LODLevel lod) const {
    float lodDistance = lodDistances[static_cast<int>(lod)];
    float nextDistance = (static_cast<int>(lod) < 4) ? lodDistances[static_cast<int>(lod) + 1] : lodDistance * 2;
    if (distance < lodDistance) return 1.0f;
    if (distance > nextDistance) return 0.0f;
    return 1.0f - (distance - lodDistance) / (nextDistance - lodDistance);
}

bool TerrainLODManager::ShouldTransition(float distance, LODLevel currentLOD) const {
    float lodDistance = lodDistances[static_cast<int>(currentLOD)];
    return std::abs(distance - lodDistance) < transitionWidth;
}

void TerrainLODManager::UpdateChunkLOD(int chunkX, int chunkZ, float distance) {
    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(chunkX)) << 32) | static_cast<uint32_t>(chunkZ);
    chunkLODs[key] = CalculateLOD(distance);
}

LODLevel TerrainLODManager::GetChunkLOD(int chunkX, int chunkZ) const {
    uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(chunkX)) << 32) | static_cast<uint32_t>(chunkZ);
    auto it = chunkLODs.find(key);
    if (it != chunkLODs.end()) return it->second;
    return LODLevel::LOD0;
}

float TerrainLODManager::CalculateMorphFactor(float distance, LODLevel lod) const {
    if (!useMorphing) return 1.0f;
    float lodDistance = lodDistances[static_cast<int>(lod)];
    float morphStart = lodDistance - transitionWidth * 0.5f;
    float morphEnd = lodDistance + transitionWidth * 0.5f;
    if (distance < morphStart) return 1.0f;
    if (distance > morphEnd) return 0.0f;
    return 1.0f - (distance - morphStart) / (morphEnd - morphStart);
}

// ============================================
// Terrain Mesh Builder Implementation
// ============================================

void TerrainMeshBuilder::BuildChunkMesh(Chunk* chunk, LODLevel lod,
                                        std::vector<float>& vertices,
                                        std::vector<uint32_t>& indices) {
    (void)chunk;
    (void)lod;
    (void)vertices;
    (void)indices;
}

void TerrainMeshBuilder::BuildHeightmapMesh(const Heightmap& heightmap,
                                            float worldScale,
                                            std::vector<float>& vertices,
                                            std::vector<uint32_t>& indices) {
    int width = heightmap.GetWidth();
    int height = heightmap.GetHeight();
    vertices.clear();
    indices.clear();
    
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float h = heightmap.GetHeight(x, z);
            vertices.push_back(x * worldScale);
            vertices.push_back(h * worldScale);
            vertices.push_back(z * worldScale);
            
            Vec3 normal = heightmap.GetNormal(x, z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            vertices.push_back(static_cast<float>(x) / width);
            vertices.push_back(static_cast<float>(z) / height);
        }
    }
    
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int i = z * width + x;
            indices.push_back(i);
            indices.push_back(i + width);
            indices.push_back(i + 1);
            indices.push_back(i + 1);
            indices.push_back(i + width);
            indices.push_back(i + width + 1);
        }
    }
}

void TerrainMeshBuilder::BuildLODMesh(const Heightmap& heightmap, int chunkX, int chunkZ,
                                      LODLevel lod, std::vector<float>& vertices,
                                      std::vector<uint32_t>& indices) {
    int step = 1 << static_cast<int>(lod);
    int width = heightmap.GetWidth();
    int height = heightmap.GetHeight();
    vertices.clear();
    indices.clear();
    
    for (int z = 0; z < height; z += step) {
        for (int x = 0; x < width; x += step) {
            float h = heightmap.GetHeight(x, z);
            vertices.push_back((chunkX * CHUNK_SIZE + x));
            vertices.push_back(h);
            vertices.push_back((chunkZ * CHUNK_SIZE + z));
        }
    }
    
    int reducedWidth = (width + step - 1) / step;
    for (int z = 0; z < reducedWidth - 1; ++z) {
        for (int x = 0; x < reducedWidth - 1; ++x) {
            int i = z * reducedWidth + x;
            indices.push_back(i);
            indices.push_back(i + reducedWidth);
            indices.push_back(i + 1);
            indices.push_back(i + 1);
            indices.push_back(i + reducedWidth);
            indices.push_back(i + reducedWidth + 1);
        }
    }
}

void TerrainMeshBuilder::StitchLODEdges(std::vector<float>& vertices,
                                          std::vector<uint32_t>& indices,
                                          LODLevel centerLOD, LODLevel neighborLOD,
                                          int edge) {
    (void)vertices;
    (void)indices;
    (void)centerLOD;
    (void)neighborLOD;
    (void)edge;
}

int TerrainMeshBuilder::GetVertexIndex(int x, int z, int resolution) {
    return z * resolution + x;
}

float TerrainMeshBuilder::GetHeightAtVertex(const Heightmap& heightmap, int x, int z,
                                            int chunkX, int chunkZ, int resolution) {
    (void)chunkX;
    (void)chunkZ;
    int worldX = x * (heightmap.GetWidth() / resolution);
    int worldZ = z * (heightmap.GetHeight() / resolution);
    return heightmap.GetHeight(worldX, worldZ);
}

// ============================================
// Terrain Decorator Implementation
// ============================================

TerrainDecorator::TerrainDecorator(int seed) : seed(seed) {}
TerrainDecorator::~TerrainDecorator() {}

void TerrainDecorator::AddTree(const std::string& type, float probability,
                               int minHeight, int maxHeight) {
    TerrainDecoration decor;
    decor.type = "tree_" + type;
    decor.probability = probability;
    decor.minHeight = minHeight;
    decor.maxHeight = maxHeight;
    decorations.push_back(decor);
}

void TerrainDecorator::AddRock(const std::string& type, float probability,
                               int minHeight, int maxHeight) {
    TerrainDecoration decor;
    decor.type = "rock_" + type;
    decor.probability = probability;
    decor.minHeight = minHeight;
    decor.maxHeight = maxHeight;
    decorations.push_back(decor);
}

void TerrainDecorator::AddGrass(float probability, int minHeight, int maxHeight) {
    TerrainDecoration decor;
    decor.type = "grass";
    decor.probability = probability;
    decor.minHeight = minHeight;
    decor.maxHeight = maxHeight;
    decorations.push_back(decor);
}

void TerrainDecorator::AddFlower(const std::string& type, float probability,
                                 int minHeight, int maxHeight) {
    TerrainDecoration decor;
    decor.type = "flower_" + type;
    decor.probability = probability;
    decor.minHeight = minHeight;
    decor.maxHeight = maxHeight;
    decorations.push_back(decor);
}

void TerrainDecorator::AddBush(const std::string& type, float probability,
                               int minHeight, int maxHeight) {
    TerrainDecoration decor;
    decor.type = "bush_" + type;
    decor.probability = probability;
    decor.minHeight = minHeight;
    decor.maxHeight = maxHeight;
    decorations.push_back(decor);
}

void TerrainDecorator::DecorateChunk(Chunk* chunk, const Heightmap& heightmap,
                                     int chunkX, int chunkZ) {
    (void)chunk;
    (void)heightmap;
    (void)chunkX;
    (void)chunkZ;
}

void TerrainDecorator::DecorateTerrain(World* world, const Heightmap& heightmap,
                                     int startX, int startZ, int width, int height) {
    (void)world;
    (void)heightmap;
    (void)startX;
    (void)startZ;
    (void)width;
    (void)height;
}

bool TerrainDecorator::CanPlaceDecoration(const Vec3& position, const std::string& type) const {
    (void)position;
    (void)type;
    return true;
}

Vec3 TerrainDecorator::FindValidPosition(const Heightmap& heightmap, int chunkX, int chunkZ) const {
    (void)heightmap;
    (void)chunkX;
    (void)chunkZ;
    return Vec3(0, 0, 0);
}

float TerrainDecorator::RandomFloat() const {
    return std::sin(static_cast<float>(rand()) + seed) * std::cos(static_cast<float>(rand()) + seed * 0.5f);
}

int TerrainDecorator::RandomInt(int min, int max) const {
    return min + rand() % (max - min + 1);
}

// ============================================
// Terrain System Implementation
// ============================================

TerrainSystem::TerrainSystem(World* world)
    : world(world), globalHeightmapScale(1.0f),
      useStreaming(false), useLOD(false), useDecoration(false), useHeightmap(false),
      totalChunksGenerated(0), totalDecorationsPlaced(0) {}

TerrainSystem::~TerrainSystem() {}

void TerrainSystem::Initialize(TerrainType type, int seed) {
    generator = std::make_unique<TerrainGenerator>(type, seed);
    streamer = std::make_unique<TerrainStreamer>(world);
    lodManager = std::make_unique<TerrainLODManager>();
    decorator = std::make_unique<TerrainDecorator>(seed);
}

void TerrainSystem::SetHeightmap(const std::string& filename) {
    (void)filename;
}

void TerrainSystem::SetHeightmap(std::unique_ptr<Heightmap> heightmap) {
    globalHeightmap = std::move(heightmap);
    useHeightmap = true;
}

void TerrainSystem::Update(const Vec3& cameraPos, const Vec3& cameraDir, float deltaTime) {
    (void)deltaTime;
    if (useStreaming && streamer) {
        streamer->Update(cameraPos, cameraDir);
    }
    if (useLOD && lodManager) {
        lodManager->UpdateChunkLOD(
            static_cast<int>(cameraPos.x) / CHUNK_SIZE,
            static_cast<int>(cameraPos.z) / CHUNK_SIZE,
            0.0f
        );
    }
}

void TerrainSystem::GenerateChunk(int chunkX, int chunkZ) {
    if (!generator || !world) return;
    Chunk* chunk = world->GetChunk(chunkX, 0, chunkZ);
    if (!chunk) {
        chunk = world->GetOrCreateChunk(chunkX, 0, chunkZ);
    }
    generator->GenerateChunk(chunk, chunkX, chunkZ);
    totalChunksGenerated++;
    if (useDecoration && decorator && globalHeightmap) {
        decorator->DecorateChunk(chunk, *globalHeightmap, chunkX, chunkZ);
        totalDecorationsPlaced++;
    }
}

void TerrainSystem::GenerateRegion(int startX, int startZ, int width, int height) {
    for (int z = startZ; z < startZ + height; ++z) {
        for (int x = startX; x < startX + width; ++x) {
            GenerateChunk(x, z);
        }
    }
}

void TerrainSystem::RegenerateChunk(int chunkX, int chunkZ) {
    GenerateChunk(chunkX, chunkZ);
}

float TerrainSystem::GetHeightAt(float worldX, float worldZ) const {
    if (generator) return generator->GetHeightAt(worldX, worldZ);
    return 0.0f;
}

Vec3 TerrainSystem::GetNormalAt(float worldX, float worldZ) const {
    if (!generator) return Vec3(0, 1, 0);
    float hL = generator->GetHeightAt(worldX - 1.0f, worldZ);
    float hR = generator->GetHeightAt(worldX + 1.0f, worldZ);
    float hD = generator->GetHeightAt(worldX, worldZ - 1.0f);
    float hU = generator->GetHeightAt(worldX, worldZ + 1.0f);
    Vec3 normal(hL - hR, 2.0f, hD - hU);
    return normal.normalize();
}

void TerrainSystem::DecorateChunk(int chunkX, int chunkZ) {
    if (!decorator || !world || !globalHeightmap) return;
    Chunk* chunk = world->GetChunk(chunkX, 0, chunkZ);
    if (chunk) {
        decorator->DecorateChunk(chunk, *globalHeightmap, chunkX, chunkZ);
    }
}

void TerrainSystem::DecorateRegion(int startX, int startZ, int width, int height) {
    if (!decorator || !globalHeightmap) return;
    for (int z = startZ; z < startZ + height; ++z) {
        for (int x = startX; x < startX + width; ++x) {
            DecorateChunk(x, z);
        }
    }
}

bool TerrainSystem::IsChunkLoaded(int chunkX, int chunkZ) const {
    if (streamer) return streamer->IsChunkLoaded(chunkX, chunkZ);
    return false;
}

void TerrainSystem::ForceLoadChunk(int chunkX, int chunkZ) {
    if (streamer) streamer->RequestChunk(chunkX, chunkZ);
}

void TerrainSystem::ForceUnloadChunk(int chunkX, int chunkZ) {
    if (streamer) streamer->ReleaseChunk(chunkX, chunkZ);
}

void TerrainSystem::SaveTerrain(const std::string& filename) const {
    (void)filename;
}

void TerrainSystem::LoadTerrain(const std::string& filename) {
    (void)filename;
}

void TerrainSystem::ClearTerrain() {
    totalChunksGenerated = 0;
    totalDecorationsPlaced = 0;
}

int TerrainSystem::GetLoadedChunkCount() const {
    if (streamer) return streamer->GetLoadedChunkCount();
    return 0;
}

} // namespace vge
