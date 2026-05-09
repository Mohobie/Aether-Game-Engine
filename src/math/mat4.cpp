#include "mat4.h"
#include <cstring>
#include <cmath>

namespace vge {
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
        Vec3 f = (center - eye).Normalize();
        Vec3 s = f.Cross(up).Normalize();
        Vec3 u = s.Cross(f);
        
        Mat4 m = Identity();
        m.data[0] = s.x; m.data[4] = s.y; m.data[8] = s.z;
        m.data[1] = u.x; m.data[5] = u.y; m.data[9] = u.z;
        m.data[2] = -f.x; m.data[6] = -f.y; m.data[10] = -f.z;
        m.data[12] = -s.Dot(eye);
        m.data[13] = -u.Dot(eye);
        m.data[14] = f.Dot(eye);
        return m;
    }
}
