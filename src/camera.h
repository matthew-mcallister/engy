#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "math/matrix.h"
#include "math/vector.h"

/// @brief Implements a camera rig with capabilities similar to a 3d
/// workstation: Rotation, zoom, and panning about a focal point.
struct EditorCameraRig {
    typedef int Mode;
    static const Mode PIVOT = 0;
    static const Mode PAN = 1;

    Vector3 focus;
    float log_distance;
    // XXX: Should pilot angles be its own class?
    float yaw;
    float pitch;

    EditorCameraRig() : focus{0, 0, 0, 0}, log_distance{0}, yaw{0}, pitch{0} {}

    Matrix4 transform() const;
    void on_mouse_drag(float dx, float dy, Mode mode);
    void on_mouse_scroll(float dx);
};

#endif
