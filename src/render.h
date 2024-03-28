#ifndef CTRIS_RENDER_H_
#define CTRIS_RENDER_H_

#include <SDL_render.h>

typedef int32_t TextureHandle;
typedef SDL_Color Pixel;

typedef enum {
    EColor_None = 0,
    EColor_Border,
    EColor_Red,
    EColor_Blue,
    EColor_LtBlue,
    EColor_Orange,
    EColor_Green,
    EColor_Pink,
    EColor_Purple,
    EColor_MAX
} EColor;

int32_t render_init(void);
void render_drop(void);

void render_draw_background(void);
void render_draw_tile(int32_t x_pos, int32_t y_pos, EColor color);
void render_draw_board(void);

void render_present(void);

#endif
