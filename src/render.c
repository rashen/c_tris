#include "render.h"

#include "defs.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_render.h>

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_game_background = NULL;
SDL_Texture* g_tiles = NULL;

int32_t render_init(int32_t window_width, int32_t window_height, float dpi,
                    int32_t game_tiles_wide, int32_t game_tiles_high,
                    int32_t tile_size) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    g_window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, window_width * dpi,
                                window_height * dpi, SDL_WINDOW_SHOWN);
    if (g_window == NULL) {
        printf("Could not create window");
        return 1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (g_renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initializer renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_Surface* background = IMG_Load("assets/background_01.png");
    g_game_background = SDL_CreateTextureFromSurface(g_renderer, background);
    if (background == NULL) {
        printf("%s\n", SDL_GetError());
        return 1;
    }
    SDL_FreeSurface(background);

    SDL_Surface* tiles = IMG_Load("assets/tiles_01.png");
    if (tiles == NULL) {
        printf("%s\n", SDL_GetError());
        return 1;
    }
    g_tiles = SDL_CreateTextureFromSurface(g_renderer, tiles);
    SDL_FreeSurface(tiles);

    return 0;
}

void render_drop() {
    SDL_DestroyTexture(g_game_background);
    SDL_DestroyTexture(g_tiles);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void render_draw_background() {
    int r = SDL_RenderCopy(g_renderer, g_game_background, NULL, NULL);
    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void render_draw_tile(int32_t x_pos, int32_t y_pos, EColor color) {
    // Source
    int const src_x = color * TILE_SIZE;
    SDL_Rect const src = {.x = src_x, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE};

    // Dest
    int dst_x = 80;
    dst_x += x_pos * TILE_SIZE;
    int const dst_y = y_pos * TILE_SIZE;

    SDL_Rect const dst = {.x = dst_x * DPI,
                          .y = dst_y * DPI,
                          .w = TILE_SIZE * DPI,
                          .h = TILE_SIZE * DPI};
    int r = SDL_RenderCopy(g_renderer, g_tiles, &src, &dst);

    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void render_present() { SDL_RenderPresent(g_renderer); }
