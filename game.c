#include "./game.h"
#include "./constants.h"

extern int game_is_running;

void setup()
{

}
void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) game_is_running = FALSE;
            break;
    }
}
void update()
{
    
}
void render(SDL_Renderer* renderer)
{
    unsigned int ticks = SDL_GetTicks();
    SDL_SetRenderDrawColor(renderer,
    (ticks/100)%255,
    (ticks/100 + 85)%255,
    (ticks/100 + 170)%255,
    255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}