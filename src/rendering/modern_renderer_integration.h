#pragma once

// Integration layer to replace old Renderer with ModernRenderer
// without breaking existing code

#include "modern_renderer.h"

namespace vge {

// Type alias for gradual migration
using Renderer = ModernRenderer;

} // namespace vge
