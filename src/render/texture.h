#pragma once
#include "core/types.h"
#include <string>
namespace vge {
class Texture2D {
public:
    Texture2D();
    bool loadFromFile(const std::string& path);
    void bind(uint32_t slot = 0) const;
private:
    uint32_t id = 0;
};
} // namespace vge
