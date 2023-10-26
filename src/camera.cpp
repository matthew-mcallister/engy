#include <cassert>
#include <cmath>

#include <SDL2/SDL.h>

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

void EditorCameraRig::on_mouse_drag(float dx, float dy,
                                    const uint8_t *keystate) {
    bool shift = keystate[SDL_SCANCODE_LSHIFT] == SDL_PRESSED ||
                 keystate[SDL_SCANCODE_RSHIFT] == SDL_PRESSED;
    if (shift) {
        auto rot = scene::pilot_angles_xform(yaw, pitch, 0);
        auto d = -dx * rot[1] + dy * rot[2];
        focus = focus + expf(log_distance) * MOUSE_PANNING_SENSITIVITY * d;
    } else {
        yaw -= MOUSE_PIVOT_SENSITIVITY * dx;
        pitch -= MOUSE_PIVOT_SENSITIVITY * dy;
    }
}

void EditorCameraRig::on_mouse_scroll(float dx) {
    log_distance -= MOUSE_ZOOM_SENSITIVITY * dx;
}

Matrix4 FirstPersonCameraRig::forward_transform() const {
    auto rot = scene::pilot_angles_xform(yaw, pitch, 0);
    return Matrix4{-rot[1], -rot[2], rot[0], position.xyz1()};
}

void FirstPersonCameraRig::on_mouse_move(float dx, float dy) {
    yaw -= MOUSE_LOOK_SENSITIVITY * dx;
    pitch += MOUSE_LOOK_SENSITIVITY * dy;
}

void FirstPersonCameraRig::ticker(const uint8_t *keystate) {
    auto speed = MOUSE_LOOK_MOVE_SPEED;
    if ((keystate[SDL_SCANCODE_LSHIFT] | keystate[SDL_SCANCODE_RSHIFT]) ==
        SDL_PRESSED) {
        speed *= 3;
    }
    auto fwd = speed * vec3(cosf(yaw), sinf(yaw), 0);
    auto left = speed * vec3(-sinf(yaw), cosf(yaw), 0);
    auto up = vec3(0, 0, speed);
    if (keystate[SDL_SCANCODE_A] == SDL_PRESSED) {
        position += left;
    }
    if (keystate[SDL_SCANCODE_S] == SDL_PRESSED) {
        position -= fwd;
    }
    if (keystate[SDL_SCANCODE_D] == SDL_PRESSED) {
        position -= left;
    }
    if (keystate[SDL_SCANCODE_W] == SDL_PRESSED) {
        position += fwd;
    }
    if (keystate[SDL_SCANCODE_SPACE] == SDL_PRESSED) {
        position += up;
    }
    if ((keystate[SDL_SCANCODE_LCTRL] | keystate[SDL_SCANCODE_RCTRL]) ==
        SDL_PRESSED) {
        position -= up;
    }
}
