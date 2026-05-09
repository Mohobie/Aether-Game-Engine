#include "collision.h"
#include "math/vec3.h"
#include <cmath>

namespace vge {

bool AABB::Intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

// Improved ray-AABB intersection using slab method
bool Physics::RayCast(const Vec3& origin, const Vec3& direction, float maxDist, Vec3& hit) {
    Vec3 dir = direction.normalize();
    
    // AABB at origin for testing (would be passed in real implementation)
    AABB box(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f));
    
    float tmin = 0.0f;
    float tmax = maxDist;
    
    // Check each axis
    for (int i = 0; i < 3; i++) {
        float invD = 1.0f / (i == 0 ? dir.x : (i == 1 ? dir.y : dir.z));
        float t0 = ((i == 0 ? box.min.x : (i == 1 ? box.min.y : box.min.z)) - 
                    (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z))) * invD;
        float t1 = ((i == 0 ? box.max.x : (i == 1 ? box.max.y : box.max.z)) - 
                    (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z))) * invD;
        
        if (invD < 0.0f) {
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }
        
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        
        if (tmax <= tmin) {
            return false;
        }
    }
    
    hit = origin + dir * tmin;
    return true;
}

// Ray-AABB intersection for specific box
bool Physics::RayCastAABB(const Vec3& origin, const Vec3& direction, const AABB& box, float maxDist, Vec3& hit, float& outDist) {
    Vec3 dir = direction.normalize();
    
    float tmin = 0.0f;
    float tmax = maxDist;
    
    // Check each axis
    for (int i = 0; i < 3; i++) {
        float d = (i == 0 ? dir.x : (i == 1 ? dir.y : dir.z));
        if (std::abs(d) < 0.0001f) {
            // Ray is parallel to slab. No hit if origin not within slab
            float o = (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z));
            float min = (i == 0 ? box.min.x : (i == 1 ? box.min.y : box.min.z));
            float max = (i == 0 ? box.max.x : (i == 1 ? box.max.y : box.max.z));
            if (o < min || o > max) {
                return false;
            }
        } else {
            float invD = 1.0f / d;
            float t0 = ((i == 0 ? box.min.x : (i == 1 ? box.min.y : box.min.z)) - 
                        (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z))) * invD;
            float t1 = ((i == 0 ? box.max.x : (i == 1 ? box.max.y : box.max.z)) - 
                        (i == 0 ? origin.x : (i == 1 ? origin.y : origin.z))) * invD;
            
            if (invD < 0.0f) {
                float temp = t0;
                t0 = t1;
                t1 = temp;
            }
            
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            
            if (tmax <= tmin) {
                return false;
            }
        }
    }
    
    outDist = tmin;
    hit = origin + dir * tmin;
    return true;
}

} // namespace vge