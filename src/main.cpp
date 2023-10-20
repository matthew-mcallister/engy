#include <stdexcept>

#include <SDL2/SDL.h>

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 240;

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Event event;
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

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}