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
    const auto *keyboard_state = SDL_GetKeyboardState(nullptr);
    switch (event.type) {
    case SDL_MOUSEMOTION: {
        bool dragging = !!(mouse_buttons & SDL_BUTTON(2));
        auto dx = event.motion.xrel, dy = event.motion.yrel;
        if (dragging) {
            m_rig->on_mouse_drag(dx, dy, keyboard_state);
        } else {
            m_rig->on_mouse_move(dx, dy);
        }
        break;
    }
    case SDL_MOUSEWHEEL:
        m_rig->on_mouse_scroll(event.wheel.preciseY);
        break;
    }
}

void State::ticker(const uint8_t *keystate) {
    m_rig->ticker(keystate);
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

void initialize_chunk(Chunk &chunk) {
    auto &blocks = chunk.data().blocks;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                int counter = i + j + k;
                if (i % 2 == 0 && j % 2 == 0 && k % 2 == 0) {
                    counter += 1;
                }
                bool solid = counter <= 8;
                blocks[i][j][k].type =
                    solid ? block_type::SOLID : block_type::EMPTY;
            }
        }
    }
}

void main_loop(SDL_Window *window) {
    auto resolver = std::unique_ptr<AssetResolver>(
        new DirectoryAssetResolver(get_asset_root()));
    AssetApi assets{std::move(resolver)};
    Renderer renderer{assets};

    if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
        throw SystemException("Failed to capture mouse");
    }

    std::unique_ptr<FirstPersonCameraRig> rig{new FirstPersonCameraRig()};
    State state{std::move(rig)};

    Chunk chunk;
    initialize_chunk(chunk);
    chunk.update_mesh();

    auto start = std::chrono::steady_clock::now();
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
            state.handle_event(event);
        }
        const auto *keystate = SDL_GetKeyboardState(nullptr);
        state.ticker(keystate);

        auto now = std::chrono::steady_clock::now();

        std::chrono::duration<float> dt = now - start;
        renderer.prepare_frame(state);
        renderer.render_chunk(chunk);
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