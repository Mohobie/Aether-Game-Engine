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

} // namespace vge
