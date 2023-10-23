#include <algorithm>

#include "math/aabb.h"

auto AABB3::intersect_ray(Vector3 origin, Vector3 direction) const
    -> IntersectionResult {
    // Explanation: View the AABB as the intersection of intervals
    // aligned to each coordinate axis.
    //              |  |       |  |
    // --------     |  |     --+--+--
    //              |  |       |  |
    // --------     |  |     --+--+--
    //              |  |       |  |
    //     x          y        aabb
    // Then calculate the time the ray enters/leaves each interval.
    // If it is simultaneously in all 3 intervals, it intersects.
    //
    // FIXME: Correctly handle division by 0
    auto u0 = (min - origin) / direction;
    auto u1 = (max - origin) / direction;
    auto v0 = u0.min(u1);
    auto v1 = u0.max(u1);
    auto t_enter = std::max({v0[0], v0[1], v0[2]});
    auto t_leave = std::min({v1[0], v1[1], v1[2]});
    return {
        t_enter,
        t_leave,
        t_leave > 0 && t_leave >= t_enter,
    };
}
