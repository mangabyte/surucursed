#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>

// O tamanho do "Tabuleiro" em quantidade de quadrados que a cobra está
#define MATRIX_WIDTH 30
#define MATRIX_HEIGHT 12

// O tamanho de cada celula("Quadradinho") do "Tabuleiro" na tela
// (Isso foi feito para melhorar a legibilidade)
#define CELL_WIDTH WINDOW_WIDTH/MATRIX_WIDTH
#define CELL_HEIGHT WINDOW_HEIGHT/MATRIX_HEIGHT

// MACROS com os parametros de cores (RGB alpha)
#define RED 255,0,0,255
#define GREEN 0,255,0,255
#define MANGENTA 255,0,255,255
#define BLACK 0,0,0,255

// Definido na main, quando 0 o jogo para após executar a renderização
extern int game_is_running;

// Definindo o enum indica o que
// cada celula do mapa pode assumir
typedef enum mapTileType
{
  EMPTY_TILE,
  SNAKE_TILE,
  FRUIT_TILE
} mapTileType;

// Definindo o enum que indica as direções
typedef enum direction {UP,DOWN,LEFT,RIGHT} direction;

// Definindo o struct que indica o que cada
// celula do tabuleiro que a cobra ocupa deve ter
typedef struct snakeTile
{
  mapTileType type;
  direction forwardDirection;
} snakeTile;

// Definindo o union que indica o que
// uma certa celula do mapa tem
typedef union mapTile
{
  mapTileType type;
  snakeTile snake;

} mapTile;

// Matriz que armazena todas as celulas
// do mapa, a posição [0][0] é o canto
// inferior esquerdo
mapTile mapMatrix[MATRIX_WIDTH][MATRIX_HEIGHT];

// Definição da posição da cabeça da cobra na matrix
int snake_headX;
int snake_headY;

// Definição da posição da cauda da cobra na matrix
int snake_tailX;
int snake_tailY;

// Tamanho da cobra em células
int snake_size;

// --Funções para renderização --

// Definição de funçoes que retornam a posição
// do centro da celula na tela em função da
// posição dele na matrix.
int MatrixToWindowX(int _matrixX) {return _matrixX*CELL_WIDTH + CELL_WIDTH/2;}
int MatrixToWindowY(int _matrixY) {return WINDOW_HEIGHT - (_matrixY * CELL_HEIGHT + CELL_HEIGHT/2);}

// Definição de uma função que dá as específicações
// de desenho de um retângulo
SDL_Rect rectFromCellPos(int cell_posX, int cell_posY)
{
  return (SDL_Rect){
    MatrixToWindowX(cell_posX) - CELL_WIDTH/2,
    MatrixToWindowY(cell_posY) - CELL_HEIGHT/2,
    CELL_WIDTH,
    CELL_HEIGHT
  };
}

void setup()
{
  // Iniciando toda a matriz como vazia
  for(int i = 0; i < MATRIX_WIDTH; i++)
  {
    for(int j = 0; j < MATRIX_HEIGHT; j++)
    {
      mapMatrix[i][j].type = EMPTY_TILE;
    }
  }

  // Iniciando posição da cabeça da cobra
  snake_headX = 5;
  snake_headY = 3;

  // Iniciando posição da cauda da cobra
  snake_tailX = 2;
  snake_tailY = 2;

  // Iniciando as posições iniciais da cobra
  mapMatrix[2][2].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[2][3].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[2][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[3][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[4][4].snake = (snakeTile){SNAKE_TILE,RIGHT};
  mapMatrix[5][4].snake = (snakeTile){SNAKE_TILE,DOWN};
  mapMatrix[5][3].snake = (snakeTile){SNAKE_TILE,DOWN};

  snake_size = 7;

  // Colocando uma fruta no mapa para testagem
  mapMatrix[28][10].type = FRUIT_TILE;
}
void process_input()
{
  // Auto-explicativo
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
  SDL_SetRenderDrawColor(renderer, BLACK);
  SDL_RenderClear(renderer);

  SDL_Rect rect;

  int cell_posX;
  int cell_posY;

  cell_posX = snake_tailX;
  cell_posY = snake_tailY;

  for(int i = 0; i < snake_size; i++)
  {
    if(mapMatrix[cell_posX][cell_posY].type != SNAKE_TILE)
    {
      rect = rectFromCellPos(cell_posX, cell_posY);

      fprintf(stderr,"A cobra termina antes do tamanho informado em [%d][%d].\n",cell_posX,cell_posY);

      game_is_running = FALSE;
      break;
    }
    rect = rectFromCellPos(cell_posX, cell_posY);

    // Coloca a cor a ser desenhada (Verde)
    SDL_SetRenderDrawColor(renderer, GREEN);

    // Desenha um quadrado na célula
    SDL_RenderFillRect(renderer, &rect);

    // Coloca a cor a ser desenhada (Vermelha)
    SDL_SetRenderDrawColor(renderer, RED);

    // Salva as direções da cobra
    // Move o ponteiro para a proxima casa
    char dirX = 0;
    char dirY = 0;

    switch (mapMatrix[cell_posX][cell_posY].snake.forwardDirection)
    {
      case UP:
        cell_posY++;
        dirY = 1;
        break;
      case DOWN:
        cell_posY--;
        dirY = -1;
        break;
      case LEFT:
        cell_posX--;
        dirX = -1;
        break;
      case RIGHT:
        cell_posX++;
        dirX = 1;
        break;
    }

    // Desenha as linhas que indicam a forwardDirection da atual parte da cobra
    SDL_RenderDrawLine(renderer,
      MatrixToWindowX(cell_posX - dirX),
      MatrixToWindowY(cell_posY - dirY),
      MatrixToWindowX(cell_posX - dirX) + dirX*( CELL_WIDTH/2),
      MatrixToWindowY(cell_posY - dirY) + dirY*(-CELL_HEIGHT/2)
    );
  }

  // Percorre toda matriz
  for(int cell_posX = 0; cell_posX < MATRIX_WIDTH; cell_posX++)
  {
    for(int cell_posY = 0; cell_posY < MATRIX_HEIGHT; cell_posY++)
    {
      // Define um quadrado na posição da célula
      rect = rectFromCellPos(cell_posX, cell_posY);

      switch (mapMatrix[cell_posX][cell_posY].type)
      {
        case EMPTY_TILE:
          break;

        case SNAKE_TILE:
          break;

        case FRUIT_TILE:
          // Coloca a cor a ser desenhada (Vermelha)
          SDL_SetRenderDrawColor(renderer, RED);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);
          break;

        default:
          // Coloca a cor a ser desenhada (Mangenta)
          SDL_SetRenderDrawColor(renderer, MANGENTA);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);
          break;
      }
    }
  }

  SDL_RenderPresent(renderer);
}
