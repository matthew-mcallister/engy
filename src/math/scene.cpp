#include <cassert>
#include <cmath>

#include "config.h"
#include "math/matrix.h"
#include "math/scene.h"
#include "math/vector.h"

namespace scene {

// XXX: Is it faster to recompute the vertex position from gl_FragCoord
// or to pass it directly as an input to the fragment shader?
Matrix4 projection(float fovy, float aspect, float z_near, float z_far) {
    float dz = z_far - z_near;
    float tanfovy = tan(fovy);
    float tanfovx = aspect * tanfovy;
    return Matrix4{{1 / tanfovx, 0, 0, 0},
                   {0, 1 / tanfovy, 0, 0},
                   {0, 0, -z_near / dz, 1},
                   {0, 0, z_far * z_near / dz, 0}};
}

Matrix4 looking_basis(Vector3 direction) {
    Vector4 z = direction;
    Vector4 x, y;
    if (abs(z.z()) > 0.99) {
        // Looking straight up or down; use fallback value
        x = Vector4{1, 0, 0, 0};
        y = z.cross(x);
    } else {
        Vector4 down{0, 0, -1, 0};
        y = (down - down.dot(z) * z).normalized();
        x = y.cross(z);
    }
    return {x, y, z, Vector4()};
}

Matrix4 targeting_camera_xform(Vector3 pos, Vector3 target) {
    Vector3 direction = (target - pos).normalized();
    return looking_camera_xform(pos, direction);
}

Matrix4 looking_camera_xform(Vector3 pos, Vector3 direction) {
    return looking_basis(direction).offset(pos).rigid_inverse();
}

Matrix4 pilot_angles_xform(float yaw, float pitch, float roll) {
    // clang-format off
    Matrix4 rot1{{1, 0,           0,          0},
                 {0, cosf(roll), -sinf(roll), 0},
                 {0, sinf(roll),  cosf(roll), 0},
                 {0, 0,           0,          1}};
    Matrix4 rot2{{ cosf(pitch), 0, -sinf(pitch), 0},
                 { 0,           1,  0,           0},
                 { sinf(pitch), 0,  cosf(pitch), 0},
                 { 0,           0,  0,           1}};
    Matrix4 rot3{{ cosf(yaw), sinf(yaw), 0, 0},
                 {-sinf(yaw), cosf(yaw), 0, 0},
                 { 0,         0,         1, 0},
                 { 0,         0,         0, 1}};
    return rot3 * rot2 * rot1;
    // clang-format on
}

} // namespace scene
