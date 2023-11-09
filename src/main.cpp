#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "asset.h"
#include "camera.h"
#include "config.h"
#include "exceptions.h"
#include "math/aabb.h"
#include "math/vector.h"
#include "mesh_builder.h"
#include "render.h"
#include "vulkan/device.h"
#include "vulkan/renderer.h"

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

// ChunkMesh create_mesh(TextureMap &textures) {
//     MeshData data;
//     BlockFace face;
//     face.texture = *textures.get("blocks/dirt.png");
//     data.add_face(face);
//
//     ChunkMesh mesh;
//     mesh.update(data);
//     return mesh;
// }

// clang-format off
const std::array<float, 12> VERTICES = {
    -0.5, -0.5, 0.0,
    -0.5,  0.5, 0.0,
     0.5, -0.5, 0.0,
     0.5,  0.5, 0.0,
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

    // TODO: Only set relative mouse modewhen window is focused
    // if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
    //    throw SystemException("Failed to capture mouse");
    //}

    std::unique_ptr<FirstPersonCameraRig> rig{new FirstPersonCameraRig()};
    State state{std::move(rig)};

    auto device = VulkanDevice::create(window, 0, true);
    auto swapchain = VulkanSwapchain::create(device, vk::SwapchainKHR{});
    VulkanRenderer renderer = {std::move(device), std::move(swapchain)};
    renderer.create_graphics_pipeline(assets);

    renderer.staging().begin_staging();
    const auto mesh = renderer.create_mesh(
        {(const char *)VERTICES.data(), sizeof(float) * VERTICES.size()},
        INDICES);
    renderer.staging().end_staging(renderer.device().graphics_queue());
    renderer.staging().wait();

    // renderer.make_image_resident("blocks/dirt.png");
    // auto mesh = create_mesh(renderer.texture_map());

    // ChunkMap chunk_map;
    // for (int i = -2; i <= 2; i++) {
    //     for (int j = -2; j <= 2; j++) {
    //         for (int k = -2; k <= 2; k++) {
    //             chunk_map.generate_chunk({i, j, k});
    //         }
    //     }
    // }
    // for (int i = -1; i <= 1; i++) {
    //     for (int j = -1; j <= 1; j++) {
    //         for (int k = -1; k <= 1; k++) {
    //             chunk_map.update_mesh({i, j, k});
    //         }
    //     }
    // }

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
        renderer.begin_rendering_meshes();
        renderer.render_mesh(mesh);
        renderer.end_rendering();
        renderer.present();
        // renderer.prepare_frame(state);
        // renderer.render_mesh(mesh, Matrix4::identity());
        //  for (int i = -1; i <= 1; i++) {
        //      for (int j = -1; j <= 1; j++) {
        //          for (int k = -1; k <= 1; k++) {
        //              renderer.render_chunk(chunk_map[{i, j, k}]);
        //          }
        //      }
        //  }
        // SDL_GL_SwapWindow(window);
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