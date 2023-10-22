#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>

#include "asset.h"
#include "config.h"
#include "exceptions.h"
#include "render.h"

std::string get_asset_root() {
    return std::getenv("ASSET_PATH");
}

void main_loop(SDL_Window *window) {
    auto resolver = std::unique_ptr<AssetResolver>(
        new DirectoryAssetResolver(get_asset_root()));
    AssetApi assets{std::move(resolver)};
    Renderer renderer{assets};

    auto start = std::chrono::steady_clock::now();
    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> dt = now - start;
        renderer.render(dt.count());
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