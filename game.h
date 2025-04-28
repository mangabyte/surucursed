#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

extern int last_frame_time;

void setup();
void process_input();
void update();
void render(SDL_Renderer* renderer);
void load_textures(SDL_Renderer* renderer);
void cleanup_textures(void);
