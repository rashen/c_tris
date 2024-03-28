#include "defs.h"
#include "math.h"
#include "render.h"

#include <assert.h>
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
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

EColor get_color_from_state(ETileState state) {
    switch (state) {
        case ETileState_None:
            return EColor_None;
        case ETileState_Green:
            return EColor_Green;
        case ETileState_Blue:
            return EColor_Blue;
        case ETileState_Red:
            return EColor_Red;
        case ETileState_Orange:
            return EColor_Orange;
        case ETileState_LtBlue:
            return EColor_LtBlue;
        case ETileState_Yellow:
            return EColor_Pink;
        case ETileState_Purple:
            return EColor_Purple;
        case ETileState_Border:
            return EColor_Border;
    }
    return EColor_None;
}

typedef struct {
    IVec2 pos;
    IVec2 tiles[4];
    EColor color;
} Brick;

EColor get_color_from_shape(EBrickShape shape) {
    switch (shape) {
        case EBrickShape_Straight:
            return EColor_LtBlue;
        case EBrickShape_Square:
            return EColor_Pink;
        case EBrickShape_T:
            return EColor_Blue;
        case EBrickShape_LRight:
            return EColor_Purple;
        case EBrickShape_LLeft:
            return EColor_Orange;
        case EBrickShape_RSkew:
            return EColor_Red;
        case EBrickShape_LSkew:
            return EColor_Green;
    }
    return EColor_None;
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
        int32_t const x = new_pos.x + brick_tiles[i].x;
        int32_t const y = new_pos.y + brick_tiles[i].y;

        if (y >= GAME_TILES_HIGH) {
            printf("Bottom collision on (x,y) = (%i, %i)\n", x, y);
            return ECollision_Bottom;
        }

        if (x < 0 || x >= GAME_TILES_WIDE) {
            printf("Side collision on (x,y) = (%i, %i)\n", x, y);
            return ECollision_Side;
        }

        int32_t index = y * GAME_TILES_WIDE + x;
        if (index >= NUM_TILES) {
            printf("Received out-of-bounds index %i\n", index);
            assert(NULL);
        }
        if (tiles[index] != ETileState_None) {
            return ECollision_Bottom;
        }
    }

    return ECollision_None;
}

Brick create_brick(EBrickShape shape) {
    Brick brick = {0};
    brick.color = get_color_from_shape(shape);
    brick.pos = (IVec2){rand() % GAME_TILES_WIDE, 0};

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
            brick.tiles[0] = (IVec2){.x = -1, .y = 0};
            brick.tiles[1] = (IVec2){.x = 0, .y = 0};
            brick.tiles[2] = (IVec2){.x = 0, .y = 1};
            brick.tiles[3] = (IVec2){.x = 1, .y = 1};
        } break;
    }
    return brick;
}

void draw_brick(Brick const* brick) {
    for (int i = 0; i < 4; i++) {
        IVec2 pos = ivec2_add(brick->pos, brick->tiles[i]);
        render_draw_tile(pos.x, pos.y, brick->color);
    }
}

IVec2 tile_index_to_pos(int32_t index) {
    int32_t x = index % GAME_TILES_WIDE;
    int32_t y = index / GAME_TILES_WIDE;
    return (IVec2){.x = x, .y = y};
}

int32_t pos_to_tile_index(IVec2 pos) { return pos.y * GAME_TILES_WIDE + pos.x; }

void draw_tiles(ETileState tiles[]) {
    for (int i = 0; i < NUM_TILES; i++) {
        if (tiles[i] == ETileState_None) {
            continue;
        }

        EColor color = get_color_from_state(tiles[i]);
        IVec2 pos = tile_index_to_pos(i);
        render_draw_tile(pos.x, pos.y, color);
    }
}

void game_handle_touchdown(GameState* game) {

    // 1. Move brick tiles to game.tiles
    for (int32_t i = 0; i < 4; i++) {
        IVec2 const pos =
            ivec2_add(game->current_brick.tiles[i], game->current_brick.pos);
        int32_t tile_index = pos_to_tile_index(pos);
        game->tiles[tile_index] = ETileState_Green;
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
        assert(NULL);
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

// void pack_brick_tiles(IVec2 brick_tiles[]) {
//     int32_t min_y = INT32_MAX;
//     for (int i = 0; i < 4; i++) {
//         min_y = min(min_y, brick_tiles[i].y);
//     }

//     for (int i = 0; i < 4; i++) {
//         brick_tiles[i].y = brick_tiles[i].y - min_y;
//     }
// }

void brick_rotate(GameState* game, ERotation rot) {
    IVec2 new_tiles[4] = {0};
    for (int i = 0; i < 4; i++) {
        IVec2 const tile_pos = game->current_brick.tiles[i];
        if (rot == ERotation_CW) {
            new_tiles[i] = ivec2_rotate_cw(tile_pos);
        } else {
            new_tiles[i] = ivec2_rotate_ccw(tile_pos);
        }
    }

    ECollision const collision =
        tiles_check_collision(game->tiles, new_tiles, game->current_brick.pos);
    if (collision == ECollision_Bottom) {
        return;
    }

    // Move sideways until we fit
    if (collision == ECollision_Side) {
        int32_t offset[4] = {1, -1, 2, -2};
        for (int32_t i = 0; i < 4; i++) {
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

int main(void) {
    if (render_init() != 0) {
        printf("%s\n", SDL_GetError());
        goto quit;
    }

    GameState game = {0};
    game.current_brick = create_brick(EBrickShape_Straight);

    uint32_t delta_ticks = 0;
    uint32_t const target_frame_ticks = 16;

    // SDL_AddTimer(800, game_movement_callback, &game);

    SDL_Event event = {0};
    while (1) {
        uint32_t const frame_start = SDL_GetTicks();

        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT: {
                goto quit;
            }
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
                        brick_rotate(&game, ERotation_CCW);
                    } break;
                    case SDLK_x: {
                        brick_rotate(&game, ERotation_CW);
                    }
                }
            } break;
        }

        render_draw_background();
        draw_tiles(game.tiles);
        draw_brick(&game.current_brick);

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
