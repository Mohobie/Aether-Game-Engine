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
    // Clear redo stack on new edit
    redoStack.clear();
}

bool EditHistory::CanUndo() const {
    return !undoStack.empty();
}

bool EditHistory::CanRedo() const {
    return !redoStack.empty();
}

std::vector<VoxelEdit> EditHistory::Undo() {
    if (!CanUndo()) return {};
    
    std::vector<VoxelEdit> edits = undoStack.back();
    undoStack.pop_back();
    redoStack.push_back(edits);
    return edits;
}

std::vector<VoxelEdit> EditHistory::Redo() {
    if (!CanRedo()) return {};
    
    std::vector<VoxelEdit> edits = redoStack.back();
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
      selectedBlock(BLOCK_AIR), selectedColor(1.0f, 1.0f, 1.0f),
      history(100) {}

VoxelEditor::~VoxelEditor() {}

bool VoxelEditor::ApplyTool(const Vec3& position, const Vec3& normal) {
    switch (currentTool) {
        case VoxelToolType::Place:
            return PlaceBlock(position, normal);
        case VoxelToolType::Remove:
            return RemoveBlock(position);
        case VoxelToolType::Paint:
            return PaintBlock(position);
        case VoxelToolType::Smooth:
            return SmoothTerrain(position);
        case VoxelToolType::Raise:
            return RaiseTerrain(position);
        case VoxelToolType::Lower:
            return LowerTerrain(position);
        case VoxelToolType::Flatten:
            return FlattenTerrain(position, position.y);
        case VoxelToolType::Sphere:
        case VoxelToolType::Box:
            return !ApplyBrush(position).empty();
        default:
            return false;
    }
}

bool VoxelEditor::ApplyToolAt(const Vec3& worldPos) {
    Vec3 blockPos(std::floor(worldPos.x), std::floor(worldPos.y), std::floor(worldPos.z));
    return ApplyTool(blockPos, Vec3(0, 1, 0));
}

bool VoxelEditor::PlaceBlock(const Vec3& position, const Vec3& normal) {
    if (!world) return false;
    
    Vec3 placePos = position + normal;
    int x = (int)std::floor(placePos.x);
    int y = (int)std::floor(placePos.y);
    int z = (int)std::floor(placePos.z);
    
    if (!CanPlaceAt(Vec3((float)x, (float)y, (float)z))) return false;
    
    BlockTypeID oldType = world->GetBlock(x, y, z);
    world->SetBlock(x, y, z, selectedBlock);
    
    VoxelEdit edit;
    edit.position = Vec3((float)x, (float)y, (float)z);
    edit.oldType = oldType;
    edit.newType = selectedBlock;
    history.PushEdit({edit});
    
    if (onBlockPlaced) onBlockPlaced(Vec3((float)x, (float)y, (float)z), selectedBlock);
    return true;
}

bool VoxelEditor::RemoveBlock(const Vec3& position) {
    if (!world) return false;
    
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    
    if (!CanRemoveAt(Vec3((float)x, (float)y, (float)z))) return false;
    
    BlockTypeID oldType = world->GetBlock(x, y, z);
    world->SetBlock(x, y, z, BLOCK_AIR);
    
    VoxelEdit edit;
    edit.position = Vec3((float)x, (float)y, (float)z);
    edit.oldType = oldType;
    edit.newType = BLOCK_AIR;
    history.PushEdit({edit});
    
    if (onBlockRemoved) onBlockRemoved(Vec3((float)x, (float)y, (float)z), oldType);
    return true;
}

bool VoxelEditor::PaintBlock(const Vec3& position) {
    if (!world) return false;
    
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    
    BlockTypeID oldType = world->GetBlock(x, y, z);
    if (oldType == BLOCK_AIR) return false;
    
    world->SetBlock(x, y, z, selectedBlock);
    
    VoxelEdit edit;
    edit.position = Vec3((float)x, (float)y, (float)z);
    edit.oldType = oldType;
    edit.newType = selectedBlock;
    history.PushEdit({edit});
    
    if (onBlockPainted) onBlockPainted(Vec3((float)x, (float)y, (float)z), oldType, selectedBlock);
    return true;
}

bool VoxelEditor::SmoothTerrain(const Vec3& position) {
    if (!world) return false;
    
    int cx = (int)std::floor(position.x);
    int cy = (int)std::floor(position.y);
    int cz = (int)std::floor(position.z);
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            for (int dy = -radius; dy <= radius; ++dy) {
                Vec3 blockPos((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                if (!IsInBrushRadius(position, blockPos)) continue;
                
                float falloff = GetBrushFalloffFactor(position, blockPos);
                if (falloff <= 0.0f) continue;
                
                // Count solid neighbors
                int solidCount = 0;
                int totalNeighbors = 0;
                BlockTypeID currentType = world->GetBlock(cx + dx, cy + dy, cz + dz);
                
                for (int nx = -1; nx <= 1; ++nx) {
                    for (int ny = -1; ny <= 1; ++ny) {
                        for (int nz = -1; nz <= 1; ++nz) {
                            if (nx == 0 && ny == 0 && nz == 0) continue;
                            BlockTypeID neighbor = world->GetBlock(cx + dx + nx, cy + dy + ny, cz + dz + nz);
                            if (neighbor != BLOCK_AIR) solidCount++;
                            totalNeighbors++;
                        }
                    }
                }
                
                float solidRatio = (float)solidCount / totalNeighbors;
                bool shouldBeSolid = solidRatio > 0.5f;
                
                if ((shouldBeSolid && currentType == BLOCK_AIR) || 
                    (!shouldBeSolid && currentType != BLOCK_AIR)) {
                    BlockTypeID newType = shouldBeSolid ? selectedBlock : BLOCK_AIR;
                    if (newType != currentType) {
                        VoxelEdit edit;
                        edit.position = blockPos;
                        edit.oldType = currentType;
                        edit.newType = newType;
                        edits.push_back(edit);
                        world->SetBlock(cx + dx, cy + dy, cz + dz, newType);
                    }
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return !edits.empty();
}

bool VoxelEditor::RaiseTerrain(const Vec3& position) {
    if (!world) return false;
    
    int cx = (int)std::floor(position.x);
    int cy = (int)std::floor(position.y);
    int cz = (int)std::floor(position.z);
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            Vec3 blockPos((float)(cx + dx), (float)cy, (float)(cz + dz));
            if (!IsInBrushRadius(position, blockPos)) continue;
            
            float falloff = GetBrushFalloffFactor(position, blockPos);
            if (falloff <= 0.0f) continue;
            
            // Find highest solid block in column
            int topY = cy;
            for (int y = CHUNK_SIZE - 1; y >= 0; --y) {
                if (world->GetBlock(cx + dx, y, cz + dz) != BLOCK_AIR) {
                    topY = y;
                    break;
                }
            }
            
            // Add blocks on top
            int addCount = (int)(brush.strength * falloff * 2.0f);
            for (int i = 1; i <= addCount; ++i) {
                int placeY = topY + i;
                BlockTypeID current = world->GetBlock(cx + dx, placeY, cz + dz);
                if (current == BLOCK_AIR) {
                    VoxelEdit edit;
                    edit.position = Vec3((float)(cx + dx), (float)placeY, (float)(cz + dz));
                    edit.oldType = BLOCK_AIR;
                    edit.newType = selectedBlock;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, placeY, cz + dz, selectedBlock);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return !edits.empty();
}

bool VoxelEditor::LowerTerrain(const Vec3& position) {
    if (!world) return false;
    
    int cx = (int)std::floor(position.x);
    int cy = (int)std::floor(position.y);
    int cz = (int)std::floor(position.z);
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            Vec3 blockPos((float)(cx + dx), (float)cy, (float)(cz + dz));
            if (!IsInBrushRadius(position, blockPos)) continue;
            
            float falloff = GetBrushFalloffFactor(position, blockPos);
            if (falloff <= 0.0f) continue;
            
            // Find highest solid block in column
            int topY = cy;
            for (int y = CHUNK_SIZE - 1; y >= 0; --y) {
                if (world->GetBlock(cx + dx, y, cz + dz) != BLOCK_AIR) {
                    topY = y;
                    break;
                }
            }
            
            // Remove blocks from top
            int removeCount = (int)(brush.strength * falloff * 2.0f);
            for (int i = 0; i < removeCount && (topY - i) >= 0; ++i) {
                int removeY = topY - i;
                BlockTypeID current = world->GetBlock(cx + dx, removeY, cz + dz);
                if (current != BLOCK_AIR) {
                    VoxelEdit edit;
                    edit.position = Vec3((float)(cx + dx), (float)removeY, (float)(cz + dz));
                    edit.oldType = current;
                    edit.newType = BLOCK_AIR;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, removeY, cz + dz, BLOCK_AIR);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return !edits.empty();
}

bool VoxelEditor::FlattenTerrain(const Vec3& position, float targetHeight) {
    if (!world) return false;
    
    int cx = (int)std::floor(position.x);
    int cy = (int)std::floor(position.y);
    int cz = (int)std::floor(position.z);
    int targetY = (int)std::floor(targetHeight);
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            Vec3 blockPos((float)(cx + dx), (float)cy, (float)(cz + dz));
            if (!IsInBrushRadius(position, blockPos)) continue;
            
            float falloff = GetBrushFalloffFactor(position, blockPos);
            if (falloff <= 0.0f) continue;
            
            // Fill up to target height
            for (int y = 0; y <= targetY; ++y) {
                BlockTypeID current = world->GetBlock(cx + dx, y, cz + dz);
                if (current == BLOCK_AIR) {
                    VoxelEdit edit;
                    edit.position = Vec3((float)(cx + dx), (float)y, (float)(cz + dz));
                    edit.oldType = BLOCK_AIR;
                    edit.newType = selectedBlock;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, y, cz + dz, selectedBlock);
                }
            }
            
            // Remove above target height
            for (int y = targetY + 1; y < CHUNK_SIZE; ++y) {
                BlockTypeID current = world->GetBlock(cx + dx, y, cz + dz);
                if (current != BLOCK_AIR) {
                    VoxelEdit edit;
                    edit.position = Vec3((float)(cx + dx), (float)y, (float)(cz + dz));
                    edit.oldType = current;
                    edit.newType = BLOCK_AIR;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, y, cz + dz, BLOCK_AIR);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (onTerrainModified) onTerrainModified();
    }
    return !edits.empty();
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
    if (!world) return {};
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    int cx = (int)std::floor(center.x);
    int cy = (int)std::floor(center.y);
    int cz = (int)std::floor(center.z);
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dz = -radius; dz <= radius; ++dz) {
                Vec3 blockPos((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                if (!IsInBrushRadius(center, blockPos)) continue;
                
                float falloff = GetBrushFalloffFactor(center, blockPos);
                if (falloff <= 0.0f) continue;
                
                BlockTypeID current = world->GetBlock(cx + dx, cy + dy, cz + dz);
                bool isAir = (current == BLOCK_AIR);
                
                // For remove tool, only remove solid blocks
                if (currentTool == VoxelToolType::Remove && isAir) continue;
                // For place tool, respect affectAir setting
                if (currentTool == VoxelToolType::Place && !isAir && !brush.affectAir) continue;
                
                BlockTypeID newType = (currentTool == VoxelToolType::Remove) ? BLOCK_AIR : type;
                if (newType != current) {
                    VoxelEdit edit;
                    edit.position = blockPos;
                    edit.oldType = current;
                    edit.newType = newType;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, cy + dy, cz + dz, newType);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
        if (currentTool == VoxelToolType::Place && onBlockPlaced) {
            for (const auto& edit : edits) {
                onBlockPlaced(edit.position, edit.newType);
            }
        } else if (currentTool == VoxelToolType::Remove && onBlockRemoved) {
            for (const auto& edit : edits) {
                onBlockRemoved(edit.position, edit.oldType);
            }
        }
    }
    return edits;
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrushBox(const Vec3& center, BlockTypeID type) {
    if (!world) return {};
    
    std::vector<VoxelEdit> edits;
    int cx = (int)std::floor(center.x);
    int cy = (int)std::floor(center.y);
    int cz = (int)std::floor(center.z);
    
    int hx = (int)std::floor(brush.size.x * 0.5f);
    int hy = (int)std::floor(brush.size.y * 0.5f);
    int hz = (int)std::floor(brush.size.z * 0.5f);
    
    for (int dx = -hx; dx <= hx; ++dx) {
        for (int dy = -hy; dy <= hy; ++dy) {
            for (int dz = -hz; dz <= hz; ++dz) {
                BlockTypeID current = world->GetBlock(cx + dx, cy + dy, cz + dz);
                bool isAir = (current == BLOCK_AIR);
                
                if (currentTool == VoxelToolType::Remove && isAir) continue;
                if (currentTool == VoxelToolType::Place && !isAir && !brush.affectAir) continue;
                
                BlockTypeID newType = (currentTool == VoxelToolType::Remove) ? BLOCK_AIR : type;
                if (newType != current) {
                    VoxelEdit edit;
                    edit.position = Vec3((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                    edit.oldType = current;
                    edit.newType = newType;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, cy + dy, cz + dz, newType);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
    }
    return edits;
}

std::vector<VoxelEdit> VoxelEditor::ApplyBrushCylinder(const Vec3& center, BlockTypeID type) {
    if (!world) return {};
    
    std::vector<VoxelEdit> edits;
    int radius = (int)std::ceil(brush.radius);
    int cx = (int)std::floor(center.x);
    int cy = (int)std::floor(center.y);
    int cz = (int)std::floor(center.z);
    int height = (int)std::ceil(brush.size.y);
    int halfHeight = height / 2;
    
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            for (int dy = -halfHeight; dy <= halfHeight; ++dy) {
                Vec3 blockPos((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                if (!IsInBrushRadius(center, blockPos)) continue;
                
                float falloff = GetBrushFalloffFactor(center, blockPos);
                if (falloff <= 0.0f) continue;
                
                BlockTypeID current = world->GetBlock(cx + dx, cy + dy, cz + dz);
                bool isAir = (current == BLOCK_AIR);
                
                if (currentTool == VoxelToolType::Remove && isAir) continue;
                if (currentTool == VoxelToolType::Place && !isAir && !brush.affectAir) continue;
                
                BlockTypeID newType = (currentTool == VoxelToolType::Remove) ? BLOCK_AIR : type;
                if (newType != current) {
                    VoxelEdit edit;
                    edit.position = blockPos;
                    edit.oldType = current;
                    edit.newType = newType;
                    edits.push_back(edit);
                    world->SetBlock(cx + dx, cy + dy, cz + dz, newType);
                }
            }
        }
    }
    
    if (!edits.empty()) {
        history.PushEdit(edits);
    }
    return edits;
}

bool VoxelEditor::Undo() {
    if (!history.CanUndo()) return false;
    
    std::vector<VoxelEdit> edits = history.Undo();
    ApplyEdits(edits, true);
    return true;
}

bool VoxelEditor::Redo() {
    if (!history.CanRedo()) return false;
    
    std::vector<VoxelEdit> edits = history.Redo();
    ApplyEdits(edits, false);
    return true;
}

void VoxelEditor::ApplyEdits(const std::vector<VoxelEdit>& edits, bool reverse) {
    if (!world) return;
    
    for (const auto& edit : edits) {
        int x = (int)std::floor(edit.position.x);
        int y = (int)std::floor(edit.position.y);
        int z = (int)std::floor(edit.position.z);
        
        BlockTypeID type = reverse ? edit.oldType : edit.newType;
        world->SetBlock(x, y, z, type);
    }
}

std::vector<Vec3> VoxelEditor::GetAffectedBlocks(const Vec3& center) const {
    std::vector<Vec3> blocks;
    
    switch (brush.shape) {
        case BrushShape::Sphere: {
            int radius = (int)std::ceil(brush.radius);
            int cx = (int)std::floor(center.x);
            int cy = (int)std::floor(center.y);
            int cz = (int)std::floor(center.z);
            
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dy = -radius; dy <= radius; ++dy) {
                    for (int dz = -radius; dz <= radius; ++dz) {
                        Vec3 blockPos((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                        if (IsInBrushRadius(center, blockPos)) {
                            blocks.push_back(blockPos);
                        }
                    }
                }
            }
            break;
        }
        case BrushShape::Box: {
            int cx = (int)std::floor(center.x);
            int cy = (int)std::floor(center.y);
            int cz = (int)std::floor(center.z);
            int hx = (int)std::floor(brush.size.x * 0.5f);
            int hy = (int)std::floor(brush.size.y * 0.5f);
            int hz = (int)std::floor(brush.size.z * 0.5f);
            
            for (int dx = -hx; dx <= hx; ++dx) {
                for (int dy = -hy; dy <= hy; ++dy) {
                    for (int dz = -hz; dz <= hz; ++dz) {
                        blocks.push_back(Vec3((float)(cx + dx), (float)(cy + dy), (float)(cz + dz)));
                    }
                }
            }
            break;
        }
        case BrushShape::Cylinder: {
            int radius = (int)std::ceil(brush.radius);
            int cx = (int)std::floor(center.x);
            int cy = (int)std::floor(center.y);
            int cz = (int)std::floor(center.z);
            int height = (int)std::ceil(brush.size.y);
            int halfHeight = height / 2;
            
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    for (int dy = -halfHeight; dy <= halfHeight; ++dy) {
                        Vec3 blockPos((float)(cx + dx), (float)(cy + dy), (float)(cz + dz));
                        if (IsInBrushRadius(center, blockPos)) {
                            blocks.push_back(blockPos);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    
    return blocks;
}

bool VoxelEditor::IsInBrushRadius(const Vec3& center, const Vec3& block) const {
    switch (brush.shape) {
        case BrushShape::Sphere: {
            float dx = block.x - center.x;
            float dy = block.y - center.y;
            float dz = block.z - center.z;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            return dist <= brush.radius;
        }
        case BrushShape::Box: {
            float dx = std::abs(block.x - center.x);
            float dy = std::abs(block.y - center.y);
            float dz = std::abs(block.z - center.z);
            return dx <= brush.size.x * 0.5f && dy <= brush.size.y * 0.5f && dz <= brush.size.z * 0.5f;
        }
        case BrushShape::Cylinder: {
            float dx = block.x - center.x;
            float dz = block.z - center.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            float dy = std::abs(block.y - center.y);
            return dist <= brush.radius && dy <= brush.size.y * 0.5f;
        }
        default:
            return false;
    }
}

float VoxelEditor::GetBrushFalloffFactor(const Vec3& center, const Vec3& block) const {
    if (brush.falloff <= 0.0f) {
        return IsInBrushRadius(center, block) ? 1.0f : 0.0f;
    }
    
    float dx = block.x - center.x;
    float dy = block.y - center.y;
    float dz = block.z - center.z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    if (dist > brush.radius) return 0.0f;
    if (dist <= brush.radius * (1.0f - brush.falloff)) return 1.0f;
    
    float normalizedDist = (dist - brush.radius * (1.0f - brush.falloff)) / (brush.radius * brush.falloff);
    return 1.0f - normalizedDist;
}

bool VoxelEditor::CanPlaceAt(const Vec3& position) const {
    if (!world) return false;
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    return world->GetBlock(x, y, z) == BLOCK_AIR;
}

bool VoxelEditor::CanRemoveAt(const Vec3& position) const {
    if (!world) return false;
    int x = (int)std::floor(position.x);
    int y = (int)std::floor(position.y);
    int z = (int)std::floor(position.z);
    BlockTypeID block = world->GetBlock(x, y, z);
    return block != BLOCK_AIR && BlockRegistry::GetInstance().GetBlock(block).IsSolid();
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

std::string VoxelEditor::SerializeHistory() const {
    // Simple serialization - just return count for now
    return "history_count=" + std::to_string(history.GetUndoCount());
}

bool VoxelEditor::DeserializeHistory(const std::string& data) {
    (void)data;
    return false; // Not fully implemented
}

// ============================================
// TerrainPainter Implementation
// ============================================

TerrainPainter::TerrainPainter(World* world, int seed) 
    : world(world), seed(seed) {}

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
    if (!world || layers.empty()) return;
    
    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                int worldX = chunkX * CHUNK_SIZE + x;
                int worldY = chunkY * CHUNK_SIZE + y;
                int worldZ = chunkZ * CHUNK_SIZE + z;
                
                int layerIndex = SelectLayerForBlock(worldX, worldY, worldZ);
                if (layerIndex >= 0 && layerIndex < (int)layers.size()) {
                    const PaintLayer& layer = layers[layerIndex];
                    BlockTypeID current = chunk->GetBlock(x, y, z);
                    if (current != BLOCK_AIR) {
                        chunk->SetBlock(x, y, z, layer.blockType);
                    }
                }
            }
        }
    }
    chunk->SetDirty(true);
}

void TerrainPainter::PaintRegion(int startX, int startY, int startZ, 
                                   int width, int height, int depth) {
    for (int x = startX; x < startX + width; ++x) {
        for (int y = startY; y < startY + height; ++y) {
            for (int z = startZ; z < startZ + depth; ++z) {
                PaintChunk(x, y, z);
            }
        }
    }
}

void TerrainPainter::PaintWorld() {
    if (!world) return;
    // This would iterate all loaded chunks - simplified version
}

void TerrainPainter::PaintBySlope(int chunkX, int chunkY, int chunkZ) {
    if (!world) return;
    
    Chunk* chunk = world->GetChunk(chunkX, chunkY, chunkZ);
    if (!chunk) return;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                int worldX = chunkX * CHUNK_SIZE + x;
                int worldY = chunkY * CHUNK_SIZE + y;
                int worldZ = chunkZ * CHUNK_SIZE + z;
                
                float slope = CalculateSlope(worldX, worldY, worldZ);
                BlockTypeID current = chunk->GetBlock(x, y, z);
                
                if (current != BLOCK_AIR) {
                    for (const auto& layer : layers) {
                        if (slope >= layer.slopeMin && slope <= layer.slopeMax) {
                            chunk->SetBlock(x, y, z, layer.blockType);
                            break;
                        }
                    }
                }
            }
        }
