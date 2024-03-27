#include "defs.h"
#include "render.h"

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

void draw_brick(Brick const* brick) {
    SDL_Color const color = get_color(brick->state);
    for (int i = 0; i < 4; i++) {
        IVec2 pos = ivec2_add(brick->pos, brick->tiles[i]);
        render_draw_tile(pos.x, pos.y, color);
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
        render_draw_tile(pos.x, pos.y, color);
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
    if (render_init(UNSCALED_WINDOW_WIDTH, UNSCALED_WINDOW_HEIGHT, DPI,
                    GAME_TILES_WIDE, GAME_TILES_HIGH, TILE_SIZE) != 0) {
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

        render_draw_board();
        draw_brick(&game.current_brick);
        draw_tiles(game.tiles);

        render_present();

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
    render_drop();

    return 0;
}
