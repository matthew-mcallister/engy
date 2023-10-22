/// Scene-related math.

#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "math/matrix.h"
#include "math/vector.h"

namespace scene {

/// @brief Computes a perspective projection matrix.
Matrix4 projection(float fovy, float aspect, float z_near, float z_far);

/// @brief Constructs a 3-dimensional basis that points in the given
/// unit direction.
///
/// +z points forward, +x points right, and +y points down.
Matrix4 looking_basis(Vector3 direction);

/// @brief Constructs a transformation for a camera located at pos and
/// looking at target.
Matrix4 targeting_camera_xform(Vector3 pos, Vector3 target);

/// @brief Constructs a transformation for a camera located at pos and
/// looking in direction.
Matrix4 looking_camera_xform(Vector3 pos, Vector3 direction);

/// @brief Constructs an Euler rotation matrix using the pilot's angle
/// convention.
Matrix4 pilot_angles_xform(float yaw, float pitch, float roll);

} // namespace scene

#endif
