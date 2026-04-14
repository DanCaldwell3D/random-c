#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

typedef struct {
    SDL_Renderer* renderer;
    SDL_Window* window;
} AppState;



SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState* as = SDL_calloc(1, sizeof(AppState));
    *appstate = (AppState*)as;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("SDL Test", 
        800,
        600,
        SDL_WINDOW_RESIZABLE,
        &as->window,
        &as->renderer)) {
        return SDL_APP_FAILURE;
    }
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState* as = appstate;
    
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState* as = appstate;
    
    SDL_SetRenderDrawColor(as->renderer, 255, 128, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    SDL_RenderPresent(as->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState* as = appstate;

    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);

    SDL_free(as);
}