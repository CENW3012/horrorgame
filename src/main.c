#include <SDL3/SDL.h>
#include "game.h"

#ifdef HAVE_SDL3_IMAGE
#include <SDL3_image/SDL_image.h>
#endif

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

#ifdef HAVE_SDL3_IMAGE
    /* Pre-load PNG and JPEG decoder libraries (libpng16.dll, zlib1.dll,
     * jpeg-*.dll on Windows) so any missing DLL is detected here rather
     * than silently producing "Unsupported image format" later. */
    int img_init_flags = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    if (!(img_init_flags & IMG_INIT_PNG))
        SDL_Log("WARNING: PNG support unavailable (%s). "
                "On Windows make sure libpng16.dll and zlib1.dll "
                "are in the same folder as the game.", SDL_GetError());
    if (!(img_init_flags & IMG_INIT_JPG))
        SDL_Log("WARNING: JPEG support unavailable (%s). "
                "On Windows make sure the libjpeg DLL "
                "is in the same folder as the game.", SDL_GetError());
#endif

    SDL_Window *window = SDL_CreateWindow(
        "Project Yozora – A Horror Story",
        WINDOW_W, WINDOW_H, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* Scale the 1280×720 logical canvas to fill any window/fullscreen size,
       preserving aspect ratio with letterboxing. */
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_W, WINDOW_H,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    /* Enable alpha blending globally. */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Game *game = game_init(window, renderer);
    if (!game) {
        SDL_Log("game_init failed");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* ── Main loop ─────────────────────────────────────────────────────── */
    while (game->running && game->state != GAME_STATE_QUIT) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                game->running = 0;
            game_handle_event(game, &event);
        }

        /* Clear */
        SDL_SetRenderDrawColor(renderer, 8, 6, 12, 255);
        SDL_RenderClear(renderer);

        /* Update + render */
        game_update(game);
        game_render(game);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); /* ~60 FPS cap */
    }

    game_cleanup(game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
#ifdef HAVE_SDL3_IMAGE
    IMG_Quit();
#endif
    SDL_Quit();
    return 0;
}

