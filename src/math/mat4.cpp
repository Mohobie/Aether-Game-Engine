#include "mat4.h"
#include <cstring>
#include <cmath>

namespace vge {

Mat4::Mat4() {
    for(int i=0; i<16; i++) data[i] = 0;
    data[0] = data[5] = data[10] = data[15] = 1.0f;
}

Mat4 Mat4::Identity() {
    Mat4 m;
    for(int i=0; i<16; i++) m.data[i] = 0;
    m.data[0] = m.data[5] = m.data[10] = m.data[15] = 1.0f;
    return m;
}

Mat4 Mat4::Multiply(const Mat4& other) const {
    Mat4 result;
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            result.data[i*4+j] = 0;
            for(int k=0; k<4; k++) {
                result.data[i*4+j] += data[i*4+k] * other.data[k*4+j];
            }
        }
    }
    return result;
}

Mat4 Mat4::Perspective(float fov, float aspect, float near, float far) {
    Mat4 m;
    float tanHalfFov = std::tan(fov / 2.0f);
    m.data[0] = 1.0f / (aspect * tanHalfFov);
    m.data[5] = 1.0f / tanHalfFov;
    m.data[10] = -(far + near) / (far - near);
    m.data[11] = -1.0f;
    m.data[14] = -(2.0f * far * near) / (far - near);
    m.data[15] = 0;
    return m;
}

Mat4 Mat4::LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).normalize();
    Vec3 s = f.cross(up).normalize();
    Vec3 u = s.cross(f);
    
    Mat4 m = Identity();
    m.data[0] = s.x; m.data[4] = s.y; m.data[8] = s.z;
    m.data[1] = u.x; m.data[5] = u.y; m.data[9] = u.z;
    m.data[2] = -f.x; m.data[6] = -f.y; m.data[10] = -f.z;
    m.data[12] = -s.dot(eye);
    m.data[13] = -u.dot(eye);
    m.data[14] = f.dot(eye);
    return m;
}

Mat4 Mat4::Translate(const Vec3& translation) {
    Mat4 m = Identity();
    m.data[12] = translation.x;
    m.data[13] = translation.y;
    m.data[14] = translation.z;
    return m;
}

Mat4 Mat4::Rotate(float angle, const Vec3& axis) {
    Mat4 m = Identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    Vec3 a = axis.normalize();
    
    m.data[0] = c + a.x*a.x*(1-c);
    m.data[1] = a.x*a.y*(1-c) - a.z*s;
    m.data[2] = a.x*a.z*(1-c) + a.y*s;
    m.data[4] = a.y*a.x*(1-c) + a.z*s;
    m.data[5] = c + a.y*a.y*(1-c);
    m.data[6] = a.y*a.z*(1-c) - a.x*s;
    m.data[8] = a.z*a.x*(1-c) - a.y*s;
    m.data[9] = a.z*a.y*(1-c) + a.x*s;
    m.data[10] = c + a.z*a.z*(1-c);
    return m;
}

Mat4 Mat4::Scale(const Vec3& scale) {
    Mat4 m = Identity();
    m.data[0] = scale.x;
    m.data[5] = scale.y;
    m.data[10] = scale.z;
    return m;
}

Vec3 Mat4::TransformPoint(const Vec3& point) const {
    Vec3 result;
    result.x = data[0]*point.x + data[4]*point.y + data[8]*point.z + data[12];
    result.y = data[1]*point.x + data[5]*point.y + data[9]*point.z + data[13];
    result.z = data[2]*point.x + data[6]*point.y + data[10]*point.z + data[14];
    return result;
}

Mat4 Mat4::Transpose() const {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.data[i*4+j] = data[j*4+i];
        }
    }
    return result;
}

Mat4 Mat4::Inverse() const {
    Mat4 inv;
    float invOut[16];
    
    float inv0 = data[5] * data[10] * data[15] - data[5] * data[11] * data[14] - data[9] * data[6] * data[15] + data[9] * data[7] * data[14] + data[13] * data[6] * data[11] - data[13] * data[7] * data[10];
    float inv4 = -data[4] * data[10] * data[15] + data[4] * data[11] * data[14] + data[8] * data[6] * data[15] - data[8] * data[7] * data[14] - data[12] * data[6] * data[11] + data[12] * data[7] * data[10];
    float inv8 = data[4] * data[9] * data[15] - data[4] * data[11] * data[13] - data[8] * data[5] * data[15] + data[8] * data[7] * data[13] + data[12] * data[5] * data[11] - data[12] * data[7] * data[9];
    float inv12 = -data[4] * data[9] * data[14] + data[4] * data[10] * data[13] + data[8] * data[5] * data[14] - data[8] * data[6] * data[13] - data[12] * data[5] * data[10] + data[12] * data[6] * data[9];
    float inv1 = -data[1] * data[10] * data[15] + data[1] * data[11] * data[14] + data[9] * data[2] * data[15] - data[9] * data[3] * data[14] - data[13] * data[2] * data[11] + data[13] * data[3] * data[10];
    float inv5 = data[0] * data[10] * data[15] - data[0] * data[11] * data[14] - data[8] * data[2] * data[15] + data[8] * data[3] * data[14] + data[12] * data[2] * data[11] - data[12] * data[3] * data[10];
    float inv9 = -data[0] * data[9] * data[15] + data[0] * data[11] * data[13] + data[8] * data[1] * data[15] - data[8] * data[3] * data[13] - data[12] * data[1] * data[11] + data[12] * data[3] * data[9];
    float inv13 = data[0] * data[9] * data[14] - data[0] * data[10] * data[13] - data[8] * data[1] * data[14] + data[8] * data[2] * data[13] + data[12] * data[1] * data[10] - data[12] * data[2] * data[9];
    float inv2 = data[1] * data[6] * data[15] - data[1] * data[7] * data[14] - data[5] * data[2] * data[15] + data[5] * data[3] * data[14] + data[13] * data[2] * data[7] - data[13] * data[3] * data[6];
    float inv6 = -data[0] * data[6] * data[15] + data[0] * data[7] * data[14] + data[4] * data[2] * data[15] - data[4] * data[3] * data[14] - data[12] * data[2] * data[7] + data[12] * data[3] * data[6];
    float inv10 = data[0] * data[5] * data[15] - data[0] * data[7] * data[13] - data[4] * data[1] * data[15] + data[4] * data[3] * data[13] + data[12] * data[1] * data[7] - data[12] * data[3] * data[5];
    float inv14 = -data[0] * data[5] * data[14] + data[0] * data[6] * data[13] + data[4] * data[1] * data[14] - data[4] * data[2] * data[13] - data[12] * data[1] * data[6] + data[12] * data[2] * data[5];
    float inv3 = -data[1] * data[6] * data[11] + data[1] * data[7] * data[10] + data[5] * data[2] * data[11] - data[5] * data[3] * data[10] - data[9] * data[2] * data[7] + data[9] * data[3] * data[6];
    float inv7 = data[0] * data[6] * data[11] - data[0] * data[7] * data[10] - data[4] * data[2] * data[11] + data[4] * data[3] * data[10] + data[8] * data[2] * data[7] - data[8] * data[3] * data[6];
    float inv11 = -data[0] * data[5] * data[11] + data[0] * data[7] * data[9] + data[4] * data[1] * data[11] - data[4] * data[3] * data[9] - data[8] * data[1] * data[7] + data[8] * data[3] * data[5];
    float inv15 = data[0] * data[5] * data[10] - data[0] * data[6] * data[9] - data[4] * data[1] * data[10] + data[4] * data[2] * data[9] + data[8] * data[1] * data[6] - data[8] * data[2] * data[5];
    
    float det = data[0] * inv0 + data[1] * inv4 + data[2] * inv8 + data[3] * inv12;
    if (det == 0) return Identity();
    
    det = 1.0f / det;
    
    invOut[0] = inv0 * det; invOut[1] = inv1 * det; invOut[2] = inv2 * det; invOut[3] = inv3 * det;
    invOut[4] = inv4 * det; invOut[5] = inv5 * det; invOut[6] = inv6 * det; invOut[7] = inv7 * det;
    invOut[8] = inv8 * det; invOut[9] = inv9 * det; invOut[10] = inv10 * det; invOut[11] = inv11 * det;
    invOut[12] = inv12 * det; invOut[13] = inv13 * det; invOut[14] = inv14 * det; invOut[15] = inv15 * det;
    
    for (int i = 0; i < 16; ++i) inv.data[i] = invOut[i];
    return inv;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    return Multiply(other);
}

Vec3 Mat4::operator*(const Vec3& vec) const {
    return TransformPoint(vec);
}

} // namespace vge
