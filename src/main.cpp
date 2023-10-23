#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "asset.h"
#include "camera.h"
#include "config.h"
#include "exceptions.h"
#include "math/aabb.h"
#include "math/vector.h"
#include "render.h"

std::string get_asset_root() {
    return std::getenv("ASSET_PATH");
}

void State::handle_event(const SDL_Event &event) {
    const auto mouse_buttons = SDL_GetMouseState(nullptr, nullptr);
    const auto keyboard_state = SDL_GetKeyboardState(nullptr);
    switch (event.type) {
    case SDL_MOUSEMOTION: {
        bool dragging = !!(mouse_buttons & SDL_BUTTON(2));
        if (dragging) {
            EditorCameraRig::Mode mode;
            if (keyboard_state[SDL_SCANCODE_LSHIFT] == SDL_PRESSED ||
                keyboard_state[SDL_SCANCODE_RSHIFT] == SDL_PRESSED) {
                mode = EditorCameraRig::PAN;
            } else {
                mode = EditorCameraRig::PIVOT;
            }
            m_rig.on_mouse_drag(event.motion.xrel, event.motion.yrel, mode);
        }
        break;
    }
    case SDL_MOUSEWHEEL:
        m_rig.on_mouse_scroll(event.wheel.preciseY);
        break;
    }
}

/// @brief Returns a view-space vector representing where the cursor is
/// pointing.
///
/// When the cursor is on the screen, its location represents a
/// direction in space relative to the current camera. This function
/// returns that vector in view space, which can be transformed into
/// world space using the camera matrix.
Vector3 get_cursor_vector() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    float aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    float vy = 2 * tan(FOVY) * ((float)y - (float)WINDOW_HEIGHT / 2.0) /
               (float)WINDOW_HEIGHT;
    float vx = 2 * aspect_ratio * tan(FOVY) *
               ((float)x - (float)WINDOW_WIDTH / 2.0) / (float)WINDOW_WIDTH;
    return vec3(vx, vy, 1).normalized();
}

void main_loop(SDL_Window *window) {
    auto resolver = std::unique_ptr<AssetResolver>(
        new DirectoryAssetResolver(get_asset_root()));
    AssetApi assets{std::move(resolver)};
    Renderer renderer{assets};

    State state;
    state.rig().focus = vec3(0.5, 0.5, 0.5);
    state.rig().log_distance = 0.5;

    AABB3 aabb{vec3(0), vec3(1)};

    auto start = std::chrono::steady_clock::now();
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
            state.handle_event(event);
        }

        auto now = std::chrono::steady_clock::now();

        auto vec = get_cursor_vector();
        auto xform = state.rig().forward_transform();
        auto result = aabb.intersect_ray(xform[3], xform * vec);
        state.highlight() = result.intersects;

        std::chrono::duration<float> dt = now - start;
        renderer.render(state);
        SDL_GL_SwapWindow(window);
    }
}

int sdl_main() {
    SDL_Window *window;
    int result;

    result = SDL_Init(SDL_INIT_VIDEO);
    if (result < 0) {
        throw SystemException("Failed to initialize SDL");
    }

    window = SDL_CreateWindow(
        WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw SystemException("Failed to create window");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    auto gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        throw SystemException("Failed to create OpenGL context");
    }

    main_loop(window);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main() {
    sdl_main();
}