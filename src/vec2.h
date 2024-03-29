#ifndef C_TRIS_VEC2_H_
#define C_TRIS_VEC2_H_

#include "defs.h"

#include <assert.h>
#include <stdint.h>

static_assert(sizeof(f32_t) == 4, "Misleading type name");
static_assert(sizeof(f64_t) == 8, "Misleading type name");

typedef struct {
    int32_t x;
    int32_t y;
} IVec2;

IVec2 ivec2_add(IVec2 lhs, IVec2 rhs) {
    return (IVec2){.x = lhs.x + rhs.x, .y = lhs.y + rhs.y};
}

IVec2 ivec2_rotate_cw(IVec2 vec2) {
    IVec2 const rotated = {.x = -vec2.y, .y = vec2.x};
    return rotated;
}

IVec2 ivec2_rotate_ccw(IVec2 vec2) {
    IVec2 const rotated = {.x = vec2.y, .y = -vec2.x};
    return rotated;
}

#endif
