#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

// O tamanho do "Tabuleiro" em quantidade de quadrados que a cobra está
#define MATRIX_WIDTH 20
#define MATRIX_HEIGHT 20

//Tamanho "ideal" que vai servir como base para comparar estimadamente o tamanho de cada quadradinho na tela
#define IDEAL_CELL_SIZE WINDOW_WIDTH/MATRIX_WIDTH


// MACROS com as coordenadas iniciais da cabeça e da cauda da cobra
#define SNAKE_TAILX 5
#define SNAKE_TAILY 4
#define SNAKE_HEADX 5
#define SNAKE_HEADY 3

// Variaveis para gerir o tempo de jogo para movimentação da cobrinha
Uint32 last_update_time = 0;
const Uint32 update_interval = 400;  // Intervalo em milissegundos (400ms)

// MACROS com os parametros de cores (RGB alpha)
#define RED 255,0,0,255
#define GREEN 0,255,0,255
#define MANGENTA 255,0,255,255
#define BLACK 0,0,0,255

// Variáveis para o viewport ajustado
static SDL_Rect game_viewport = {0};
static int cell_size = 0;  // Tamanho real das células (quadradas)

//Definindo variáveis para os sprites
SDL_Texture* snake_head_texture = NULL;
SDL_Texture* snake_body_texture = NULL;
SDL_Texture* fruit_texture = NULL;
SDL_Texture* bg_texture = NULL;

// Definido na main, quando 0 ou FALSE o jogo para após executar a renderização
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
int MatrixToWindowX(int _matrixX) {return _matrixX*cell_size;}
int MatrixToWindowY(int _matrixY) {return WINDOW_HEIGHT - (_matrixY * cell_size);}

// Definição da variavel que delega o delay do movimento da cobra
unsigned int last_movement_time = 0;

// Definição da posição da cabeça da cobra na matrix
int snake_headX;
int snake_headY;

// Definição da posição da cauda da cobra na matrix
int snake_tailX;
int snake_tailY;

// Tamanho da cobra em células
int snake_size;

// --Funções para renderização --

// Função para calcular a área vizualizada contida na janela
void calculate_viewport() {
  // Calcula o tamanho máximo possível mantendo a proporção
  int max_cell_width = WINDOW_WIDTH / MATRIX_WIDTH;
  int max_cell_height = WINDOW_HEIGHT / MATRIX_HEIGHT;

  // Usa o menor valor para manter células quadradas
  cell_size = (max_cell_width < max_cell_height) ? max_cell_width : max_cell_height;

  // Calcula a área centralizada
  int game_width = MATRIX_WIDTH * cell_size;
  int game_height = MATRIX_HEIGHT * cell_size;

  game_viewport.x = (WINDOW_WIDTH - game_width) / 2;
  game_viewport.y = (WINDOW_HEIGHT - game_height) / 2;
  game_viewport.w = game_width;
  game_viewport.h = game_height;
}
// Função para carregar os sprites
void load_textures(SDL_Renderer* renderer) {
  // Carrega a textura da cabeça da cobra
  SDL_Surface* surface = IMG_Load("assets/snake_head.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar snake_head.png: %s\n", IMG_GetError());
      return;
  }
  snake_head_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura do corpo da cobra
  surface = IMG_Load("assets/snake_body.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar snake_body.png: %s\n", IMG_GetError());
      return;
  }
  snake_body_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura da fruta
  surface = IMG_Load("assets/fruit.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar fruit.png: %s\n", IMG_GetError());
      return;
  }
  fruit_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura do Background
  surface = IMG_Load("assets/bg.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar bg.png: %s\n", IMG_GetError());
      return;
  }
  bg_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
}
void cleanup_textures() {
  if (snake_head_texture) SDL_DestroyTexture(snake_head_texture);
  if (snake_body_texture) SDL_DestroyTexture(snake_body_texture);
  if (fruit_texture) SDL_DestroyTexture(fruit_texture);
  if (bg_texture) SDL_DestroyTexture(bg_texture);
  IMG_Quit();
}
// Definição de uma função que dá as específicações
// de desenho de um retângulo
SDL_Rect rectFromCellPos(int cell_posX, int cell_posY) {
  return (SDL_Rect){
      cell_posX * cell_size,
      (MATRIX_HEIGHT - 1 - cell_posY) * cell_size,  // Inverte Y
      cell_size,
      cell_size
  };
}

// --Game Loop--

void setup()
{
  calculate_viewport();
  // Inicializa SDL_image
  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
      fprintf(stderr, "SDL_image não pôde inicializar! SDL_image Error: %s\n", IMG_GetError());
      game_is_running = FALSE;
      return;
  }
  //Pegando o tick inicial, como um "millis" do sdl
  last_update_time = SDL_GetTicks();

  // Iniciando toda a matriz como vazia
  for(int i = 0; i < MATRIX_WIDTH; i++)
  {
    for(int j = 0; j < MATRIX_HEIGHT; j++)
    {
      mapMatrix[i][j].type = EMPTY_TILE;
    }
  }

  // Iniciando posição da cabeça da cobra
  snake_headX = SNAKE_HEADX;
  snake_headY = SNAKE_HEADY;

  // Iniciando posição da cauda da cobra
  snake_tailX = SNAKE_TAILX;
  snake_tailY = SNAKE_TAILY;

  // Iniciando as posições iniciais da cobra
  mapMatrix[5][4].snake = (snakeTile){SNAKE_TILE,DOWN};
  mapMatrix[5][3].snake = (snakeTile){SNAKE_TILE,DOWN};

  snake_size = 2;

  // Colocando uma fruta no mapa para testagem
  mapMatrix[28][10].type = FRUIT_TILE;
  mapMatrix[26][8].type = FRUIT_TILE;
  mapMatrix[24][6].type = FRUIT_TILE;

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

          //Captura as teclas UP/w , DOWN/s , LEFT/a , RIGHT/d e muda a direção da cabeça da cobra
          if (event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_UP) {
              // Se a posição da cobra for contrária a da tecla, não move
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
    // Verifica se já passou o tempo necessário para a próxima atualização
    Uint32 current_time = SDL_GetTicks();
    if (current_time - last_update_time < update_interval) {
        return;
    }
    last_update_time = current_time;
    // Move a cobra baseada na sua direção
    int new_headX = snake_headX;
    int new_headY = snake_headY;

    switch (mapMatrix[snake_headX][snake_headY].snake.forwardDirection)
    {
        case UP:
            new_headY++;
            break;
        case DOWN:
            new_headY--;
            break;
        case LEFT:
            new_headX--;
            break;
        case RIGHT:
            new_headX++;
            break;
    }

    // Caso a cobra bata na parede
    if (new_headX < 0 || new_headX >= MATRIX_WIDTH || new_headY < 0 || new_headY >= MATRIX_HEIGHT) {
        return;
    }

    // Caso a nova posição da cabeça seja o corpo da cobra
    if (mapMatrix[new_headX][new_headY].type == SNAKE_TILE) {
      // Implementar código para colidir com o corpo
  }

    // Caso a nova posição da cabeça seja uma fruta
    if (mapMatrix[new_headX][new_headY].type == FRUIT_TILE) {
        // Implementar código para comer fruta
    } else {
        // Move a cobra deslocando o corpo

        // Encontra a próxima posição da cauda
        int next_tailX = snake_tailX;
        int next_tailY = snake_tailY;

        switch (mapMatrix[snake_tailX][snake_tailY].snake.forwardDirection) {
            case UP:
                next_tailY++;
                break;
            case DOWN:
                next_tailY--;
                break;
            case LEFT:
                next_tailX--;
                break;
            case RIGHT:
                next_tailX++;
                break;
        }

        // Limpa a posição da cauda
        mapMatrix[snake_tailX][snake_tailY].type = EMPTY_TILE;

        // Atualiza a posição da cauda
        snake_tailX = next_tailX;
        snake_tailY = next_tailY;
    }

    // Atualiza a posição da cabeça
    mapMatrix[new_headX][new_headY].snake = (snakeTile){SNAKE_TILE, mapMatrix[snake_headX][snake_headY].snake.forwardDirection};
    snake_headX = new_headX;
    snake_headY = new_headY;
}
void render(SDL_Renderer* renderer) {
  // Limpa toda a tela com preto
  SDL_SetRenderDrawColor(renderer, BLACK);
  SDL_RenderClear(renderer);

  // Define a área de renderização do jogo
  SDL_RenderSetViewport(renderer, &game_viewport);

  // ======= LAYERS (Camadas de renderização) =======

  /* Layers não são literalmente programadas, mas por consequência da dinâmica,
   uma render a frente se sobrepõe a uma anterior, portanto a separação em camadas
   é puramente para organização!
  */

  // ===== Layer 0 =====

  // Renderiza o background
  for(int x = 0; x < MATRIX_WIDTH; x++) {
    for(int y = 0; y < MATRIX_HEIGHT; y++) {
        SDL_Rect dest_rect = { // Resumindo é a posição literal na tela (considerando a resolução)
          x * cell_size,
            (MATRIX_HEIGHT - 1 - y) * cell_size,
            cell_size,
            cell_size
        };
        SDL_RenderCopy(renderer, bg_texture, NULL, &dest_rect);
    }
}

  // ====== Layer 1 ======

  // Renderiza a cobra
  int cell_posX = snake_tailX;
  int cell_posY = snake_tailY;
  for(int i = 0; i < snake_size; i++) {
      if(mapMatrix[cell_posX][cell_posY].type != SNAKE_TILE) {
          fprintf(stderr, "Erro: Segmento de cobra faltando em [%d][%d]\n", cell_posX, cell_posY);
          game_is_running = FALSE;
          break;
      }

      SDL_Rect dest_rect = { // Resumindo é a posição literal na tela (considerando a resolução)
          cell_posX * cell_size,
          (MATRIX_HEIGHT - 1 - cell_posY) * cell_size,
          cell_size,
          cell_size
      };

      // Renderiza cabeça ou corpo
      if (i == snake_size-1) { // Cabeça
          SDL_RenderCopy(renderer, snake_head_texture, NULL, &dest_rect);
      } else { // Corpo
          SDL_RenderCopy(renderer, snake_body_texture, NULL, &dest_rect);
      }

      // Move para o próximo segmento
      switch (mapMatrix[cell_posX][cell_posY].snake.forwardDirection) {
          case UP: cell_posY++; break;
          case DOWN: cell_posY--; break;
          case LEFT: cell_posX--; break;
          case RIGHT: cell_posX++; break;
      }
  }

  // Renderiza as frutas
  for(int x = 0; x < MATRIX_WIDTH; x++) {
      for(int y = 0; y < MATRIX_HEIGHT; y++) {
          if(mapMatrix[x][y].type == FRUIT_TILE) {
              SDL_Rect dest_rect = { // Resumindo é a posição literal na tela (considerando a resolução)
                  x * cell_size,
                  (MATRIX_HEIGHT - 1 - y) * cell_size,
                  cell_size,
                  cell_size
              };
              SDL_RenderCopy(renderer, fruit_texture, NULL, &dest_rect);
          }
      }
  }

  // Restaura o viewport padrão para a janela toda
  SDL_RenderSetViewport(renderer, NULL);
  SDL_RenderPresent(renderer);
}
