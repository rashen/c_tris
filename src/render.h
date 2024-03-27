#ifndef CTRIS_RENDER_H_
#define CTRIS_RENDER_H_

#include <SDL_render.h>

typedef int32_t TextureHandle;
typedef SDL_Color Pixel;

int32_t render_init(int32_t window_width, int32_t window_height, float dpi,
                    int32_t game_tiles_wide, int32_t game_tiles_high,
                    int32_t tile_size);
void render_drop();

void render_draw_background();
void render_draw_tile(int32_t x_pos, int32_t y_pos, SDL_Color color);
void render_draw_board();

void render_present();

#endif
