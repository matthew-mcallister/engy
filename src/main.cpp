#include "main.h"

#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "asset.h"
#include "camera.h"
#include "config.h"
#include "exceptions.h"
#include "math/aabb.h"
#include "math/scene.h"
#include "math/vector.h"
#include "mesh_builder.h"
#include "vulkan/device.h"
#include "vulkan/renderer.h"

std::string get_asset_root() {
    return std::getenv("ASSET_PATH");
}

void set_relative_mouse(bool enable) {
    if (SDL_SetRelativeMouseMode(enable ? SDL_TRUE : SDL_FALSE)) {
        throw SystemException("Failed to capture mouse");
    }
}

void State::set_focus(bool focused) {
    m_focused = focused;
    set_relative_mouse(focused);
}

void State::handle_event(const SDL_Event &event) {
    const auto mouse_buttons = SDL_GetMouseState(nullptr, nullptr);
    const auto *keyboard_state = SDL_GetKeyboardState(nullptr);

    if (event.type == SDL_WINDOWEVENT) {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            set_focus(true);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            set_focus(false);
            break;
        }
    }
    if (!m_focused) {
        return;
    }

    switch (event.type) {
    case SDL_MOUSEMOTION: {
        // TODO: Only handle inputs if keyboard focus is acquired
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
    case SDL_KEYDOWN:
        if (keyboard_state[SDL_SCANCODE_ESCAPE] == SDL_PRESSED) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
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

Matrix4 get_projection() {
    float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    return scene::projection(FOVY, aspect, Z_NEAR, Z_FAR);
}

// clang-format off
const std::array<float, 36> VERTICES = {
    -0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
    -0.5,  0.5, 0.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0,
     0.5, -0.5, 0.0, 0.0, 0.0, -1.0, 1.0, 0.0, 0.0,
     0.5,  0.5, 0.0, 0.0, 0.0, -1.0, 1.0, 1.0, 0.0,
};
const std::array<uint32_t, 6> INDICES = {
    0, 1, 3,
    0, 3, 2
};
// clang-format on

void main_loop(SDL_Window *window) {
    auto resolver = std::unique_ptr<AssetResolver>(
        new DirectoryAssetResolver(get_asset_root()));
    AssetApi assets{std::move(resolver)};
    // Renderer renderer{assets};

    std::unique_ptr<FirstPersonCameraRig> rig{new FirstPersonCameraRig()};
    State state{std::move(rig)};

    auto device = VulkanDevice::create(window, 0, true);
    auto swapchain = VulkanSwapchain::create(device, vk::SwapchainKHR{});
    VulkanRenderer renderer{assets, std::move(device), std::move(swapchain)};
    renderer.create_graphics_pipeline(assets);

    BlockRegistry registry = BlockRegistry::create();
    ChunkMap chunk_map;

    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            for (int k = -2; k <= 2; k++) {
                chunk_map.generate_chunk({i, j, k});
            }
        }
    }

    renderer.staging().begin_staging();
    const auto mesh = renderer.create_mesh(
        {(const char *)VERTICES.data(), sizeof(float) * VERTICES.size()},
        INDICES);
    chunk_map.update_mesh(registry, renderer, {0, 0, 0});
    renderer.staging().end_staging(renderer.device().graphics_queue());
    renderer.staging().wait();

    auto start = std::chrono::steady_clock::now();
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto finish;
            }
            state.handle_event(event);
        }
        const auto *keystate = SDL_GetKeyboardState(nullptr);
        state.ticker(keystate);

        try {
            renderer.flush_frame();
            renderer.acquire_image();
        } catch (const TimeoutException &e) {
            continue;
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> dt = now - start;

        renderer.begin_rendering();
        auto proj = get_projection();
        auto view = state.rig().reverse_transform();
        ViewUniforms view_uniforms{proj, view};
        renderer.update_uniforms(view_uniforms);
        renderer.begin_rendering_meshes();
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                for (int k = -1; k <= 1; k++) {
                    const auto &chunk = chunk_map.at({i, j, k});
                    renderer.render_chunk(chunk);
                }
            }
        }
        renderer.end_rendering();
        renderer.present();
    }

finish:
    renderer.wait_idle();
}

int sdl_main() {
    SDL_Window *window;
    int result;

    result = SDL_Init(SDL_INIT_VIDEO);
    if (result < 0) {
        throw SystemException("Failed to initialize SDL");
    }

    load_vulkan_library();

    window = SDL_CreateWindow(
        WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw SystemException("Failed to create window");
    }

    main_loop(window);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main() {
    sdl_main();
}