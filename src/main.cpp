#include <iostream>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "asset.h"
#include "render.h"

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 240;

std::string get_asset_root() {
    return std::getenv("ASSET_PATH");
}

void main_loop() {
    auto resolver = std::unique_ptr<AssetResolver>(
        new DirectoryAssetResolver(get_asset_root()));
    AssetApi assets{std::move(resolver)};

    std::string shader = assets.load_text("vertex.glsl");
    std::cout << shader << std::endl;

    SDL_Event event;
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
    }
}

int sdl_main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    int result;

    result = SDL_Init(SDL_INIT_VIDEO);
    if (result < 0) {
        throw std::runtime_error("Failed to initialize SDL");
    }

    result = SDL_CreateWindowAndRenderer(
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer);
    if (result) {
        throw std::runtime_error("Failed to create window");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    auto gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        throw std::runtime_error("Failed to create OpenGL context");
    }

    main_loop();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main() {
    sdl_main();
}