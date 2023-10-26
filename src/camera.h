#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "math/matrix.h"
#include "math/vector.h"

struct CameraRig {
    virtual Matrix4 forward_transform() const = 0;
    virtual Matrix4 reverse_transform() const {
        return forward_transform().rigid_inverse();
    }
    virtual void on_mouse_move(float dx, float dy) {}
    virtual void on_mouse_drag(float dx, float dy, const uint8_t *keystate) {}
    virtual void on_mouse_scroll(float dx) {}
    virtual void ticker(const uint8_t *keystate) {}
};

/// @brief Implements a camera rig with capabilities similar to a 3d
/// workstation: Rotation, zoom, and panning about a focal point.
struct EditorCameraRig : public CameraRig {
    Vector3 focus;
    float log_distance;
    // XXX: Should pilot angles be its own class?
    float yaw;
    float pitch;

    EditorCameraRig() : focus{0, 0, 0, 0}, log_distance{0}, yaw{0}, pitch{0} {}

    virtual Matrix4 forward_transform() const;
    virtual void on_mouse_drag(float dx, float dy, const uint8_t *keystate);
    virtual void on_mouse_scroll(float dx);
};

struct FirstPersonCameraRig : public CameraRig {
    Vector3 position;
    float yaw;
    float pitch;

    FirstPersonCameraRig() : yaw{0}, pitch{0} {}

    virtual Matrix4 forward_transform() const;
    virtual void on_mouse_move(float dx, float dy);
    virtual void ticker(const uint8_t *keystate);
};

#endif
