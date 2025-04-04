#pragma once

#include <SDL2/SDL.h>

extern int last_frame_time;

void setup();
void process_input();
void update();
void render(SDL_Renderer* renderer);