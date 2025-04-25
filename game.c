#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

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
int slashcount = 0;

// MACROS com os parametros de cores (RGB alpha)
#define RED 255,0,0,255
#define GREEN 0,255,0,255
#define MANGENTA 255,0,255,255
#define BLACK 0,0,0,255

// Variáveis para o viewport ajustado
static SDL_Rect game_viewport = {0};
static int cell_size = 0;  // Tamanho real das células (quadradas)

//Definindo variáveis para os sprites
SDL_Texture* CabecaBaixo_texture = NULL;
SDL_Texture* CabecaCima_texture = NULL;
SDL_Texture* CabecaDireita_texture = NULL;
SDL_Texture* CabecaEsquerda_texture = NULL;

SDL_Texture* CaudaBaixo_texture = NULL;
SDL_Texture* CaudaCima_texture = NULL;
SDL_Texture* CaudaDireita_texture = NULL;
SDL_Texture* CaudaEsquerda_texture = NULL;

SDL_Texture* CurvaBaixoDireita_texture  = NULL;
SDL_Texture* CurvaBaixoEsquerda_texture  = NULL;
SDL_Texture* CurvaCimaDireita_texture  = NULL;
SDL_Texture* CurvaCimaEsquerda_texture  = NULL;

SDL_Texture* retoDireitaEsquerda_texture  = NULL;
SDL_Texture* retoEsquerdaDireita_texture  = NULL;
SDL_Texture* retoVertical_texture  = NULL;

SDL_Texture* pitu_texture  = NULL;
SDL_Texture* manga_texture  = NULL;
SDL_Texture* caju_texture  = NULL;
SDL_Texture* limao_texture  = NULL;

SDL_Texture* background_texture = NULL;
SDL_Texture* menu_texture = NULL;
SDL_Texture* mangabyte_texture = NULL;


// Definido na main, quando 0 ou FALSE o jogo para após executar a renderização
extern int game_is_running;

// Estados do jogo
typedef enum {
  GAME_STATE_MENU,
  GAME_STATE_PLAYING,
  GAME_STATE_SPLASH,
} GameState;

static GameState game_state = GAME_STATE_SPLASH; // Inicia o jogo como menu
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

typedef struct fruitTile
{
  mapTileType type;
  char sprite;
} fruitTile;


// Definindo o union que indica o que
// uma certa celula do mapa tem
typedef union mapTile
{
  mapTileType type;
  snakeTile snake;
  fruitTile fruit;

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
  SDL_Surface* surface = IMG_Load("assets/CabecaBaixo.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CabecaBaixo.png: %s\n", IMG_GetError());
      return;
  }
  CabecaBaixo_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CabecaCima.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CabecaCima.png: %s\n", IMG_GetError());
      return;
  }
  CabecaCima_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CabecaDireita.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CabecaDireita.png: %s\n", IMG_GetError());
      return;
  }
  CabecaDireita_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CabecaEsquerda.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CabecaEsquerda.png: %s\n", IMG_GetError());
      return;
  }
  CabecaEsquerda_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega Cauda
  surface = IMG_Load("assets/CaudaBaixo.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CaudaBaixo.png: %s\n", IMG_GetError());
      return;
  }
  CaudaBaixo_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CaudaCima.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CaudaCima.png: %s\n", IMG_GetError());
      return;
  }
  CaudaCima_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CaudaDireita.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CaudaDireita.png: %s\n", IMG_GetError());
      return;
  }
  CaudaDireita_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CaudaEsquerda.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CaudaEsquerda.png: %s\n", IMG_GetError());
      return;
  }
  CaudaEsquerda_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Curvas
  surface = IMG_Load("assets/CurvaBaixoDireita.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CurvaBaixoDireita.png: %s\n", IMG_GetError());
      return;
  }
  CurvaBaixoDireita_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CurvaBaixoEsquerda.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CurvaBaixoEsquerda.png: %s\n", IMG_GetError());
      return;
  }
  CurvaBaixoEsquerda_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CurvaCimaDireita.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CurvaCimaDireita.png: %s\n", IMG_GetError());
      return;
  }
  CurvaCimaDireita_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/CurvaCimaEsquerda.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar CurvaCimaEsquerda.png: %s\n", IMG_GetError());
      return;
  }
  CurvaCimaEsquerda_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  //Reto
  surface = IMG_Load("assets/RetoDireitaEsquerda.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar RetoDireitaEsquerda.png: %s\n", IMG_GetError());
      return;
  }
  retoDireitaEsquerda_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/RetoEsquerdaDireita.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar RetoEsquerdaDireita.png: %s\n", IMG_GetError());
      return;
  }
  retoEsquerdaDireita_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/RetoVertical.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar RetoVertical.png: %s\n", IMG_GetError());
      return;
  }
  retoVertical_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Frutas
  surface = IMG_Load("assets/limao.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar limao.png: %s\n", IMG_GetError());
      return;
  }
  limao_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/manga.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar manga.png: %s\n", IMG_GetError());
      return;
  }
  manga_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/pitu.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar pitu.png: %s\n", IMG_GetError());
      return;
  }
  pitu_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("assets/caju.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar caju.png: %s\n", IMG_GetError());
      return;
  }
  caju_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura do Background
  surface = IMG_Load("assets/background.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar background.png: %s\n", IMG_GetError());
      return;
  }
  background_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura do Menu
  surface = IMG_Load("assets/menu.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar menu.png: %s\n", IMG_GetError());
      return;
  }
  menu_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  // Carrega a textura da Splash Screen
  surface = IMG_Load("assets/mangabyte.png");
  if (!surface) {
      fprintf(stderr, "Erro ao carregar mangabyte.png: %s\n", IMG_GetError());
      return;
  }
  mangabyte_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
}
void cleanup_textures() {

  if(CabecaBaixo_texture) SDL_DestroyTexture(CabecaBaixo_texture);
  if(CabecaCima_texture) SDL_DestroyTexture(CabecaCima_texture);
  if(CabecaDireita_texture) SDL_DestroyTexture(CabecaDireita_texture);
  if(CabecaEsquerda_texture) SDL_DestroyTexture(CabecaEsquerda_texture);
  if(CaudaBaixo_texture) SDL_DestroyTexture(CaudaBaixo_texture);
  if(CaudaCima_texture) SDL_DestroyTexture(CaudaCima_texture);
  if(CaudaDireita_texture) SDL_DestroyTexture(CaudaDireita_texture);
  if(CaudaEsquerda_texture) SDL_DestroyTexture(CaudaEsquerda_texture);
  if(CurvaBaixoDireita_texture) SDL_DestroyTexture(CurvaBaixoDireita_texture);
  if(CurvaBaixoEsquerda_texture) SDL_DestroyTexture(CurvaBaixoEsquerda_texture);
  if(CurvaCimaDireita_texture) SDL_DestroyTexture(CurvaCimaDireita_texture);
  if(CurvaCimaEsquerda_texture) SDL_DestroyTexture(CurvaCimaEsquerda_texture);
  if(retoDireitaEsquerda_texture) SDL_DestroyTexture(retoDireitaEsquerda_texture);
  if(retoEsquerdaDireita_texture) SDL_DestroyTexture(retoEsquerdaDireita_texture);
  if(retoVertical_texture) SDL_DestroyTexture(retoVertical_texture);
  if(pitu_texture) SDL_DestroyTexture(pitu_texture);
  if(manga_texture) SDL_DestroyTexture(manga_texture);
  if(caju_texture) SDL_DestroyTexture(caju_texture);
  if(limao_texture) SDL_DestroyTexture(limao_texture);
  if(background_texture) SDL_DestroyTexture(background_texture);
  if(menu_texture) SDL_DestroyTexture(menu_texture);
  if(mangabyte_texture) SDL_DestroyTexture(mangabyte_texture);

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
  for (int i = 0; i < 5; i++) { // i < x = quantas frutas existirão no mapa
    int fruitX, fruitY;
    do {
        fruitX = rand() % MATRIX_WIDTH;
        fruitY = rand() % MATRIX_HEIGHT;
    } while (mapMatrix[fruitX][fruitY].type != EMPTY_TILE);

    mapMatrix[fruitX][fruitY].fruit = (fruitTile){FRUIT_TILE, rand()%7};
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

          // Lê input para trocar para o jogo se estiver na tela menu
          if (game_state == GAME_STATE_MENU && event.key.keysym.sym == SDLK_SPACE) {
            game_state = GAME_STATE_PLAYING;
            setup(); // Reinicia o jogo
          }
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

  if (game_state != GAME_STATE_PLAYING && game_state != GAME_STATE_SPLASH) return; // Não atualiza se a tela do jogo não estiver rodando
  if (game_state==GAME_STATE_SPLASH){ // Timer simples da Splash Screen para mudar para o menu
    if (slashcount >= 10000){
      game_state = GAME_STATE_MENU;
      return;
    } slashcount++;
  }
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

      mapMatrix[fruitX][fruitY].fruit = (fruitTile){FRUIT_TILE, rand()%7};
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
  if (game_state==GAME_STATE_SPLASH){
    SDL_Rect splash_rect = {
      0,
      0,
      WINDOW_HEIGHT,
      WINDOW_HEIGHT
    };
    SDL_RenderCopy(renderer, mangabyte_texture, NULL, &splash_rect);
  }
  else if (game_state==GAME_STATE_MENU){
  SDL_Rect menu_rect = {
    0,
    0,
    WINDOW_HEIGHT,
    WINDOW_HEIGHT
  };
  SDL_RenderCopy(renderer, menu_texture, NULL, &menu_rect);}
  else if (game_state==GAME_STATE_PLAYING){
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
      SDL_Texture* to_render = NULL;
      // Renderiza o segmento apropriado
      switch(seg_type) {
        case SEGMENT_HEAD:
        {
          switch(mapMatrix[cell_posX][cell_posY].snake.forwardDirection)
          {
            case UP: to_render = CabecaBaixo_texture; break;
            case RIGHT: to_render = CabecaEsquerda_texture; break;
            case DOWN: to_render = CabecaCima_texture; break;
            case LEFT: to_render = CabecaDireita_texture; break;
          }
          break;
        }
        case SEGMENT_TAIL: {
            // Encontre a direção da cauda (oposto da direção do segmento anterior)
            direction tail_dir = mapMatrix[cell_posX][cell_posY].snake.forwardDirection;

            switch(tail_dir)
            {
              case UP: to_render = CaudaCima_texture; break;
              case RIGHT: to_render = CaudaDireita_texture; break;
              case DOWN: to_render = CaudaBaixo_texture; break;
              case LEFT: to_render = CaudaEsquerda_texture; break;
            }
            break;
        }
        case SEGMENT_CURVE:
          {
            switch(mapMatrix[cell_posX][cell_posY].snake.forwardDirection){
              case UP: // Direita - Cima
                to_render = prev_direction == LEFT ? CurvaCimaDireita_texture : CurvaCimaEsquerda_texture;
                break;
              case DOWN: // Esquerda - Cima
                to_render = prev_direction == LEFT ? CurvaBaixoDireita_texture : CurvaBaixoEsquerda_texture;
                break;
              case LEFT: // Direita - Baixo
                to_render = prev_direction == DOWN ? CurvaCimaEsquerda_texture : CurvaBaixoEsquerda_texture;
                break;
              case RIGHT: // Esquerda - Baixo
                to_render = prev_direction == DOWN ? CurvaCimaDireita_texture : CurvaBaixoDireita_texture;
                break;
            }
      break;
      }
        case SEGMENT_STRAIGHT: {
            switch(mapMatrix[cell_posX][cell_posY].snake.forwardDirection) {
                case UP: to_render = retoVertical_texture; break;
                case RIGHT: to_render = retoEsquerdaDireita_texture; break;
                case DOWN: to_render = retoVertical_texture; break;
                case LEFT: to_render = retoDireitaEsquerda_texture; break;
            }
            break;
        }

    }
    SDL_RenderCopyEx(renderer, to_render, NULL, &dest_rect, 0, NULL, SDL_FLIP_NONE);


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
          if(mapMatrix[x][y].type == FRUIT_TILE)
          {
            SDL_Texture* to_render = NULL;
            switch (mapMatrix[x][y].fruit.sprite)
            {
              case 0:
              case 1:
                to_render = manga_texture;
                break;
              case 2:
              case 3:
                to_render = limao_texture;
                break;
              case 4:
              case 5:
                to_render = caju_texture;
                break;
              case 6:
                to_render = pitu_texture;
                break;

            }
            SDL_Rect dest_rect = rectFromCellPos(x, y);
            SDL_RenderCopy(renderer, to_render, NULL, &dest_rect);
          }
      }
  }
}
  // Restaura o viewport padrão para a janela toda
  SDL_RenderSetViewport(renderer, NULL);
  SDL_RenderPresent(renderer);
}


