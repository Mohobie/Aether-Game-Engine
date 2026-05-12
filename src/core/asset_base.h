#pragma once

#include <string>
#include <cstdint>

namespace vge {

// Base asset type enum
enum class AssetType {
    Unknown,
    Texture,
    Model,
    Sound,
    Shader,
    Font,
    Material,
    Count
};

// Asset metadata
struct AssetMetadata {
    std::string id;
    std::string path;
    AssetType type;
    size_t size = 0;
    uint64_t lastModified = 0;
    uint32_t refCount = 0;
    bool loaded = false;
    bool fromPack = false;
};

// Base asset class
class Asset {
public:
    virtual ~Asset() = default;
    AssetMetadata metadata;
};

} // namespace vge
