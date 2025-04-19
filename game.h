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

// Declarações das texturas
extern SDL_Texture* snake_head_texture;
extern SDL_Texture* snake_body_texture;
extern SDL_Texture* fruit_texture;
extern SDL_Texture* bg_texture;
