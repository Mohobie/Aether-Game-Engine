# Math System

The engine uses a custom math library with vectors, matrices, and quaternions.

## Vec3 - 3D Vector

```cpp
#include "math/vec3.h"

// Creation
vge::Vec3 pos(1.0f, 2.0f, 3.0f);
vge::Vec3 origin = vge::Vec3::Zero;
vge::Vec3 up = vge::Vec3::Up;  // (0, 1, 0)

// Operations
vge::Vec3 a(1, 2, 3);
vge::Vec3 b(4, 5, 6);

vge::Vec3 sum = a + b;        // (5, 7, 9)
vge::Vec3 diff = a - b;       // (-3, -3, -3)
vge::Vec3 scaled = a * 2.0f;  // (2, 4, 6)
float dot = a.Dot(b);         // 32.0f
vge::Vec3 cross = a.Cross(b); // (-3, 6, -3)

// Length and normalization
float len = a.Length();       // 3.741...
vge::Vec3 norm = a.Normalized(); // Unit vector

// Distance
float dist = vge::Vec3::Distance(a, b);

// Lerp (linear interpolation)
vge::Vec3 mid = vge::Vec3::Lerp(a, b, 0.5f);  // (2.5, 3.5, 4.5)
```

## Mat4 - 4x4 Matrix

Used for transformations (translation, rotation, scale, projection).

```cpp
#include "math/mat4.h"

// Identity matrix
vge::Mat4 identity = vge::Mat4::Identity;

// Translation
vge::Mat4 translate = vge::Mat4::Translation(10.0f, 5.0f, 0.0f);

// Rotation (degrees)
vge::Mat4 rotateX = vge::Mat4::RotationX(45.0f);
vge::Mat4 rotateY = vge::Mat4::RotationY(90.0f);
vge::Mat4 rotateZ = vge::Mat4::RotationZ(30.0f);

// Scale
vge::Mat4 scale = vge::Mat4::Scale(2.0f, 2.0f, 2.0f);

// Combine transforms (order matters: SRT = Scale * Rotate * Translate)
vge::Mat4 transform = scale * rotateY * translate;

// Transform a point
vge::Vec3 point(1, 0, 0);
vge::Vec3 transformed = transform.TransformPoint(point);

// Transform a direction (no translation)
vge::Vec3 direction = transform.TransformDirection(point);

// Perspective projection
vge::Mat4 proj = vge::Mat4::Perspective(
    60.0f,      // FOV in degrees
    16.0f/9.0f, // Aspect ratio
    0.1f,       // Near plane
    1000.0f     // Far plane
);

// LookAt (camera view matrix)
vge::Mat4 view = vge::Mat4::LookAt(
    vge::Vec3(0, 5, 10),   // Camera position
    vge::Vec3(0, 0, 0),    // Look at target
    vge::Vec3(0, 1, 0)     // Up vector
);

// View-Projection matrix
vge::Mat4 viewProj = view * proj;

// Inverse
vge::Mat4 inverse = transform.Inverse();

// Transpose
vge::Mat4 transposed = transform.Transpose();
```

## Common Patterns

### Camera Transform
```cpp
class Camera {
    vge::Vec3 position;
    float yaw = 0.0f;    // Horizontal rotation
    float pitch = 0.0f;  // Vertical rotation
    
    vge::Mat4 GetViewMatrix() {
        // Calculate forward direction from yaw/pitch
        vge::Vec3 forward;
        forward.x = cos(pitch) * sin(yaw);
        forward.y = sin(pitch);
        forward.z = -cos(pitch) * cos(yaw);
        
        return vge::Mat4::LookAt(position, position + forward, vge::Vec3::Up);
    }
};
```

### Ray from Screen Point
```cpp
// Convert screen coordinates to world ray
Ray ScreenToRay(int screenX, int screenY, int screenWidth, int screenHeight,
                const vge::Mat4& view, const vge::Mat4& proj) {
    // Normalized device coordinates (-1 to 1)
    float x = (2.0f * screenX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenY) / screenHeight;
    
    // Unproject near and far points
    vge::Mat4 invVP = (view * proj).Inverse();
    vge::Vec3 nearPoint = invVP.TransformPoint(vge::Vec3(x, y, -1.0f));
    vge::Vec3 farPoint = invVP.TransformPoint(vge::Vec3(x, y, 1.0f));
    
    vge::Vec3 direction = (farPoint - nearPoint).Normalized();
    return Ray(nearPoint, direction);
}
```

## AABB (Axis-Aligned Bounding Box)

```cpp
#include "rendering/frustum.h"  // Contains AABB

// Create AABB
vge::AABB box(vge::Vec3(-1, -1, -1), vge::Vec3(1, 1, 1));

// Check if point is inside
bool inside = box.Contains(vge::Vec3(0, 0, 0));  // true

// Get center and size
vge::Vec3 center = box.GetCenter();
vge::Vec3 size = box.GetSize();

// Expand to include point
box.Expand(vge::Vec3(5, 5, 5));

// Check intersection with another AABB
vge::AABB other(vge::Vec3(0, 0, 0), vge::Vec3(2, 2, 2));
bool intersects = box.Intersects(other);
```
