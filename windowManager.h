#pragma once

#include <SDL2/SDL.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;

void destroy_window();
int initialize_window(void);
