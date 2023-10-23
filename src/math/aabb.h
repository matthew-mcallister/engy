#ifndef AABB_H_INCLUDED
#define AABB_H_INCLUDED

#include "math/vector.h"

struct IntersectionResult {
    float t_enter;
    float t_leave;
    bool intersects;
};

struct AABB3 {
    Vector3 min, max;

    AABB3() = default;
    AABB3(Vector3 min, Vector3 max) : min{min}, max{max} {}

    bool is_empty() const {
        return max.x() < min.x() || max.y() < min.y() || max.z() < min.z();
    }

    /// @brief Returns the intersection of two AABBs. If the AABBs do
    /// not intersect, the output will be an empty AABB.
    AABB3 intersect(AABB3 other) {
        return {min.max(other.min), max.min(other.max)};
    }

    AABB3 join(AABB3 other) { return {min.min(other.min), max.max(other.max)}; }

    bool contains(Vector3 point) { return point.ge3(min) && point.le3(max); }

    auto intersect_ray(Vector3 origin, Vector3 direction) const
        -> IntersectionResult;
};

#endif