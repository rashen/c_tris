#include "render.h"

#include "defs.h"
#include "log.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_pixels.h>
#include <SDL_render.h>

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_texture_background = NULL;
SDL_Texture* g_texture_tile = NULL;
SDL_Texture* g_texture_particle = NULL;

int32_t render_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        return 1;
    }

    g_window =
        SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, UNSCALED_WINDOW_WIDTH * DPI,
                         UNSCALED_WINDOW_HEIGHT * DPI, SDL_WINDOW_SHOWN);
    if (g_window == NULL) {
        return 1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (g_renderer == NULL) {
        return 1;
    }

    SDL_Surface* background = IMG_Load("assets/background_01.png");
    g_texture_background = SDL_CreateTextureFromSurface(g_renderer, background);
    if (background == NULL) {
        return 1;
    }
    SDL_FreeSurface(background);

    SDL_Surface* tiles = IMG_Load("assets/tiles_01.png");
    if (tiles == NULL) {
        return 1;
    }
    g_texture_tile = SDL_CreateTextureFromSurface(g_renderer, tiles);
    SDL_FreeSurface(tiles);

    // g_texture_particle = SDL_CreateTexture(g_renderer,
    // SDL_PIXELFORMAT_RGBA32,
    //                                        SDL_TEXTUREACCESS_STATIC, 2, 2);
    // SDL_Color particle_pixels[4] = {
    //     (SDL_Color){.a = 255, .r = 255, .g = 255, .b = 255},
    //     (SDL_Color){.a = 255, .r = 255, .g = 255, .b = 255},
    //     (SDL_Color){.a = 255, .r = 255, .g = 255, .b = 255},
    //     (SDL_Color){.a = 255, .r = 255, .g = 255, .b = 255}};
    // SDL_UpdateTexture(g_texture_particle, NULL, particle_pixels,
    //                   sizeof(SDL_Color));

    return 0;
}

void render_drop(void) {
    SDL_DestroyTexture(g_texture_background);
    SDL_DestroyTexture(g_texture_tile);
    // SDL_DestroyTexture(g_texture_particle);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

void render_draw_background(void) {
    int r = SDL_RenderCopy(g_renderer, g_texture_background, NULL, NULL);
    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void render_draw_tile(int32_t x_pos, int32_t y_pos, EColor color) {
    // Source
    int32_t const src_x = (int32_t)color * TILE_SIZE;
    SDL_Rect const src = {.x = src_x, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE};

    // Dest
    int32_t dst_x = 80;
    dst_x += x_pos * TILE_SIZE;
    int32_t const dst_y = y_pos * TILE_SIZE;

    SDL_Rect const dst = {.x = dst_x * DPI,
                          .y = dst_y * DPI,
                          .w = TILE_SIZE * DPI,
                          .h = TILE_SIZE * DPI};
    int r = SDL_RenderCopy(g_renderer, g_texture_tile, &src, &dst);

    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void render_particle(f32_t x, f32_t y) {
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
    f32_t const game_board_tile_offset = 10.f;

    f32_t const x_window =
        (game_board_tile_offset + x) * (f32_t)TILE_SIZE * (f32_t)DPI;
    f32_t const y_window = y * (f32_t)TILE_SIZE * (f32_t)DPI;
    f32_t const size = 2.f * (f32_t)DPI;

    // LOG_INFO("Drawing particle at (%f, %f)\n", x_window, y_window);

    SDL_FRect const rect = {.x = x_window, .y = y_window, .w = size, .h = size};
    SDL_RenderFillRectF(g_renderer, &rect);
}

void render_present(void) { SDL_RenderPresent(g_renderer); }
