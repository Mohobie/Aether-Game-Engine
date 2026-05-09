#pragma once

namespace vge {
    struct Mat4 {
        float data[16];
        Mat4() {
            for(int i=0; i<16; i++) data[i] = 0;
            data[0] = data[5] = data[10] = data[15] = 1.0f;
        }
    };
}
