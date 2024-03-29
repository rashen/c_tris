#ifndef C_TRIS_PARTICLES_H_
#define C_TRIS_PARTICLES_H_

#include "defs.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define GRAVITY 9.8f
#define MIN_LIFETIME 1.f
#define MAX_LIFETIME 2.f
#define MIN_VELOCITY 0.5f
#define MAX_VELOCITY 2.f
#define MAX_PARTICLES 200

typedef struct {
    f32_t lifetime;
    f32_t age;
    f32_t x;
    f32_t y;
    f32_t x_vel;
    f32_t y_vel;
    bool is_alive;
} Particle;

int32_t particles_spawn(int32_t num, f32_t y, f32_t x_min, f32_t x_max);
void particles_update(f32_t delta_time);

#endif
