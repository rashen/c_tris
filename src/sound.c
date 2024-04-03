#include <assert.h>
#include <SDL_mixer.h>

Mix_Music* g_music = NULL;
Mix_Chunk* g_touchdown = NULL;

void sound_init(void) {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    g_touchdown = Mix_LoadWAV("assets/explosion.wav");
    g_music = Mix_LoadMUS("assets/connected.ogg");
    assert(g_music != NULL);
    Mix_PlayMusic(g_music, -1);
}

void sound_release(void) {
    Mix_FreeMusic(g_music);
    Mix_FreeChunk(g_touchdown);
    Mix_Quit();
}

void sound_touchdown(void) { Mix_PlayChannel(-1, g_touchdown, 0); }
