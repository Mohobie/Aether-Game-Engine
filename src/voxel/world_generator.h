#pragma once
#include "chunk.h"
#include "block_types.h"
#include <random>

namespace vge {
    class WorldGenerator {
    public:
        WorldGenerator();
        void GenerateChunk(Chunk& chunk, int cx, int cy, int cz);
        void SetSeed(unsigned int seed);
        float GetNoise(float x, float y, float z);
        float GetHeightNoise(float x, float z);
        void GenerateTree(Chunk& chunk, int x, int y, int z);
        void GenerateTree(Chunk& chunk, int x, int y, int z, BlockTypeID trunkId, BlockTypeID leafId);
        
        // Set block IDs for generation (data-driven)
        void SetSurfaceBlock(const std::string& id) { surfaceBlock = id; }
        void SetSubsurfaceBlock(const std::string& id) { subsurfaceBlock = id; }
        void SetStoneBlock(const std::string& id) { stoneBlock = id; }
        void SetWaterBlock(const std::string& id) { waterBlock = id; }
        void SetTreeTrunkBlock(const std::string& id) { treeTrunkBlock = id; }
        void SetTreeLeafBlock(const std::string& id) { treeLeafBlock = id; }
        
    private:
        unsigned int seed = 12345;
        std::mt19937 rng;
        
        // Configurable block IDs
        std::string surfaceBlock = "grass";
        std::string subsurfaceBlock = "dirt";
        std::string stoneBlock = "stone";
        std::string waterBlock = "water";
        std::string treeTrunkBlock = "wood";
        std::string treeLeafBlock = "leaves";
    };
}