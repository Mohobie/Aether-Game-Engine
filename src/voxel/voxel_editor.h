#pragma once
#include "math/vec3.h"
#include "voxel/block_types.h"
#include "voxel/world.h"
#include <functional>
#include <vector>
#include <string>

namespace vge {

// ============================================
// Voxel Editor Tool Types
// ============================================
enum class VoxelToolType {
    Place,      // Place blocks
    Remove,     // Remove blocks
    Paint,      // Change block type/color
    Smooth,     // Smooth terrain (average heights)
    Raise,      // Raise terrain
    Lower,      // Lower terrain
    Flatten,    // Flatten to target height
    Sphere,     // Sphere brush (place/remove in sphere)
    Box,        // Box brush (place/remove in box)
    Count
};

// ============================================
// Brush Shape
// ============================================
enum class BrushShape {
    Sphere,
    Box,
    Cylinder,
    Count
};

// ============================================
// Voxel Edit Operation
// ============================================
struct VoxelEdit {
    Vec3 position;
    BlockTypeID oldType;
    BlockTypeID newType;
    Vec3 oldColor;
    Vec3 newColor;
    uint8_t oldMetadata;
    uint8_t newMetadata;
    
    VoxelEdit() : oldType(BLOCK_AIR), newType(BLOCK_AIR), oldMetadata(0), newMetadata(0) {}
};

// ============================================
// Edit History for Undo/Redo
// ============================================
class EditHistory {
private:
    std::vector<std::vector<VoxelEdit>> undoStack;
    std::vector<std::vector<VoxelEdit>> redoStack;
    size_t maxSize;
    
public:
    EditHistory(size_t maxSize = 100);
    ~EditHistory();
    
    void PushEdit(const std::vector<VoxelEdit>& edits);
    bool CanUndo() const;
    bool CanRedo() const;
    std::vector<VoxelEdit> Undo();
    std::vector<VoxelEdit> Redo();
    void Clear();
    size_t GetUndoCount() const { return undoStack.size(); }
    size_t GetRedoCount() const { return redoStack.size(); }
};

// ============================================
// Voxel Brush Settings
// ============================================
struct BrushSettings {
    BrushShape shape;
    float radius;           // For sphere/cylinder
    Vec3 size;              // For box (x,y,z dimensions)
    float strength;         // 0.0 - 1.0 (for smooth/raise/lower)
    float falloff;          // 0.0 = hard edge, 1.0 = smooth falloff
    bool affectAir;         // Whether to affect air blocks
    
    BrushSettings() 
        : shape(BrushShape::Sphere), radius(1.0f), size(1,1,1),
          strength(1.0f), falloff(0.0f), affectAir(false) {}
};

// ============================================
// Voxel Editor
// ============================================
class VoxelEditor {
private:
    World* world;
    VoxelToolType currentTool;
    BlockTypeID selectedBlock;
    Vec3 selectedColor;
    BrushSettings brush;
    EditHistory history;
    
    // Callbacks
    std::function<void(const Vec3&, BlockTypeID)> onBlockPlaced;
    std::function<void(const Vec3&, BlockTypeID)> onBlockRemoved;
    std::function<void(const Vec3&, BlockTypeID, BlockTypeID)> onBlockPainted;
    std::function<void()> onTerrainModified;
    
public:
    VoxelEditor(World* world);
    ~VoxelEditor();
    
    // Tool selection
    void SetTool(VoxelToolType tool) { currentTool = tool; }
    VoxelToolType GetTool() const { return currentTool; }
    
    // Block selection
    void SetSelectedBlock(BlockTypeID type) { selectedBlock = type; }
    BlockTypeID GetSelectedBlock() const { return selectedBlock; }
    void SetSelectedColor(const Vec3& color) { selectedColor = color; }
    Vec3 GetSelectedColor() const { return selectedColor; }
    
    // Brush settings
    void SetBrushShape(BrushShape shape) { brush.shape = shape; }
    BrushShape GetBrushShape() const { return brush.shape; }
    void SetBrushRadius(float radius) { brush.radius = radius; }
    float GetBrushRadius() const { return brush.radius; }
    void SetBrushSize(const Vec3& size) { brush.size = size; }
    Vec3 GetBrushSize() const { return brush.size; }
    void SetBrushStrength(float strength) { brush.strength = strength; }
    float GetBrushStrength() const { return brush.strength; }
    void SetBrushFalloff(float falloff) { brush.falloff = falloff; }
    float GetBrushFalloff() const { return brush.falloff; }
    void SetAffectAir(bool affect) { brush.affectAir = affect; }
    bool GetAffectAir() const { return brush.affectAir; }
    BrushSettings& GetBrushSettings() { return brush; }
    
    // Primary action at position
    bool ApplyTool(const Vec3& position, const Vec3& normal);
    bool ApplyToolAt(const Vec3& worldPos);
    
    // Specific tool operations
    bool PlaceBlock(const Vec3& position, const Vec3& normal);
    bool RemoveBlock(const Vec3& position);
    bool PaintBlock(const Vec3& position);
    bool SmoothTerrain(const Vec3& position);
    bool RaiseTerrain(const Vec3& position);
    bool LowerTerrain(const Vec3& position);
    bool FlattenTerrain(const Vec3& position, float targetHeight);
    
    // Brush operations (affects multiple blocks)
    std::vector<VoxelEdit> ApplyBrush(const Vec3& center);
    std::vector<VoxelEdit> ApplyBrushSphere(const Vec3& center, BlockTypeID type);
    std::vector<VoxelEdit> ApplyBrushBox(const Vec3& center, BlockTypeID type);
    std::vector<VoxelEdit> ApplyBrushCylinder(const Vec3& center, BlockTypeID type);
    
    // Undo/Redo
    bool Undo();
    bool Redo();
    bool CanUndo() const { return history.CanUndo(); }
    bool CanRedo() const { return history.CanRedo(); }
    void ClearHistory() { history.Clear(); }
    
    // History access
    size_t GetUndoCount() const { return history.GetUndoCount(); }
    size_t GetRedoCount() const { return history.GetRedoCount(); }
    
    // Apply edits (for undo/redo)
    void ApplyEdits(const std::vector<VoxelEdit>& edits, bool reverse);
    
    // Utility
    std::vector<Vec3> GetAffectedBlocks(const Vec3& center) const;
    bool IsInBrushRadius(const Vec3& center, const Vec3& block) const;
    float GetBrushFalloffFactor(const Vec3& center, const Vec3& block) const;
    
    // Callbacks
    void SetOnBlockPlaced(std::function<void(const Vec3&, BlockTypeID)> callback);
    void SetOnBlockRemoved(std::function<void(const Vec3&, BlockTypeID)> callback);
    void SetOnBlockPainted(std::function<void(const Vec3&, BlockTypeID, BlockTypeID)> callback);
    void SetOnTerrainModified(std::function<void()> callback);
    
    // Validation
    bool CanPlaceAt(const Vec3& position) const;
    bool CanRemoveAt(const Vec3& position) const;
    
    // Serialization
    std::string SerializeHistory() const;
    bool DeserializeHistory(const std::string& data);
};

// ============================================
// Terrain Painting
// ============================================
struct PaintLayer {
    BlockTypeID blockType;
    Vec3 color;
    float blendWeight;
    float minHeight;
    float maxHeight;
    float slopeMin;
    float slopeMax;
    
    PaintLayer() : blockType(BLOCK_AIR), blendWeight(1.0f), 
                   minHeight(-9999), maxHeight(9999), slopeMin(0), slopeMax(90) {}
};

class TerrainPainter {
private:
    World* world;
    std::vector<PaintLayer> layers;
    int seed;
    
public:
    TerrainPainter(World* world, int seed = 0);
    ~TerrainPainter();
    
    // Layer management
    void AddLayer(const PaintLayer& layer);
    void RemoveLayer(size_t index);
    void ClearLayers();
    size_t GetLayerCount() const { return layers.size(); }
    PaintLayer& GetLayer(size_t index);
    const PaintLayer& GetLayer(size_t index) const;
    
    // Painting
    void PaintChunk(int chunkX, int chunkY, int chunkZ);
    void PaintRegion(int startX, int startY, int startZ, int width, int height, int depth);
    void PaintWorld();
    
    // Slope-based painting
    void PaintBySlope(int chunkX, int chunkY, int chunkZ);
    void PaintByHeight(int chunkX, int chunkY, int chunkZ);
    
    // Blend painting
    void BlendPaintChunk(int chunkX, int chunkY, int chunkZ);
    
    // Utility
    float CalculateSlope(int x, int y, int z) const;
    float CalculateSlopeAt(float worldX, float worldZ) const;
    int SelectLayerForBlock(int x, int y, int z) const;
    
    // Noise-based variation
    float GetNoiseVariation(int x, int y, int z) const;
    
    // Serialization
    bool SaveLayers(const std::string& filename) const;
    bool LoadLayers(const std::string& filename);
};

// ============================================
// Block Modification (Metadata/State)
// ============================================
struct BlockModification {
    Vec3 position;
    uint8_t newMetadata;
    uint8_t oldMetadata;
    Vec3 newColor;
    Vec3 oldColor;
    
    BlockModification() : newMetadata(0), oldMetadata(0) {}
};

class BlockModifier {
private:
    World* world;
    std::vector<BlockModification> modifications;
    
public:
    BlockModifier(World* world);
    ~BlockModifier();
    
    // Metadata operations
    bool SetBlockMetadata(int x, int y, int z, uint8_t metadata);
    bool SetBlockMetadata(const Vec3& pos, uint8_t metadata);
    uint8_t GetBlockMetadata(int x, int y, int z) const;
    uint8_t GetBlockMetadata(const Vec3& pos) const;
    
    // Color operations
    bool SetBlockColor(int x, int y, int z, const Vec3& color);
    bool SetBlockColor(const Vec3& pos, const Vec3& color);
    Vec3 GetBlockColor(int x, int y, int z) const;
    Vec3 GetBlockColor(const Vec3& pos) const;
    
    // State operations (rotation, orientation, etc.)
    bool SetBlockRotation(int x, int y, int z, uint8_t rotation);
    uint8_t GetBlockRotation(int x, int y, int z) const;
    
    // Bulk operations
    void ModifyRegion(int startX, int startY, int startZ, 
                      int width, int height, int depth,
                      uint8_t metadata);
    void ColorRegion(int startX, int startY, int startZ,
                     int width, int height, int depth,
                     const Vec3& color);
    
    // Apply stored modifications
    void ApplyModifications();
    void ClearModifications();
    
    // Serialization
    std::string SerializeModifications() const;
    bool DeserializeModifications(const std::string& data);
};

// ============================================
// Voxel Selection (for box/area selection)
// ============================================
struct VoxelSelection {
    Vec3 start;
    Vec3 end;
    bool active;
    
    VoxelSelection() : active(false) {}
    
    void SetStart(const Vec3& pos) { start = pos; active = true; }
    void SetEnd(const Vec3& pos) { end = pos; }
    void Clear() { active = false; }
    bool IsValid() const { return active; }
    
    Vec3 GetMin() const {
        return Vec3(std::min(start.x, end.x), std::min(start.y, end.y), std::min(start.z, end.z));
    }
    Vec3 GetMax() const {
        return Vec3(std::max(start.x, end.x), std::max(start.y, end.y), std::max(start.z, end.z));
    }
    
    std::vector<Vec3> GetSelectedBlocks() const;
    int GetVolume() const;
};

// ============================================
// Fill Operations
// ============================================
class VoxelFill {
private:
    World* world;
    
public:
    VoxelFill(World* world);
    ~VoxelFill();
    
    // Fill selection with block
    std::vector<VoxelEdit> FillSelection(const VoxelSelection& selection, BlockTypeID type);
    
    // Replace blocks
    std::vector<VoxelEdit> ReplaceBlocks(const VoxelSelection& selection, 
                                         BlockTypeID oldType, BlockTypeID newType);
    
    // Clear selection (set to air)
    std::vector<VoxelEdit> ClearSelection(const VoxelSelection& selection);
    
    // Pattern fill
    std::vector<VoxelEdit> PatternFill(const VoxelSelection& selection,
                                        const std::vector<BlockTypeID>& pattern,
                                        int patternWidth, int patternHeight);
    
    // Gradient fill
    std::vector<VoxelEdit> GradientFill(const VoxelSelection& selection,
                                          BlockTypeID startType, BlockTypeID endType,
                                          const Vec3& direction);
};

} // namespace vge
