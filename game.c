#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>

// O tamanho do "Tabuleiro" em quantidade de quadrados que a cobra está
#define MATRIX_WIDTH 30
#define MATRIX_HEIGHT 12

// O tamanho de cada celula("Quadradinho") do "Tabuleiro" na tela
// (Isso foi feito para melhorar a legibilidade)
#define CELL_WIDTH WINDOW_WIDTH/MATRIX_WIDTH
#define CELL_HEIGHT WINDOW_HEIGHT/MATRIX_HEIGHT

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

// Definição de funçoes que retornam a posição
// do centro da celula na tela em função da
// posição dele na matrix.
// São usadas na renderização de cada celula
int MatrixToWindowX(int _matrixX) {return _matrixX*CELL_WIDTH + CELL_WIDTH/2;}
int MatrixToWindowY(int _matrixY) {return WINDOW_HEIGHT - (_matrixY * CELL_HEIGHT + CELL_HEIGHT/2);}


// Definição da posição da cabeça da cobra na matrix
int snake_headX;
int snake_headY;

// Definição da posição da cauda da cobra na matrix
int snake_tailX;
int snake_tailY;

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

  // Colocando uma fruta no mapa para testagem
  mapMatrix[28][10].type = FRUIT_TILE;
}

  void process_input() {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) game_is_running = FALSE;
            if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
                if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != DOWN) {
                    mapMatrix[snake_headX][snake_headY].snake.forwardDirection = UP;
                }
            }
            if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != RIGHT) {
                    mapMatrix[snake_headX][snake_headY].snake.forwardDirection = LEFT;
                }
            }
            if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
                if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != UP) {
                    mapMatrix[snake_headX][snake_headY].snake.forwardDirection = DOWN;
                }
            }
            if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != LEFT) {
                    mapMatrix[snake_headX][snake_headY].snake.forwardDirection = RIGHT;
                }
            }
            break;
    }
}

void update()
{

}
void render(SDL_Renderer* renderer)
{
  // Percorre toda matriz
  for(int i = 0; i < MATRIX_WIDTH; i++)
  {
    for(int j = 0; j < MATRIX_HEIGHT; j++)
    {
      // Pega a posição na tela da célula atual
      int window_x = MatrixToWindowX(i);
      int window_y = MatrixToWindowY(j);

      // Define um quadrado na posição da célula
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
          // Coloca a cor a ser desenhada (Preta)
          SDL_SetRenderDrawColor(renderer,
            0,
            0,
            0,
            255);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);
          break;


        case SNAKE_TILE:
          // Coloca a cor a ser desenhada (Verde)
          SDL_SetRenderDrawColor(renderer,
            0,
            255,
            0,
            255);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);

          // Coloca a cor a ser desenhada (Vermelha)
          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            0,
            255);

          // Desenha uma linha indicando a direção do pedaço da cobra
          switch (mapMatrix[i][j].snake.forwardDirection)
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
          // Coloca a cor a ser desenhada (Vermelha)
          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            0,
            255);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);
          break;


        default:
          // Coloca a cor a ser desenhada (Mangenta)
          SDL_SetRenderDrawColor(renderer,
            255,
            0,
            255,
            255);

          // Desenha um quadrado na célula
          SDL_RenderFillRect(renderer, &rect);
          break;
      }
    }
  }

  SDL_RenderPresent(renderer);
}
