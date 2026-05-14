#include "vec3.h"
#include <cmath>

namespace vge {

Vec3::Vec3() : x(0), y(0), z(0) {}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 Vec3::operator/(float scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
}

Vec3 Vec3::operator*(const Vec3& other) const {
    return Vec3(x * other.x, y * other.y, z * other.z);
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x; y += other.y; z += other.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& other) {
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar; y *= scalar; z *= scalar;
    return *this;
}

Vec3& Vec3::operator/=(float scalar) {
    x /= scalar; y /= scalar; z /= scalar;
    return *this;
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vec3::lengthSquared() const {
    return x * x + y * y + z * z;
}

Vec3 Vec3::normalize() const {
    float len = length();
    if (len > 0.0f) {
        return Vec3(x / len, y / len, z / len);
    }
    return Vec3(0.0f, 0.0f, 0.0f);
}

} // namespace vge
