#include "math.h"
#include <cstring>
namespace aether {
Mat4 Mat4::identity() { Mat4 r; std::memset(r.m, 0, sizeof(r.m)); r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f; return r; }
Mat4 Mat4::translate(const Vec3& t) { Mat4 r = identity(); r.m[12]=t.x; r.m[13]=t.y; r.m[14]=t.z; return r; }
Mat4 Mat4::scale(const Vec3& s) { Mat4 r = identity(); r.m[0]=s.x; r.m[5]=s.y; r.m[10]=s.z; return r; }
Mat4 Mat4::perspective(float fov, float aspect, float near, float far) {
    Mat4 r; std::memset(r.m, 0, sizeof(r.m));
    float tanHalfFov = std::tan(fov * 0.5f);
    r.m[0] = 1.0f / (aspect * tanHalfFov);
    r.m[5] = 1.0f / tanHalfFov;
    r.m[10] = -(far + near) / (far - near);
    r.m[11] = -1.0f;
    r.m[14] = -(2.0f * far * near) / (far - near);
    return r;
}
Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye); float fl = f.length(); if (fl > 0) f = f * (1.0f/fl);
    Vec3 s = f.cross(up); float sl = s.length(); if (sl > 0) s = s * (1.0f/sl);
    Vec3 u = s.cross(f);
    Mat4 r = identity();
    r.m[0]=s.x; r.m[4]=s.y; r.m[8]=s.z;
    r.m[1]=u.x; r.m[5]=u.y; r.m[9]=u.z;
    r.m[2]=-f.x; r.m[6]=-f.y; r.m[10]=-f.z;
    r.m[12]=-s.dot(eye); r.m[13]=-u.dot(eye); r.m[14]=f.dot(eye);
    return r;
}
Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 r;
    for (int i=0; i<4; ++i)
        for (int j=0; j<4; ++j)
            r.m[i*4+j] = m[i*4+0]*o.m[0*4+j] + m[i*4+1]*o.m[1*4+j] + m[i*4+2]*o.m[2*4+j] + m[i*4+3]*o.m[3*4+j];
    return r;
}
} // namespace aether
