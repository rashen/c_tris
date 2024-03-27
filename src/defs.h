#ifndef C_TRIS_DEFS_H_
#define C_TRIS_DEFS_H_

#define N_ELEMENTS(X) (sizeof(X) / sizeof(*(X)))

/* Dimensions:
Width of the game board is TILE_SIZE * GAMES_TILES_WIDE = 80. To center this the
drawing rect should start at 240/2 - 80/2 = 80; This splits the window into
three equally sized parts.
*/
#define TILE_SIZE 8
#define GAME_TILES_WIDE 10
#define GAME_TILES_HIGH 16 // This is the whole height

// Taken from Tic80
#define UNSCALED_WINDOW_WIDTH 240
#define UNSCALED_WINDOW_HEIGHT 136
#define DPI 4

#endif
