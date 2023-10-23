#include <cassert>
#include <cmath>

#include "camera.h"
#include "config.h"
#include "math/matrix.h"
#include "math/scene.h"
#include "math/vector.h"

Matrix4 EditorCameraRig::forward_transform() const {
    auto rot = scene::pilot_angles_xform(yaw, pitch, 0);
    auto offset = focus + rot[0] * expf(log_distance);
    return Matrix4{rot[1], -rot[2], -rot[0], offset.xyz1()};
}

Matrix4 EditorCameraRig::reverse_transform() const {
    return forward_transform().rigid_inverse();
}

void EditorCameraRig::on_mouse_drag(float dx, float dy,
                                    EditorCameraRig::Mode mode) {
    if (mode == EditorCameraRig::PIVOT) {
        yaw -= MOUSE_PIVOT_SENSITIVITY * dx;
        pitch -= MOUSE_PIVOT_SENSITIVITY * dy;
    } else {
        assert(mode == EditorCameraRig::PAN);
        auto rot = scene::pilot_angles_xform(yaw, pitch, 0);
        auto d = -dx * rot[1] + dy * rot[2];
        focus = focus + expf(log_distance) * MOUSE_PANNING_SENSITIVITY * d;
    }
}

void EditorCameraRig::on_mouse_scroll(float dx) {
    log_distance -= MOUSE_ZOOM_SENSITIVITY * dx;
}
