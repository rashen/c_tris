#include <assert.h>
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_gesture.h>
#include <SDL_image.h>
#include <SDL_keycode.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <stddef.h>

/* TODO:
 * Fix collision and rotation
 * Proper textures.
 * Remove black border on left side
 */

#define N_ELEMENTS(X) (sizeof(X) / sizeof(*(X)))

#define TILE_SIZE 8
#define GAME_TILES_WIDE 10
#define GAME_TILES_HIGH 16 // This is the whole height

// Taken from Tic80
#define UNSCALED_WINDOW_WIDTH 240
#define UNSCALED_WINDOW_HEIGHT 136
#define DPI 4

/* Dimensions:
Width of the game board is TILE_SIZE * GAMES_TILES_WIDE = 80. To center this the
drawing rect should start at 240/2 - 80/2 = 80; This splits the window into
three equally sized parts.
*/

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_game_board = NULL;
SDL_Texture* g_game_background = NULL;
SDL_Texture* g_tile = NULL;

int32_t game_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    g_window =
        SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, UNSCALED_WINDOW_WIDTH * DPI,
                         UNSCALED_WINDOW_HEIGHT * DPI, SDL_WINDOW_SHOWN);
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
        GAME_TILES_WIDE * TILE_SIZE, GAME_TILES_HIGH * TILE_SIZE);

    if (g_game_board == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create global texture: %s", SDL_GetError());
        return 1;
    }

    g_game_background = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        (GAME_TILES_WIDE + 2) * TILE_SIZE, GAME_TILES_HIGH * TILE_SIZE);
    if (g_game_background == NULL) {
        return 1;
    }

    g_tile =
        SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING, TILE_SIZE, TILE_SIZE);
    if (g_tile == NULL) {
        return 1;
    }

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
        return 1;
    }

    return 0;
}

void game_release() {
    // TODO: Why are we wasting time cleaning up when closing the game?
    SDL_DestroyTexture(g_game_board);
    SDL_DestroyTexture(g_game_background);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

typedef enum {
    EBrickShape_Straight = 0,
    EBrickShape_Square,
    EBrickShape_T,
    EBrickShape_LRight,
    EBrickShape_LLeft,
    EBrickShape_RSkew,
    EBrickShape_LSkew,
} EBrickShape;

typedef struct {
    float x;
    float y;
} APP_Vec2;

typedef struct {
    int32_t x;
    int32_t y;
} IVec2;

IVec2 ivec2_add(IVec2 lhs, IVec2 rhs) {
    return (IVec2){.x = lhs.x + rhs.x, .y = lhs.y + rhs.y};
}

typedef enum {
    ETileState_None = 0,
    ETileState_Green,
    ETileState_Blue,
    ETileState_Red,
    ETileState_Orange,
    ETileState_LtBlue,
    ETileState_Yellow,
    ETileState_Purple,
    ETileState_Border,
} ETileState;

typedef struct {
    IVec2 pos;
    IVec2 tiles[4];
    ETileState state; // TODO: This name is misleading
} Brick;

SDL_Color get_color(ETileState tile_state) {
    switch (tile_state) {
        case ETileState_None:
            return (SDL_Color){.r = 59, .g = 31, .b = 45, .a = 128};
        case ETileState_Green:
            return (SDL_Color){.r = 52, .g = 101, .b = 36, .a = 255};
        case ETileState_Blue:
            return (SDL_Color){.r = 84, .g = 119, .b = 196, .a = 255};
        case ETileState_Red:
            return (SDL_Color){.r = 208, .g = 70, .b = 72, .a = 255};
        case ETileState_Orange:
            return (SDL_Color){.r = 210, .g = 125, .b = 44, .a = 255};
        case ETileState_LtBlue:
            return (SDL_Color){.r = 109, .g = 193, .b = 201, .a = 255};
        case ETileState_Yellow:
            return (SDL_Color){.r = 218, .g = 212, .b = 94, .a = 255};
        case ETileState_Purple:
            return (SDL_Color){.r = 48, .g = 52, .b = 109, .a = 255};
        case ETileState_Border:
            return (SDL_Color){.r = 133, .g = 149, .b = 161, .a = 255};
    }
}

// TODO: Rename to imply color
ETileState get_state_from_shape(EBrickShape shape) {
    switch (shape) {
        case EBrickShape_Straight:
            return ETileState_LtBlue;
        case EBrickShape_Square:
            return ETileState_Yellow;
        case EBrickShape_T:
            return ETileState_Blue;
        case EBrickShape_LRight:
            return ETileState_Purple;
        case EBrickShape_LLeft:
            return ETileState_Orange;
        case EBrickShape_RSkew:
            return ETileState_Red;
        case EBrickShape_LSkew:
            return ETileState_Green;
    }
}

#define NUM_TILES (GAME_TILES_WIDE * GAME_TILES_HIGH)

typedef struct {
    ETileState tiles[NUM_TILES];
    Brick current_brick;
} GameState;

typedef enum {
    ECollision_None = 0,
    ECollision_Side = 1,
    ECollision_Bottom = 2,
} ECollision;

ECollision tiles_check_collision(ETileState tiles[], IVec2 brick_tiles[],
                                 IVec2 new_pos) {
    for (int32_t i = 0; i < 4; i++) {
        int32_t x = new_pos.x + brick_tiles[i].x;
        int32_t y = new_pos.y + brick_tiles[i].y;

        if (x < 0 || x >= GAME_TILES_WIDE) {
            return ECollision_Side;
        }

        if (y >= GAME_TILES_HIGH) {
            return ECollision_Bottom;
        }

        int32_t index = y * GAME_TILES_WIDE + x;
        assert(index < NUM_TILES);
        if (tiles[index] != ETileState_None) {
            return ECollision_Bottom;
        }
    }

    return ECollision_None;
}

void tiles_add_brick(ETileState tiles[], Brick* brick) { return; }

Brick create_brick(EBrickShape shape) {
    Brick brick = {0};
    brick.state = get_state_from_shape(shape);
    // TODO: Pick a random pos on first row
    brick.pos = (IVec2){1, 1};

    switch (shape) {
        case EBrickShape_Straight: {
            brick.tiles[0] = (IVec2){.x = -1, .y = 0};
            brick.tiles[1] = (IVec2){.x = 0, .y = 0};
            brick.tiles[2] = (IVec2){.x = 1, .y = 0};
            brick.tiles[3] = (IVec2){.x = 2, .y = 0};
        } break;
        case EBrickShape_Square: {
            brick.tiles[0] = (IVec2){.x = 0, .y = 0};
            brick.tiles[1] = (IVec2){.x = 1, .y = 0};
            brick.tiles[2] = (IVec2){.x = 0, .y = 1};
            brick.tiles[3] = (IVec2){.x = 1, .y = 1};
        } break;
        case EBrickShape_T: {
            brick.tiles[0] = (IVec2){.x = 0, .y = 1};
            brick.tiles[1] = (IVec2){.x = 1, .y = 1};
            brick.tiles[2] = (IVec2){.x = 1, .y = 2};
            brick.tiles[3] = (IVec2){.x = 1, .y = 0};
        } break;
        case EBrickShape_LRight: {
            brick.tiles[0] = (IVec2){.x = 0, .y = 0};
            brick.tiles[1] = (IVec2){.x = 0, .y = 1};
            brick.tiles[2] = (IVec2){.x = 0, .y = 2};
            brick.tiles[3] = (IVec2){.x = 1, .y = 2};
        } break;
        case EBrickShape_LLeft: {
            brick.tiles[0] = (IVec2){.x = 1, .y = 0};
            brick.tiles[1] = (IVec2){.x = 1, .y = 1};
            brick.tiles[2] = (IVec2){.x = 1, .y = 2};
            brick.tiles[3] = (IVec2){.x = 0, .y = 2};
        } break;
        case EBrickShape_RSkew: {
            brick.tiles[0] = (IVec2){.x = 0, .y = 1};
            brick.tiles[1] = (IVec2){.x = 1, .y = 1};
            brick.tiles[2] = (IVec2){.x = 1, .y = 0};
            brick.tiles[3] = (IVec2){.x = 0, .y = 1};
        } break;
        case EBrickShape_LSkew: {
            brick.tiles[0] = (IVec2){.x = 1, .y = 0};
            brick.tiles[1] = (IVec2){.x = 0, .y = 0};
            brick.tiles[2] = (IVec2){.x = 0, .y = 1};
            brick.tiles[3] = (IVec2){.x = 1, .y = 0};
        } break;
    }
    return brick;
}

void draw_tile(IVec2 pos, SDL_Color color) {

    SDL_Color tile[TILE_SIZE * TILE_SIZE] = {0};
    for (int i = 0; i < N_ELEMENTS(tile); i++) {
        tile[i] = color;
    }

    int r = SDL_UpdateTexture(g_tile, NULL, &tile, sizeof(SDL_Color));

    int const x_start = 80;
    int const y_start = 0;

    int const x = x_start + pos.x * TILE_SIZE;
    int const y = y_start + pos.y * TILE_SIZE;

    SDL_Rect const dst = {
        .x = x * DPI, .y = y * DPI, .w = TILE_SIZE * DPI, .h = TILE_SIZE * DPI};
    r |= SDL_RenderCopy(g_renderer, g_tile, NULL, &dst);

    if (r != 0) {
        printf("%s\n", SDL_GetError());
    }
}

void draw_brick(Brick const* brick) {
    SDL_Color const color = get_color(brick->state);
    for (int i = 0; i < 4; i++) {
        draw_tile(ivec2_add(brick->pos, brick->tiles[i]), color);
    }
}

void draw_board() {
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

IVec2 tile_index_to_pos(uint32_t index) {
    int32_t x = index % GAME_TILES_WIDE;
    int32_t y = index / GAME_TILES_WIDE;
    return (IVec2){.x = x, .y = y};
}

uint32_t pos_to_tile_index(IVec2 pos) {
    return pos.y * GAME_TILES_WIDE + pos.x;
}

// TODO: I'm not sure the explicit size is doing anything here
void draw_tiles(ETileState tiles[NUM_TILES]) {
    for (int i = 0; i < NUM_TILES; i++) {
        if (tiles[i] == ETileState_None) {
            continue;
        }

        SDL_Color color = get_color(tiles[i]);
        IVec2 pos = tile_index_to_pos(i);
        draw_tile(pos, color);
    }
}

void game_handle_touchdown(GameState* game) {

    // 1. Move brick tiles to game.tiles
    ETileState state = game->current_brick.state;
    for (int i = 0; i < 4; i++) {
        IVec2 const pos =
            ivec2_add(game->current_brick.tiles[i], game->current_brick.pos);
        uint32_t tile_index = pos_to_tile_index(pos);
        game->tiles[tile_index] = state;
    }

    // 2. Spawn a new (random) brick
    EBrickShape const shape = (EBrickShape)rand() % 8;
    game->current_brick = create_brick(shape);
    // 3. Check if we should remove a line of tiles
    // 4. Update score
}

uint32_t game_movement_callback(uint32_t interval, void* game_state) {
    GameState* game = (GameState*)game_state;
    Brick* b = &game->current_brick;
    ETileState* t = game->tiles;

    IVec2 new_pos = b->pos;
    new_pos.y += 1;

    ECollision const collision = tiles_check_collision(t, b->tiles, new_pos);
    if (collision == ECollision_None) {
        b->pos = new_pos;
    } else if (collision == ECollision_Side) {
        assert("Unreachable");
    } else if (collision == ECollision_Bottom) {
        game_handle_touchdown(game);
    }

    return interval;
}

typedef enum {
    ERotation_CW = 0,
    ERotation_CCW,
} ERotation;

int32_t min(int32_t lhs, int32_t rhs) { return lhs < rhs ? lhs : rhs; }

// TODO: Maybe this should be the other way around. Packing downwards.
void pack_brick_tiles(IVec2 brick_tiles[]) {
    int32_t min_y = INT32_MIN;
    for (int i = 0; i < 4; i++) {
        min_y = min(min_y, brick_tiles[i].y);
    }

    for (int i = 0; i < 4; i++) {
        brick_tiles[i].y = brick_tiles[i].y - min_y;
    }
}

IVec2 vec2_rotate_cw(IVec2 vec2) { return (IVec2){.x = -vec2.y, .y = vec2.x}; }

IVec2 vec2_rotate_ccw(IVec2 vec2) { return (IVec2){.x = vec2.y, .y = -vec2.x}; }

void game_rotate(GameState* game, ERotation rot) {
    IVec2 new_tiles[4] = {0};
    for (int i = 0; i < 4; i++) {
        IVec2 const tile_pos = game->current_brick.tiles[i];
        if (rot == ERotation_CW) {
            new_tiles[i] = vec2_rotate_cw(tile_pos);
        } else {
            new_tiles[i] = vec2_rotate_ccw(tile_pos);
        }
    }
    pack_brick_tiles(new_tiles);

    ECollision const collision =
        tiles_check_collision(game->tiles, new_tiles, game->current_brick.pos);
    if (collision == ECollision_Bottom) {
        return;
    }

    if (collision == ECollision_Side) {
        int32_t offset[4] = {1, -1, 2, -2};
        for (int i = 0; i < N_ELEMENTS(offset); i++) {
            IVec2 offseted_pos = game->current_brick.pos;
            offseted_pos.x += offset[i];
            if (ECollision_None ==
                tiles_check_collision(game->tiles, new_tiles, offseted_pos)) {
                game->current_brick.pos = offseted_pos;
                break;
            }
        }
    }

    memcpy(game->current_brick.tiles, new_tiles, 4 * sizeof(IVec2));
}

void move_sideway(GameState* game, int32_t dy) {
    IVec2 new_pos = game->current_brick.pos;
    new_pos.x += dy;
    if (ECollision_None == tiles_check_collision(game->tiles,
                                                 game->current_brick.tiles,
                                                 new_pos)) {
        game->current_brick.pos = new_pos;
    }
}

int main(int argc, char** args) {
    if (game_init() != 0) {
        goto quit;
    }

    GameState game = {0};
    game.current_brick = create_brick(EBrickShape_Straight);

    uint32_t delta_ticks = 0;
    uint32_t const target_frame_ticks = 16;

    SDL_TimerID movement_timer =
        SDL_AddTimer(800, game_movement_callback, &game);

    SDL_Event event = {0};
    while (1) {
        uint32_t const frame_start = SDL_GetTicks();

        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        goto quit;
                    } break;
                    case SDLK_LEFT:
                    case SDLK_a: {
                        move_sideway(&game, -1);
                    } break;
                    case SDLK_RIGHT:
                    case SDLK_d: {
                        move_sideway(&game, 1);
                    } break;
                    case SDLK_DOWN:
                    case SDLK_s: {
                        // TODO: This is a hack (that works surprisingly well)
                        game_movement_callback(0, &game);
                    } break;
                    case SDLK_z: {
                        game_rotate(&game, ERotation_CCW);
                    } break;
                    case SDLK_x: {
                        game_rotate(&game, ERotation_CW);
                    }
                }
            } break;
        }

        draw_board();
        draw_brick(&game.current_brick);
        draw_tiles(game.tiles);

        SDL_RenderPresent(g_renderer);

        uint32_t const frame_end = SDL_GetTicks();
        delta_ticks = frame_end - frame_start;
        if (delta_ticks < target_frame_ticks) {
            SDL_Delay(target_frame_ticks - delta_ticks);
            // Fetch ticks again after delay. The delay can potentially be
            // longer than specified.
            delta_ticks = SDL_GetTicks() - frame_start;
        }
    }

quit:
    game_release();

    return 0;
}
