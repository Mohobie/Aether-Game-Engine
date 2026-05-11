#include "voxel/chunk.h"
#include "voxel/block_mesh_builder.h"
#include "voxel/block_registry.h"
#include "voxel/world_renderer.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace vge;

// Helper to check if two Vec3 are approximately equal
bool Vec3ApproxEqual(const Vec3& a, const Vec3& b, float epsilon = 0.001f) {
    return std::abs(a.x - b.x) < epsilon &&
           std::abs(a.y - b.y) < epsilon &&
           std::abs(a.z - b.z) < epsilon;
}

void TestBlockColors() {
    std::cout << "\n=== Block Color Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    
    // Load default blocks (try relative paths)
    bool loaded = registry.LoadFromFile("assets/blocks/default_blocks.json");
    if (!loaded) {
        loaded = registry.LoadFromFile("../assets/blocks/default_blocks.json");
    }
    if (!loaded) {
        loaded = registry.LoadFromFile("../../assets/blocks/default_blocks.json");
    }
    assert(loaded && "Failed to load block definitions");
    
    // Verify expected colors from JSON
    Vec3 grassColor = registry.GetBlock("grass").GetColor();
    Vec3 dirtColor = registry.GetBlock("dirt").GetColor();
    Vec3 stoneColor = registry.GetBlock("stone").GetColor();
    Vec3 woodColor = registry.GetBlock("wood").GetColor();
    Vec3 leavesColor = registry.GetBlock("leaves").GetColor();
    Vec3 sandColor = registry.GetBlock("sand").GetColor();
    Vec3 waterColor = registry.GetBlock("water").GetColor();
    
    // Grass = green
    assert(Vec3ApproxEqual(grassColor, Vec3(0.2f, 0.7f, 0.2f)) && "Grass color mismatch");
    std::cout << "✓ Grass color: (" << grassColor.x << ", " << grassColor.y << ", " << grassColor.z << ")" << std::endl;
    
    // Dirt = brown
    assert(Vec3ApproxEqual(dirtColor, Vec3(0.4f, 0.3f, 0.2f)) && "Dirt color mismatch");
    std::cout << "✓ Dirt color: (" << dirtColor.x << ", " << dirtColor.y << ", " << dirtColor.z << ")" << std::endl;
    
    // Stone = gray
    assert(Vec3ApproxEqual(stoneColor, Vec3(0.5f, 0.5f, 0.5f)) && "Stone color mismatch");
    std::cout << "✓ Stone color: (" << stoneColor.x << ", " << stoneColor.y << ", " << stoneColor.z << ")" << std::endl;
    
    // Wood = brown
    assert(Vec3ApproxEqual(woodColor, Vec3(0.4f, 0.25f, 0.1f)) && "Wood color mismatch");
    std::cout << "✓ Wood color: (" << woodColor.x << ", " << woodColor.y << ", " << woodColor.z << ")" << std::endl;
    
    // Leaves = green (slightly different)
    assert(Vec3ApproxEqual(leavesColor, Vec3(0.1f, 0.6f, 0.1f)) && "Leaves color mismatch");
    std::cout << "✓ Leaves color: (" << leavesColor.x << ", " << leavesColor.y << ", " << leavesColor.z << ")" << std::endl;
    
    // Sand = yellowish
    assert(Vec3ApproxEqual(sandColor, Vec3(0.9f, 0.85f, 0.6f)) && "Sand color mismatch");
    std::cout << "✓ Sand color: (" << sandColor.x << ", " << sandColor.y << ", " << sandColor.z << ")" << std::endl;
    
    // Water = blue
    assert(Vec3ApproxEqual(waterColor, Vec3(0.1f, 0.3f, 0.8f)) && "Water color mismatch");
    std::cout << "✓ Water color: (" << waterColor.x << ", " << waterColor.y << ", " << waterColor.z << ")" << std::endl;
    
    std::cout << "Block color test passed!" << std::endl;
}

void TestChunkMeshBuilder() {
    std::cout << "\n=== Chunk Mesh Builder Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    if (registry.GetCount() <= 1) {
        registry.LoadFromFile("assets/blocks/default_blocks.json");
    }
    
    Chunk chunk(0, 0, 0);
    chunk.loaded = true;
    
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    
    // Create a simple 2x2x2 structure at the corner
    chunk.SetBlock(0, 0, 0, grass);
    chunk.SetBlock(1, 0, 0, dirt);
    chunk.SetBlock(0, 1, 0, stone);
    chunk.SetBlock(0, 0, 1, grass);
    
    // Build mesh
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(chunk);
    
    std::cout << "Mesh vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "Mesh indices: " << mesh.GetIndexCount() << std::endl;
    
    // A single isolated block has 6 faces * 4 vertices = 24 vertices
    // But with our 2x2x2 structure, some faces are hidden
    // grass at (0,0,0): 3 faces exposed (front, right, top) = 3*4 = 12 verts
    // dirt at (1,0,0): 4 faces exposed = 16 verts
    // stone at (0,1,0): 4 faces exposed = 16 verts  
    // grass at (0,0,1): 3 faces exposed = 12 verts
    // Total: ~56 verts (some faces between blocks are hidden)
    
    assert(mesh.GetVertexCount() > 0 && "Mesh should have vertices");
    assert(mesh.GetIndexCount() > 0 && "Mesh should have indices");
    assert(mesh.GetIndexCount() % 3 == 0 && "Indices should form triangles");
    
    std::cout << "✓ Mesh generated successfully" << std::endl;
    std::cout << "✓ Vertices: " << mesh.GetVertexCount() << ", Indices: " << mesh.GetIndexCount() << std::endl;
    
    std::cout << "Chunk mesh builder test passed!" << std::endl;
}

void TestFaceCulling() {
    std::cout << "\n=== Face Culling Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    if (registry.GetCount() <= 1) {
        registry.LoadFromFile("assets/blocks/default_blocks.json");
    }
    
    Chunk chunk(0, 0, 0);
    chunk.loaded = true;
    
    BlockTypeID stone = registry.GetBlockId("stone");
    
    // Create a solid 3x3x3 block (all stone)
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                chunk.SetBlock(x, y, z, stone);
            }
        }
    }
    
    Mesh mesh = BlockMeshBuilder::BuildChunkMesh(chunk);
    
    // A 3x3x3 solid cube should only render the outer shell
    // 6 faces * 9 blocks per face = 54 exposed faces
    // But corners/edges are shared, so actually:
    // Each face of the big cube is 3x3 = 9 quads = 9*4 = 36 verts per face
    // 6 faces * 36 verts = 216 verts (but shared edges reduce this slightly)
    // Actually with proper face culling, internal faces are skipped entirely
    
    std::cout << "Solid 3x3x3 mesh vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "Solid 3x3x3 mesh indices: " << mesh.GetIndexCount() << std::endl;
    
    // Should be much less than 3*3*3*24 = 648 (no culling)
    // With culling: 6 faces * 9 quads * 4 verts = 216 verts
    assert(mesh.GetVertexCount() < 300 && "Face culling should reduce vertex count significantly");
    
    std::cout << "✓ Face culling working - reduced from ~648 to " << mesh.GetVertexCount() << " vertices" << std::endl;
    
    std::cout << "Face culling test passed!" << std::endl;
}

void TestWorldRenderer() {
    std::cout << "\n=== World Renderer Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    if (registry.GetCount() <= 1) {
        registry.LoadFromFile("assets/blocks/default_blocks.json");
    }
    
    WorldRenderer worldRenderer;
    Shader shader;
    shader.LoadFromSource(Shader::GetUnlitVertexShader(), 
                          Shader::GetUnlitFragmentShader());
    worldRenderer.Initialize(&shader);
    
    Chunk chunk(0, 0, 0);
    chunk.loaded = true;
    
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    
    // Create a small terrain
    for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
            chunk.SetBlock(x, 0, z, dirt);
            chunk.SetBlock(x, 1, z, grass);
        }
    }
    
    Camera camera;
    camera.SetPosition(Vec3(16, 16, 32));
    
    // Render chunk (should build mesh)
    worldRenderer.RenderChunk(&chunk, camera);
    
    assert(worldRenderer.HasMesh(&chunk) && "Chunk should have mesh after render");
    assert(worldRenderer.GetMeshCount() == 1 && "Should have exactly 1 mesh");
    
    std::cout << "✓ WorldRenderer created mesh for chunk" << std::endl;
    
    // Modify chunk and verify dirty flag triggers rebuild
    chunk.SetBlock(2, 2, 2, dirt);
    assert(chunk.IsDirty() && "Chunk should be dirty after modification");
    
    worldRenderer.RenderChunk(&chunk, camera);
    assert(!chunk.IsDirty() && "Chunk should be clean after render update");
    
    std::cout << "✓ Mesh rebuilt when chunk changed" << std::endl;
    
    worldRenderer.Cleanup();
    assert(worldRenderer.GetMeshCount() == 0 && "Should have 0 meshes after cleanup");
    
    std::cout << "✓ Cleanup successful" << std::endl;
    
    std::cout << "World renderer test passed!" << std::endl;
}

void TestRendererIntegration() {
    std::cout << "\n=== Renderer Integration Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    if (registry.GetCount() <= 1) {
        registry.LoadFromFile("assets/blocks/default_blocks.json");
    }
    
    Renderer renderer;
    bool init = renderer.Initialize();
    assert(init && "Renderer should initialize");
    
    WorldRenderer* worldRenderer = renderer.GetWorldRenderer();
    assert(worldRenderer != nullptr && "Renderer should have WorldRenderer");
    
    std::cout << "✓ Renderer initialized with WorldRenderer" << std::endl;
    
    // Create a world with some chunks
    World world;
    BlockTypeID grass = registry.GetBlockId("grass");
    BlockTypeID dirt = registry.GetBlockId("dirt");
    BlockTypeID stone = registry.GetBlockId("stone");
    
    // Set up a simple terrain
    for (int x = -4; x < 4; x++) {
        for (int z = -4; z < 4; z++) {
            world.SetBlock(x, 0, z, dirt);
            world.SetBlock(x, 1, z, grass);
            world.SetBlock(x, -1, z, stone);
        }
    }
    
    Camera camera;
    camera.SetPosition(Vec3(0, 5, 10));
    
    // Render world (ASCII mode)
    renderer.BeginFrame();
    renderer.RenderWorld(world, camera);
    renderer.EndFrame();
    
    std::cout << "✓ World rendered through Renderer" << std::endl;
    
    // Verify meshes were built
    // The renderer should have updated chunk meshes during RenderWorld
    
    renderer.Shutdown();
    std::cout << "✓ Renderer shutdown cleanly" << std::endl;
    
    std::cout << "Renderer integration test passed!" << std::endl;
}

void TestNeighborAwareMeshBuilding() {
    std::cout << "\n=== Neighbor-Aware Mesh Building Test ===" << std::endl;
    
    BlockRegistry& registry = BlockRegistry::GetInstance();
    if (registry.GetCount() <= 1) {
        registry.LoadFromFile("assets/blocks/default_blocks.json");
    }
    
    BlockTypeID stone = registry.GetBlockId("stone");
    
    // Create two adjacent chunks
    Chunk chunkA(0, 0, 0);
    chunkA.loaded = true;
    Chunk chunkB(1, 0, 0); // Adjacent in +X direction
    chunkB.loaded = true;
    
    // Fill chunkA with stone
    for (int y = 0; y < 4; y++) {
        for (int z = 0; z < 4; z++) {
            chunkA.SetBlock(CHUNK_SIZE - 1, y, z, stone); // Edge block
        }
    }
    
    // Fill chunkB with stone
    for (int y = 0; y < 4; y++) {
        for (int z = 0; z < 4; z++) {
            chunkB.SetBlock(0, y, z, stone); // Edge block
        }
    }
    
    // Build mesh for chunkA without neighbor info
    Mesh meshNoNeighbor = BlockMeshBuilder::BuildChunkMesh(chunkA);
    
    // Build mesh for chunkA with chunkB as neighbor
    Mesh meshWithNeighbor = BlockMeshBuilder::BuildChunkMesh(chunkA, &chunkB, nullptr, nullptr, nullptr, nullptr, nullptr);
    
    std::cout << "Mesh without neighbor: " << meshNoNeighbor.GetVertexCount() << " vertices" << std::endl;
    std::cout << "Mesh with neighbor: " << meshWithNeighbor.GetVertexCount() << " vertices" << std::endl;
    
    // With neighbor, the face between chunks should be culled
    // Without neighbor, that face is rendered (assuming air on the other side)
    assert(meshWithNeighbor.GetVertexCount() < meshNoNeighbor.GetVertexCount() && 
           "Mesh with neighbor should have fewer vertices (face culled)");
    
    std::cout << "✓ Neighbor-aware culling works - saved " 
              << (meshNoNeighbor.GetVertexCount() - meshWithNeighbor.GetVertexCount()) 
              << " vertices" << std::endl;
    
    std::cout << "Neighbor-aware mesh building test passed!" << std::endl;
}

int main() {
    std::cout << "=== Renderer Integration Tests ===" << std::endl;
    
    TestBlockColors();
    TestChunkMeshBuilder();
    TestFaceCulling();
    TestWorldRenderer();
    TestRendererIntegration();
    TestNeighborAwareMeshBuilding();
    
    std::cout << "\n=== All Renderer Integration Tests Passed ===" << std::endl;
    return 0;
}
