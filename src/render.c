#include "render.h"

#include "defs.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_render.h>

#define MAX_TEXTURES 64
// #define INVALID_HANDLE -1

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_game_board = NULL;
SDL_Texture* g_game_background = NULL;
SDL_Texture* g_tiles = NULL;

// typedef enum {
//     ETextureOffset_Empty = 0,
//     ETextureOffset_Border,
//     ETextureOffset_Red,
//     ETextureOffset_Blue,
//     ETextureOffset_LtBlue,
//     ETextureOffset_Orange,
//     ETextureOffset_Green,
//     ETextureOffset_Pink,
//     ETextureOffset_Purple,
//     ETextureOffset_MAX
// } ETextureOffset;

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

    g_game_board = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        game_tiles_wide * tile_size, game_tiles_high * tile_size);

    if (g_game_board == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create global texture: %s", SDL_GetError());
        return 1;
    }

    g_game_background = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        (game_tiles_wide + 2) * tile_size, game_tiles_high * tile_size);
    if (g_game_background == NULL) {
        return 1;
    }

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
    SDL_DestroyTexture(g_game_board);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void render_draw_background() {
    SDL_Color game_background[(GAME_TILES_WIDE + 2) * TILE_SIZE *
                              GAME_TILES_HIGH * TILE_SIZE] = {0};

    size_t const column_width = (GAME_TILES_WIDE + 2) * TILE_SIZE;
    // TODO: Move ETileState to a header file
    // SDL_Color const background_color = get_color(TileState_Border);
    for (int i = 0; i < N_ELEMENTS(game_background); i++) {
        game_background[i] =
            (SDL_Color){.r = 133, .g = 149, .b = 161, .a = 255};
    }
    SDL_Rect const background_rect = {
        .x = 0, .y = 0, .w = column_width, .h = GAME_TILES_HIGH * TILE_SIZE};
    int r = SDL_UpdateTexture(g_game_background, &background_rect,
                              &game_background, sizeof(SDL_Color));
    if (r != 0) {
        printf("%s", SDL_GetError());
    }
}

void render_draw_tile(int32_t x_pos, int32_t y_pos, EColor color) {
    // SDL_Color tile[TILE_SIZE * TILE_SIZE] = {0};
    // for (int i = 0; i < N_ELEMENTS(tile); i++) {
    //     tile[i] = color;
    // }

    // int r = SDL_UpdateTexture(g_tile, NULL, &tile, sizeof(SDL_Color));
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

void render_draw_board() {
    SDL_Color
        pixels[GAME_TILES_WIDE * GAME_TILES_HIGH * TILE_SIZE * TILE_SIZE] = {0};
    for (int i = 0; i < N_ELEMENTS(pixels); i++) {
        pixels[i] = (SDL_Color){.r = 255, .g = 0, .b = 0, .a = 255};
    }

    int r = SDL_UpdateTexture(g_game_board, NULL, &pixels, sizeof(SDL_Color));

    int const screen_mid = UNSCALED_WINDOW_WIDTH * DPI / 2;
    int const background_width = (GAME_TILES_WIDE + 2) * TILE_SIZE * DPI;
    int const board_width = GAME_TILES_WIDE * TILE_SIZE * DPI;
    int const board_height = GAME_TILES_HIGH * TILE_SIZE * DPI;
    int const window_height = UNSCALED_WINDOW_HEIGHT * DPI;

    SDL_Rect const background_dst_rect = {.x =
                                              screen_mid - background_width / 2,
                                          .y = 0,
                                          .w = background_width,
                                          .h = window_height};

    SDL_Rect const game_board_dst_rect = {.x = screen_mid - board_width / 2,
                                          .y = 0,
                                          .w = board_width,
                                          .h = board_height};

    r |= SDL_RenderCopy(g_renderer, g_game_background, NULL,
                        &background_dst_rect);
    r |= SDL_RenderCopy(g_renderer, g_game_board, NULL, &game_board_dst_rect);

    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void render_present() { SDL_RenderPresent(g_renderer); }
