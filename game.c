#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

#define MATRIX_WIDTH 30
#define MATRIX_HEIGHT 12

#define CELL_WIDTH WINDOW_WIDTH/MATRIX_WIDTH
#define CELL_HEIGHT WINDOW_HEIGHT/MATRIX_HEIGHT

extern int game_is_running;

typedef enum mapTileType
{
  EMPTY_TILE,
  SNAKE_TILE,
  FRUIT_TILE
} mapTileType;

typedef enum direction {UP,DOWN,LEFT,RIGHT} direction;

typedef struct snakeTile
{
  mapTileType type;
  direction nextPart;
} snakeTile;

typedef union mapTile
{
  mapTileType type;
  snakeTile snake;

} mapTile;

mapTile mapMatrix[MATRIX_WIDTH][MATRIX_HEIGHT];

int MatrixToWindowX(int _matrixX) {return _matrixX*CELL_WIDTH + CELL_WIDTH/2;}
int MatrixToWindowY(int _matrixY) {return WINDOW_HEIGHT - (_matrixY * CELL_HEIGHT + CELL_HEIGHT/2);}

int snake_headX;
int snake_headY;

int snake_tailX;
int snake_tailY;

void setup()
{
  for(int i = 0; i < MATRIX_WIDTH; i++)
  {
    for(int j = 0; j < MATRIX_HEIGHT; j++)
    {
      mapMatrix[i][j].type = EMPTY_TILE;
    }
  }

  snake_headX = 5;
  snake_headY = 3;

  snake_tailX = 2;
  snake_tailY = 2;

  mapMatrix[2][2].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[2][3].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[2][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[3][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[4][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[5][4].snake = (snakeTile){SNAKE_TILE,DOWN};
  mapMatrix[5][3].snake = (snakeTile){SNAKE_TILE,DOWN};

  mapMatrix[28][10].type = FRUIT_TILE;
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
  for(int i = 0; i < MATRIX_WIDTH; i++)
  {
    for(int j = 0; j < MATRIX_HEIGHT; j++)
    {
      int window_x = MatrixToWindowX(i);
      int window_y = MatrixToWindowY(j);

      SDL_Rect rect =
      {
        window_x - CELL_WIDTH/2,
        window_y - CELL_HEIGHT/2,
        CELL_WIDTH,
        CELL_HEIGHT
      };
      switch (mapMatrix[i][j].type)
      {
        case EMPTY_TILE:
          SDL_SetRenderDrawColor(renderer,
            0,
            0,
            0,
            255);
          SDL_RenderFillRect(renderer, &rect);
          break;


        case SNAKE_TILE:
          SDL_SetRenderDrawColor(renderer,
            0,
            255,
            0,
            255);
          SDL_RenderFillRect(renderer, &rect);

          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            0,
            255);
          switch (mapMatrix[i][j].snake.nextPart)
          {
            case UP:
              SDL_RenderDrawLine(renderer, window_x, window_y,
                window_x, window_y - CELL_HEIGHT/2);
              break;
            case DOWN:
              SDL_RenderDrawLine(renderer, window_x, window_y,
                window_x, window_y + CELL_HEIGHT/2);
              break;
            case LEFT:
              SDL_RenderDrawLine(renderer, window_x, window_y,
                window_x - CELL_WIDTH/2, window_y);
              break;
            case RIGHT:
              SDL_RenderDrawLine(renderer, window_x, window_y,
                window_x + CELL_WIDTH/2, window_y);
              break;
          }
          break;


        case FRUIT_TILE:
          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            0,
            255);
          SDL_RenderFillRect(renderer, &rect);
          break;


        default:
          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            255,
            255);
          SDL_RenderFillRect(renderer, &rect);
          break;
      }
    }
  }

  SDL_RenderPresent(renderer);
}
