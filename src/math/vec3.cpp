#include "vec3.h"
#include <cmath>

namespace VoxelEngine {
    Vec3 Vec3::operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 Vec3::operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 Vec3::operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    float Vec3::Length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    Vec3 Vec3::Normalize() const {
        float len = Length();
        if (len > 0) {
            return Vec3(x/len, y/len, z/len);
        }
        return Vec3(0, 0, 0);
    }
    
    float Vec3::Dot(const Vec3& other) const {
        return x*other.x + y*other.y + z*other.z;
    }
    
    Vec3 Vec3::Cross(const Vec3& other) const {
        return Vec3(
            y*other.z - z*other.y,
            z*other.x - x*other.z,
            x*other.y - y*other.x
        );
    }
}
