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
SDL_Texture* snake_tail_texture = NULL;
SDL_Texture* snake_curve_o_texture = NULL; // Tipo de curva para fora (O-utside), como: | Tipo de curva para dentro (I-nside)
SDL_Texture* snake_curve_i_texture = NULL; //               ⌟ ⌞                                        ⌜ ⌝
SDL_Texture* fruit_texture = NULL;         //               ⌝ ⌜                                        ⌞ ⌟
SDL_Texture* background_texture = NULL;            //  Pelo menos quando for implementada as animações, por enquanto é só um flip, mas
                                           //  com os arquivos já implementados para modificação mínima futura

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

// Definição da variavel que delega o delay do movimento da cobra
unsigned int last_movement_time = 0;

// Definição da posição da cabeça da cobra na matrix
int snake_headX;
int snake_headY;

// Definição da posição da cauda da cobra na matrix
int snake_tailX;
int snake_tailY;

// Head direction
direction head_dir = UP;

// Tamanho da cobra em células
int snake_size;

// Enum de verificação do tipo de célula da cobra
typedef enum {
  SEGMENT_STRAIGHT,
  SEGMENT_CURVE,
  SEGMENT_TAIL,
  SEGMENT_HEAD
} SegmentType;

// Função para determinar tipo da célula
SegmentType determine_segment_type(int x, int y, int is_head, int is_tail, int prev_dir) {
  if (is_head) return SEGMENT_HEAD;
  if (is_tail) return SEGMENT_TAIL;

  direction current_dir = mapMatrix[x][y].snake.forwardDirection;

  // Verifica se há mudança de direção em relação ao segmento anterior
  if (prev_dir != -1 && current_dir != prev_dir) {
      return SEGMENT_CURVE;
  }

  return SEGMENT_STRAIGHT;
}

// Função que calcula o angulo de um segmento curvo
int calculate_curve_angle(direction prev_dir, direction current_dir) {
  // Mapeia combinações de direções para ângulos específicos
  if ((prev_dir == RIGHT && current_dir == UP) || (prev_dir == DOWN && current_dir == LEFT)) {
      return 1;    // Curva de direita para cima ou baixo para esquerda
  }
  if ((prev_dir == LEFT && current_dir == UP) || (prev_dir == DOWN && current_dir == RIGHT)) {
      return 2;   // Curva de esquerda para cima ou baixo para direita
  }
  if ((prev_dir == RIGHT && current_dir == DOWN) || (prev_dir == UP && current_dir == LEFT)) {
      return 3;  // Curva de direita para baixo ou cima para esquerda
  }
  if ((prev_dir == LEFT && current_dir == DOWN) || (prev_dir == UP && current_dir == RIGHT)) {
      return 4;  // Curva de esquerda para baixo ou cima para direita
  }
  return 0; // Padrão (não deveria acontecer)
}
// --Funções para renderização --

// Função para calcular e inicializar a área vizualizada contida na janela
void initialize_viewport() {
  // Calcula o tamanho máximo possível mantendo a proporção
  int max_cell_width = WINDOW_WIDTH / MATRIX_WIDTH;
  int max_cell_height = WINDOW_HEIGHT / MATRIX_HEIGHT;

  // Usa o menor valor para manter células quadradas
  cell_size = (max_cell_width < max_cell_height) ? max_cell_width : max_cell_height;

  // Calcula a área útil do jogo
  int game_width = MATRIX_WIDTH * cell_size;
  int game_height = MATRIX_HEIGHT * cell_size;

  // Colocando no struct as coordenadas de onde será feita a área útil
  game_viewport.x = (WINDOW_WIDTH - game_width) / 2;
  game_viewport.y = (WINDOW_HEIGHT - game_height) / 2;

  // Colocando o tamanho da tela útil no struct
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

  // Carrega a textura da cauda da cobra
  surface = IMG_Load("assets/snake_tail.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar snake_tail.png: %s\n", IMG_GetError());
      return;
  }
  snake_tail_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura da curva para fora
  surface = IMG_Load("assets/snake_curve_o.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar snake_curve_o.png: %s\n", IMG_GetError());
      return;
  }
  snake_curve_o_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura da curva para dentro
  surface = IMG_Load("assets/snake_curve_i.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar snake_curve_i.png: %s\n", IMG_GetError());
      return;
  }
  snake_curve_i_texture = SDL_CreateTextureFromSurface(renderer, surface);
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
  surface = IMG_Load("assets/background.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar background.png: %s\n", IMG_GetError());
      return;
  }
  background_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
}
void cleanup_textures() {
  if (snake_head_texture) SDL_DestroyTexture(snake_head_texture);
  if (snake_body_texture) SDL_DestroyTexture(snake_body_texture);
  if (snake_curve_o_texture) SDL_DestroyTexture(snake_curve_o_texture);
  if (snake_curve_i_texture) SDL_DestroyTexture(snake_curve_i_texture);
  if (snake_tail_texture) SDL_DestroyTexture(snake_tail_texture);
  if (fruit_texture) SDL_DestroyTexture(fruit_texture);
  if (background_texture) SDL_DestroyTexture(background_texture);
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
  srand(SDL_GetTicks()); // iniciando número aleatório para geração de fruta
  initialize_viewport();
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
  snake_tailX = SNAKE_HEADX;
  snake_tailY = SNAKE_HEADY-2;

  // Iniciando as posições iniciais da cobra
  mapMatrix[snake_headX][snake_headY].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[snake_headX][snake_headY-1].snake = (snakeTile){SNAKE_TILE,UP};
  mapMatrix[snake_headX][snake_headY-2].snake = (snakeTile){SNAKE_TILE,UP};


  snake_size = 3;

  // Criando as primeiras frutas em posições aleatórias
  for (int i = 0; i < 1; i++) { // i < x = quantas frutas existirão no mapa
    int fruitX, fruitY;
    do {
        fruitX = rand() % MATRIX_WIDTH;
        fruitY = rand() % MATRIX_HEIGHT;
    } while (mapMatrix[fruitX][fruitY].type != EMPTY_TILE);

    mapMatrix[fruitX][fruitY].type = FRUIT_TILE;
}

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
                  head_dir = UP;
              }
          }
          if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
              if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != RIGHT) {
                head_dir = LEFT;
              }
          }
          if (event.key.keysym.sym == SDLK_s || event.key.keysym.sym == SDLK_DOWN) {
              if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != UP) {
                head_dir = DOWN;
              }
          }
          if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
              if (mapMatrix[snake_headX][snake_headY].snake.forwardDirection != LEFT) {
                head_dir = RIGHT;
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

    switch (head_dir)
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
      // Aumenta o tamanho da cobra
      snake_size++;

      // Remove a fruta comida
      mapMatrix[new_headX][new_headY].type = EMPTY_TILE;

      // Gera uma nova fruta em posição aleatória
      int fruitX, fruitY;
      do {
          fruitX = rand() % MATRIX_WIDTH;
          fruitY = rand() % MATRIX_HEIGHT;
      } while (mapMatrix[fruitX][fruitY].type != EMPTY_TILE);

      mapMatrix[fruitX][fruitY].type = FRUIT_TILE;
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
    mapMatrix[snake_headX][snake_headY].snake.forwardDirection = head_dir;
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
        SDL_Rect dest_rect = rectFromCellPos(x, y);
        SDL_RenderCopy(renderer, background_texture, NULL, &dest_rect);
    }
}

  // ====== Layer 1 ======

  // Renderiza a cobra
int cell_posX = snake_tailX;
int cell_posY = snake_tailY;
int prev_direction = -1; // Direção do segmento anterior

for(int i = 0; i < snake_size; i++) {
    if(mapMatrix[cell_posX][cell_posY].type != SNAKE_TILE) {
        fprintf(stderr, "Erro: Segmento de cobra faltando em [%d][%d]\n", cell_posX, cell_posY);
        game_is_running = FALSE;
        break;
    }

    SDL_Rect dest_rect = rectFromCellPos(cell_posX, cell_posY);

    // Determina o tipo de segmento
    SegmentType seg_type = determine_segment_type(
        cell_posX, cell_posY,
        (i == snake_size-1), // Verifica se o segmento é uma cabeça
        (i == 0),            // Verifica se o segmento é uma cauda
        prev_direction       // Direção do segmento anterior
    );
      // Renderiza o segmento apropriado
      switch(seg_type) {
        case SEGMENT_HEAD: {
            double angle = 0;
            switch(mapMatrix[cell_posX][cell_posY].snake.forwardDirection) {
                case UP: angle = 0; break;
                case RIGHT: angle = 90; break;
                case DOWN: angle = 180; break;
                case LEFT: angle = 270; break;
            }
            SDL_RenderCopyEx(renderer, snake_head_texture, NULL, &dest_rect, angle, NULL, SDL_FLIP_NONE);
            break;
        }
        case SEGMENT_TAIL: {
            // Encontre a direção da cauda (oposto da direção do segmento anterior)
            direction tail_dir = mapMatrix[cell_posX][cell_posY].snake.forwardDirection;
            double angle = 0;
            switch(tail_dir) {
              case UP: angle = 0; break;
              case RIGHT: angle = 90; break;
              case DOWN: angle = 180; break;
              case LEFT: angle = 270; break;
            }
            SDL_RenderCopyEx(renderer, snake_tail_texture, NULL, &dest_rect, angle, NULL, SDL_FLIP_NONE);
            break;
        }
        case SEGMENT_CURVE: {
            switch(calculate_curve_angle(prev_direction, mapMatrix[cell_posX][cell_posY].snake.forwardDirection)){
              case 1: // Direita - Cima
              SDL_RenderCopyEx(renderer, snake_curve_i_texture, NULL, &dest_rect, 0, NULL, SDL_FLIP_NONE);
              break;
              case 2: // Esquerda - Cima
              SDL_RenderCopyEx(renderer, snake_curve_o_texture, NULL, &dest_rect, 0, NULL, SDL_FLIP_NONE);
              break;
              case 3: // Direita - Baixo
              SDL_RenderCopyEx(renderer, snake_curve_o_texture, NULL, &dest_rect, 180, NULL, SDL_FLIP_NONE);
              break;
              case 4: // Esquerda - Baixo
              SDL_RenderCopyEx(renderer, snake_curve_i_texture, NULL, &dest_rect, 180, NULL, SDL_FLIP_NONE);
              break;
            }
      break;
      }
        case SEGMENT_STRAIGHT: {
            double angle = 0;
            switch(mapMatrix[cell_posX][cell_posY].snake.forwardDirection) {
                case UP: angle = 0; break;
                case RIGHT: angle = 90; break;
                case DOWN: angle = 180; break;
                case LEFT: angle = 270; break;
            }
            SDL_RenderCopyEx(renderer, snake_body_texture, NULL, &dest_rect, angle, NULL, SDL_FLIP_NONE);
            break;
        }

    }

    // Atualiza a direção anterior para o próximo segmento
    prev_direction = mapMatrix[cell_posX][cell_posY].snake.forwardDirection;

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
              SDL_Rect dest_rect = rectFromCellPos(x, y);
              SDL_RenderCopy(renderer, fruit_texture, NULL, &dest_rect);
          }
      }
  }

  // Restaura o viewport padrão para a janela toda
  SDL_RenderSetViewport(renderer, NULL);
  SDL_RenderPresent(renderer);
}
