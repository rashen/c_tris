#include "particles.h"

#include "log.h"
#include "render.h"

#include <SDL_stdinc.h>
#include <stdlib.h>

Particle g_particles[MAX_PARTICLES] = {0};

f32_t randf_in_range(f32_t min, f32_t max) {
    f32_t span = max - min;
    return min + (f32_t)rand() / ((f32_t)RAND_MAX / span);
}

int32_t particles_spawn(int32_t num, f32_t y, f32_t x_min, f32_t x_max) {
    int32_t num_spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && num_spawned <= num; i++) {
        if (g_particles[i].is_alive == false) {
            num_spawned++;

            f32_t const angle = randf_in_range(-1.f, 1.f); // Radians
            f32_t const speed = randf_in_range(MIN_VELOCITY, MAX_VELOCITY);

            g_particles[i] = (Particle){
                .lifetime = randf_in_range(MIN_LIFETIME, MAX_LIFETIME),
                .age = 0.0,
                .y = y,
                .x = randf_in_range(x_min, x_max),
                .x_vel = SDL_sinf(angle) * speed,
                .y_vel = -(SDL_cosf(angle) * speed),
                .is_alive = true};
        }
    }

    return num_spawned;
}

void particle_update(Particle* particle, f32_t delta_time) {

    particle->age += delta_time;
    if (particle->age > particle->lifetime) {
        particle->is_alive = false;
        return;
    }

    particle->x = particle->x_vel * delta_time;
    particle->y = particle->y_vel * delta_time;
    particle->y_vel += GRAVITY * delta_time;
}

void particles_update(f32_t delta_time) {
    for (int32_t i = 0; i < MAX_PARTICLES; i++) {
        if (g_particles[i].is_alive) {
            particle_update(&g_particles[i], delta_time);
            render_particle(g_particles[i].x, g_particles[i].y);
        }
    }
}
