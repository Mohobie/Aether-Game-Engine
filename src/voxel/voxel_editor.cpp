#include "voxel_editor.h"
#include "world.h"
#include "chunk.h"
#include "block_types.h"
#include "block_registry.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace vge {

// ============================================
// EditHistory Implementation
// ============================================

EditHistory::EditHistory(size_t maxSize) : maxSize(maxSize) {}
EditHistory::~EditHistory() {}

void EditHistory::PushEdit(const std::vector<VoxelEdit>& edits) {
    if (edits.empty()) return;
    undoStack.push_back(edits);
    if (undoStack.size() > maxSize) {
        undoStack.erase(undoStack.begin());
    }
    redoStack.clear();
}

bool EditHistory::CanUndo() const { return !undoStack.empty(); }
bool EditHistory::CanRedo() const { return !redoStack.empty(); }

std::vector<VoxelEdit> EditHistory::Undo() {
    if (undoStack.empty()) return {};
    auto edits = undoStack.back();
    undoStack.pop_back();
    redoStack.push_back(edits);
    return edits;
}

std::vector<VoxelEdit> EditHistory::Redo() {
    if (redoStack.empty()) return {};
    auto edits = redoStack.back();
    redoStack.pop_back();
    undoStack.push_back(edits);
    return edits;
}

void EditHistory::Clear() {
    undoStack.clear();
    redoStack.clear();
}

// ============================================
// VoxelEditor Implementation
// ============================================

VoxelEditor::VoxelEditor(World* world)
    : world(world), currentTool(VoxelToolType::Place),
      selectedBlock(1), selectedColor(1,1,1), history(100) {}

VoxelEditor::~VoxelEditor() {}

bool VoxelEditor::ApplyTool(const Vec3& position, const Vec3& normal) {
    switch (currentTool) {
        case VoxelToolType::Place:
            return PlaceBlock(position, normal);
        case VoxelToolType::Remove:
            return RemoveBlock(position);
        case VoxelToolType::Paint:
            return PaintBlock(position);
        default:
            return false;
    }
}

bool VoxelEditor::ApplyToolAt(const Vec3& worldPos) {
    return ApplyTool(worldPos, Vec3(0, 1, 0));
}

bool VoxelEditor::PlaceBlock(const Vec3& position, const Vec3& normal) {
    if (!world) return false;
    
    Vec3 placePos = position + normal;
    
    std::vector<VoxelEdit> edits;
    VoxelEdit edit;
    edit.position = placePos;
    edit.oldType = world->GetBlock(placePos.x, placePos.y, placePos.z);
    edit.newType = selectedBlock;
    edits.push_back(edit);
    
    world->SetBlock(placePos.x, placePos.y, placePos.z, selectedBlock);
    history.PushEdit(edits);
    
    if (onBlockPlaced) onBlockPlaced(placePos, selectedBlock);
    return true;
}

bool VoxelEditor::RemoveBlock(const Vec3& position) {
    if (!world) return false;
    
    std::vector<VoxelEdit> edits;
    VoxelEdit edit;
    edit.position = position;
    edit.oldType = world->GetBlock(position.x, position.y, position.z);
    edit.newType = BLOCK_AIR;
    edits.push_back(edit);
    
    world->SetBlock(position.x, position.y, position.z, BLOCK_AIR);
    history.PushEdit(edits);
    
    if (onBlockRemoved) onBlockRemoved(position, edit.oldType);
    return true;
}

bool VoxelEditor::PaintBlock(const Vec3& position) {
    if (!world) return false;
    
    std::vector<VoxelEdit> edits;
    VoxelEdit edit;
    edit.position = position;
    edit.oldType = world->GetBlock(position.x, position.y, position.z);
    edit.newType = selectedBlock;
    edits.push_back(edit);
    
    world->SetBlock(position.x, position.y, position.z, selectedBlock);
    history.PushEdit(edits);
    
    if (onBlockPainted) onBlockPainted(position, edit.oldType, selectedBlock);
    return true;
}

bool VoxelEditor::SmoothTerrain(const Vec3& position) {
    (void)position;
    return false;
}

bool VoxelEditor::RaiseTerrain(const Vec3& position) {
    (void)position;
    return false;
}

bool VoxelEditor::LowerTerrain(const Vec3& position) {
    (void)position;
    return false;
}

bool VoxelEditor::FlattenTerrain(const Vec3& position, float targetHeight) {
    (void)position;
    (void)targetHeight;
    return false;
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrush(const Vec3& center) {
    switch (brush.shape) {
        case BrushShape::Sphere:
            return ApplyBrushSphere(center, selectedBlock);
        case BrushShape::Box:
            return ApplyBrushBox(center, selectedBlock);
        case BrushShape::Cylinder:
            return ApplyBrushCylinder(center, selectedBlock);
        default:
            return {};
    }
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrushSphere(const Vec3& center, BlockTypeID type) {
    std::vector<VoxelEdit> edits;
    if (!world) return edits;
    
    int r = (int)brush.radius;
    for (int x = -r; x <= r; ++x) {
        for (int y = -r; y <= r; ++y) {
            for (int z = -r; z <= r; ++z) {
                Vec3 pos = center + Vec3(x, y, z);
                float dist = std::sqrt(x*x + y*y + z*z);
                if (dist <= brush.radius) {
                    BlockTypeID old = world->GetBlock(pos.x, pos.y, pos.z);
                    if (old != type) {
                        VoxelEdit edit;
                        edit.position = pos;
                        edit.oldType = old;
                        edit.newType = type;
                        edits.push_back(edit);
                        world->SetBlock(pos.x, pos.y, pos.z, type);
                    }
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return edits;
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrushBox(const Vec3& center, BlockTypeID type) {
    std::vector<VoxelEdit> edits;
    if (!world) return edits;
    
    Vec3 halfSize = brush.size * 0.5f;
    for (int x = (int)-halfSize.x; x <= (int)halfSize.x; ++x) {
        for (int y = (int)-halfSize.y; y <= (int)halfSize.y; ++y) {
            for (int z = (int)-halfSize.z; z <= (int)halfSize.z; ++z) {
                Vec3 pos = center + Vec3(x, y, z);
                BlockTypeID old = world->GetBlock(pos.x, pos.y, pos.z);
                if (old != type) {
                    VoxelEdit edit;
                    edit.position = pos;
                    edit.oldType = old;
                    edit.newType = type;
                    edits.push_back(edit);
                    world->SetBlock(pos.x, pos.y, pos.z, type);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return edits;
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrushCylinder(const Vec3& center, BlockTypeID type) {
    std::vector<VoxelEdit> edits;
    (void)center;
    (void)type;
    return edits;
}

bool VoxelEditor::Undo() {
    auto edits = history.Undo();
    if (!edits.empty() && world) {
        for (const auto& edit : edits) {
            world->SetBlock(edit.position.x, edit.position.y, edit.position.z, edit.oldType);
        }
    }
    return !edits.empty();
}

bool VoxelEditor::Redo() {
    auto edits = history.Redo();
    if (!edits.empty() && world) {
        for (const auto& edit : edits) {
            world->SetBlock(edit.position.x, edit.position.y, edit.position.z, edit.newType);
        }
    }
    return !edits.empty();
}

void VoxelEditor::ApplyEdits(const std::vector<VoxelEdit>& edits, bool reverse) {
    if (!world) return;
    for (const auto& edit : edits) {
        BlockTypeID type = reverse ? edit.oldType : edit.newType;
        world->SetBlock(edit.position.x, edit.position.y, edit.position.z, type);
    }
}

std::vector<Vec3> VoxelEditor::GetAffectedBlocks(const Vec3& center) const {
    std::vector<Vec3> blocks;
    int r = (int)brush.radius;
    for (int x = -r; x <= r; ++x) {
        for (int y = -r; y <= r; ++y) {
            for (int z = -r; z <= r; ++z) {
                Vec3 pos = center + Vec3(x, y, z);
                if (IsInBrushRadius(center, pos)) {
                    blocks.push_back(pos);
                }
            }
        }
    }
    return blocks;
}

bool VoxelEditor::IsInBrushRadius(const Vec3& center, const Vec3& block) const {
    Vec3 diff = block - center;
    float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
    return dist <= brush.radius;
}

float VoxelEditor::GetBrushFalloffFactor(const Vec3& center, const Vec3& block) const {
    Vec3 diff = block - center;
    float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
    if (dist > brush.radius) return 0.0f;
    float falloff = static_cast<float>(brush.falloff);
    if (falloff <= 0.0f) return 1.0f;
    float t = dist / brush.radius;
    return 1.0f - (t * falloff);
}

void VoxelEditor::SetOnBlockPlaced(std::function<void(const Vec3&, BlockTypeID)> callback) {
    onBlockPlaced = callback;
}

void VoxelEditor::SetOnBlockRemoved(std::function<void(const Vec3&, BlockTypeID)> callback) {
    onBlockRemoved = callback;
}

void VoxelEditor::SetOnBlockPainted(std::function<void(const Vec3&, BlockTypeID, BlockTypeID)> callback) {
    onBlockPainted = callback;
}

void VoxelEditor::SetOnTerrainModified(std::function<void()> callback) {
    onTerrainModified = callback;
}

bool VoxelEditor::CanPlaceAt(const Vec3& position) const {
    if (!world) return false;
    return world->GetBlock(position.x, position.y, position.z) == BLOCK_AIR;
}

bool VoxelEditor::CanRemoveAt(const Vec3& position) const {
    if (!world) return false;
    return world->GetBlock(position.x, position.y, position.z) != BLOCK_AIR;
}

std::string VoxelEditor::SerializeHistory() const {
    return "";
}

bool VoxelEditor::DeserializeHistory(const std::string& data) {
    (void)data;
    return false;
}

// ============================================
// TerrainPainter Implementation
// ============================================

TerrainPainter::TerrainPainter(World* world, int seed) : world(world), seed(seed) {}
TerrainPainter::~TerrainPainter() {}

void TerrainPainter::AddLayer(const PaintLayer& layer) {
    layers.push_back(layer);
}

void TerrainPainter::RemoveLayer(size_t index) {
    if (index < layers.size()) {
        layers.erase(layers.begin() + index);
    }
}

void TerrainPainter::ClearLayers() {
    layers.clear();
}

PaintLayer& TerrainPainter::GetLayer(size_t index) {
    return layers[index];
}

const PaintLayer& TerrainPainter::GetLayer(size_t index) const {
    return layers[index];
}

void TerrainPainter::PaintChunk(int chunkX, int chunkY, int chunkZ) {
    (void)chunkX; (void)chunkY; (void)chunkZ;
}

void TerrainPainter::PaintRegion(int startX, int startY, int startZ, int width, int height, int depth) {
    (void)startX; (void)startY; (void)startZ; (void)width; (void)height; (void)depth;
}

void TerrainPainter::PaintWorld(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ) {
    (void)minChunkX; (void)minChunkZ; (void)maxChunkX; (void)maxChunkZ;
}

void TerrainPainter::PaintBySlope(int chunkX, int chunkY, int chunkZ) {
    (void)chunkX; (void)chunkY; (void)chunkZ;
}

void TerrainPainter::PaintByHeight(int chunkX, int chunkY, int chunkZ) {
    (void)chunkX; (void)chunkY; (void)chunkZ;
}

void TerrainPainter::BlendPaintChunk(int chunkX, int chunkY, int chunkZ) {
    (void)chunkX; (void)chunkY; (void)chunkZ;
}

float TerrainPainter::CalculateSlope(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return 0.0f;
}

float TerrainPainter::CalculateSlopeAt(float worldX, float worldZ) const {
    (void)worldX; (void)worldZ;
    return 0.0f;
}

int TerrainPainter::SelectLayerForBlock(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return -1;
}

float TerrainPainter::GetNoiseVariation(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return 0.0f;
}

bool TerrainPainter::SaveLayers(const std::string& filename) const {
    (void)filename;
    return false;
}

bool TerrainPainter::LoadLayers(const std::string& filename) {
    (void)filename;
    return false;
}

// ============================================
// BlockModifier Implementation
// ============================================

BlockModifier::BlockModifier(World* world) : world(world) {}
BlockModifier::~BlockModifier() {}

bool BlockModifier::SetBlockMetadata(int x, int y, int z, uint8_t metadata) {
    (void)x; (void)y; (void)z; (void)metadata;
    return false;
}

bool BlockModifier::SetBlockMetadata(const Vec3& pos, uint8_t metadata) {
    return SetBlockMetadata(pos.x, pos.y, pos.z, metadata);
}

uint8_t BlockModifier::GetBlockMetadata(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return 0;
}

uint8_t BlockModifier::GetBlockMetadata(const Vec3& pos) const {
    return GetBlockMetadata(pos.x, pos.y, pos.z);
}

bool BlockModifier::SetBlockColor(int x, int y, int z, const Vec3& color) {
    (void)x; (void)y; (void)z; (void)color;
    return false;
}

bool BlockModifier::SetBlockColor(const Vec3& pos, const Vec3& color) {
    return SetBlockColor(pos.x, pos.y, pos.z, color);
}

Vec3 BlockModifier::GetBlockColor(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return Vec3(1, 1, 1);
}

Vec3 BlockModifier::GetBlockColor(const Vec3& pos) const {
    return GetBlockColor(pos.x, pos.y, pos.z);
}

bool BlockModifier::SetBlockRotation(int x, int y, int z, uint8_t rotation) {
    (void)x; (void)y; (void)z; (void)rotation;
    return false;
}

uint8_t BlockModifier::GetBlockRotation(int x, int y, int z) const {
    (void)x; (void)y; (void)z;
    return 0;
}

void BlockModifier::ModifyRegion(int startX, int startY, int startZ,
                                  int width, int height, int depth,
                                  uint8_t metadata) {
    (void)startX; (void)startY; (void)startZ; (void)width; (void)height; (void)depth; (void)metadata;
}

void BlockModifier::ColorRegion(int startX, int startY, int startZ,
                                 int width, int height, int depth,
                                 const Vec3& color) {
    (void)startX; (void)startY; (void)startZ; (void)width; (void)height; (void)depth; (void)color;
}

void BlockModifier::ApplyModifications() {
}

void BlockModifier::ClearModifications() {
    modifications.clear();
}

std::string BlockModifier::SerializeModifications() const {
    return "";
}

bool BlockModifier::DeserializeModifications(const std::string& data) {
    (void)data;
    return false;
}

// ============================================
// VoxelSelection Implementation
// ============================================

std::vector<Vec3> VoxelSelection::GetSelectedBlocks() const {
    std::vector<Vec3> blocks;
    if (!active) return blocks;
    
    Vec3 min = GetMin();
    Vec3 max = GetMax();
    
    for (int x = (int)min.x; x <= (int)max.x; ++x) {
        for (int y = (int)min.y; y <= (int)max.y; ++y) {
            for (int z = (int)min.z; z <= (int)max.z; ++z) {
                blocks.push_back(Vec3(x, y, z));
            }
        }
    }
    return blocks;
}

int VoxelSelection::GetVolume() const {
    if (!active) return 0;
    Vec3 min = GetMin();
    Vec3 max = GetMax();
    return ((int)(max.x - min.x + 1)) * ((int)(max.y - min.y + 1)) * ((int)(max.z - min.z + 1));
}

// ============================================
// VoxelFill Implementation
// ============================================

VoxelFill::VoxelFill(World* world) : world(world) {}
VoxelFill::~VoxelFill() {}

std::vector<VoxelEdit> VoxelFill::FillSelection(const VoxelSelection& selection, BlockTypeID type) {
    std::vector<VoxelEdit> edits;
    if (!world || !selection.active) return edits;
    
    auto blocks = selection.GetSelectedBlocks();
    for (const auto& pos : blocks) {
        BlockTypeID old = world->GetBlock(pos.x, pos.y, pos.z);
        if (old != type) {
            VoxelEdit edit;
            edit.position = pos;
            edit.oldType = old;
            edit.newType = type;
            edits.push_back(edit);
            world->SetBlock(pos.x, pos.y, pos.z, type);
        }
    }
    return edits;
}

std::vector<VoxelEdit> VoxelFill::ReplaceBlocks(const VoxelSelection& selection,
                                                   BlockTypeID oldType, BlockTypeID newType) {
    std::vector<VoxelEdit> edits;
    if (!world || !selection.active) return edits;
    
    auto blocks = selection.GetSelectedBlocks();
    for (const auto& pos : blocks) {
        BlockTypeID current = world->GetBlock(pos.x, pos.y, pos.z);
        if (current == oldType) {
            VoxelEdit edit;
            edit.position = pos;
            edit.oldType = current;
            edit.newType = newType;
            edits.push_back(edit);
            world->SetBlock(pos.x, pos.y, pos.z, newType);
        }
    }
    return edits;
}

std::vector<VoxelEdit> VoxelFill::ClearSelection(const VoxelSelection& selection) {
    return FillSelection(selection, BLOCK_AIR);
}

std::vector<VoxelEdit> VoxelFill::PatternFill(const VoxelSelection& selection,
                                               const std::vector<BlockTypeID>& pattern,
                                               int patternWidth, int patternHeight) {
    std::vector<VoxelEdit> edits;
    (void)selection; (void)pattern; (void)patternWidth; (void)patternHeight;
    return edits;
}

std::vector<VoxelEdit> VoxelFill::GradientFill(const VoxelSelection& selection,
                                                BlockTypeID startType, BlockTypeID endType,
                                                const Vec3& direction) {
    std::vector<VoxelEdit> edits;
    (void)selection;
    (void)startType;
    (void)endType;
    (void)direction;
    return edits;
}

} // namespace vge
