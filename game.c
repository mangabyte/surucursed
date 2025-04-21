#include <stdio.h>
#include "./game.h"
#include "./constants.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdlib.h>   // Adicionado geração de números aleatórios (stdlib.h)
#include <time.h>     // Adicionado  e controle de tempo (time.h) para gerenciamento de frutas.

// O tamanho do "Tabuleiro" em quantidade de quadrados que a cobra está
#define MATRIX_WIDTH 30
#define MATRIX_HEIGHT 15

// O tamanho de cada celula("Quadradinho") do "Tabuleiro" na tela
// (Isso foi feito para melhorar a legibilidade)
#define CELL_WIDTH WINDOW_WIDTH/MATRIX_WIDTH
#define CELL_HEIGHT WINDOW_HEIGHT/MATRIX_HEIGHT

// MACROS com os parametros de cores (RGB alpha)
#define RED 255,0,0,255
#define GREEN 0,255,0,255
#define MANGENTA 255,0,255,255
#define BLACK 0,0,0,255

// Definido na main, quando 0 ou FALSE o jogo para após executar a renderização
extern int game_is_running;

// Definindo o enum indica o que
// cada celula do mapa pode assumir
typedef enum mapTileType{
  EMPTY_TILE,
  SNAKE_TILE,
  FRUIT_TILE
} mapTileType;

// Definindo o enum que indica as direções
typedef enum direction {UP,DOWN,LEFT,RIGHT} direction;

// Definindo o struct que indica o que cada
// celula do tabuleiro que a cobra ocupa deve ter
typedef struct snakeTile{
  mapTileType type;
  direction forwardDirection;
} snakeTile;

// Definindo o union que indica o que
// uma certa celula do mapa tem
typedef union mapTile{
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
// Aqui vao algumas alteracoes:

Uint32 next_move_time; // Tempo para a próxima movimentação da cobra
Uint32 move_delay = 300; // Tempo entre movimentos (velocidade da cobra em ms)

// Variáveis para controle das frutas
int fruit_count = 0;         // Quantidade atual de frutas no mapa
int max_fruits = 3;          // Máximo de frutas simultâneas no mapa
Uint32 next_fruit_time;      // Tempo para a próxima geração de fruta
Uint32 fruit_spawn_delay = 5000; // Tempo entre frutas em ms

// Estrutura para pontuação
int score = 0;
// --Funções para renderização --
// Definição de funçoes que retornam a posição
// do centro da celula na tela em função da
// posição dele na matrix.
int MatrixToWindowX(int _matrixX) {return _matrixX*CELL_WIDTH + CELL_WIDTH/2;}
int MatrixToWindowY(int _matrixY) {return WINDOW_HEIGHT - (_matrixY * CELL_HEIGHT + CELL_HEIGHT/2);}

// Definição de uma função que dá as específicações
// de desenho de um retângulo
SDL_Rect rectFromCellPos(int cell_posX, int cell_posY){
  return (SDL_Rect){
    MatrixToWindowX(cell_posX) - CELL_WIDTH/2,
    MatrixToWindowY(cell_posY) - CELL_HEIGHT/2,
    CELL_WIDTH,
    CELL_HEIGHT
  };
}

// Função para gerar uma nova fruta em posição aleatória
void spawn_fruit(){
  if (fruit_count >= max_fruits) return;

  int attempts = 0;
  int max_attempts = 100; // Evita loop infinito
  while (attempts < max_attempts) {
    int x = rand() % MATRIX_WIDTH;
    int y = rand() % MATRIX_HEIGHT;
    // Verifica se a posição está vazia
    if (mapMatrix[x][y].type == EMPTY_TILE) {
      mapMatrix[x][y].type = FRUIT_TILE;
      fruit_count++;
      return;
    }
    attempts++;
  }
  // Se não conseguiu posicionar após várias tentativas
  fprintf(stderr, "Não foi possível posicionar a fruta após %d tentativas.\n", max_attempts);
}

// Função para mover a cobra uma célula na direção atual
void move_snake() {
  int new_headX = snake_headX;
  int new_headY = snake_headY;
  // Determinar a nova posição da cabeça baseada na direção
  switch (mapMatrix[snake_headX][snake_headY].snake.forwardDirection) {
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
  // Verificar se a nova posição está dentro dos limites
  if (new_headX < 0 || new_headX >= MATRIX_WIDTH || new_headY < 0 ||
    new_headY >= MATRIX_HEIGHT){
      fprintf(stderr, "Game over: A cobra bateu na borda em [%d][%d].\n", new_headX, new_headY);
      game_is_running = FALSE;
      return;
  }
  // Verificar colisão com a própria cobra
  if (mapMatrix[new_headX][new_headY].type == SNAKE_TILE) {
      fprintf(stderr, "Game over: A cobra colidiu consigo mesma em [%d][%d].\n", new_headX, new_headY);
      game_is_running = FALSE;
      return;
  }

  // Verificar se comeu uma fruta
  int ate_fruit = (mapMatrix[new_headX][new_headY].type == FRUIT_TILE);
  // Guardar a direção atual da cabeça
  direction current_direction = mapMatrix[snake_headX][snake_headY].snake.forwardDirection;
  // Mover a cabeça para a nova posição
  mapMatrix[new_headX][new_headY].snake = (snakeTile){SNAKE_TILE, current_direction};
  // Atualizar a posição da cabeça
  snake_headX = new_headX;
  snake_headY = new_headY;
  // Se não comeu fruta, move a cauda (remove o último segmento)
  if (!ate_fruit) {
    direction tail_direction = mapMatrix[snake_tailX][snake_tailY].snake.forwardDirection;
    mapMatrix[snake_tailX][snake_tailY].type = EMPTY_TILE;
    // Mover a cauda para a próxima posição
    switch (tail_direction) {
      case UP:
        snake_tailY++;
        break;
      case DOWN:
        snake_tailY--;
        break;
      case LEFT:
        snake_tailX--;
        break;
      case RIGHT:
        snake_tailX++;
        break;
    }
  } else {
      // Comeu fruta
      snake_size++;
      score += 10;
      fruit_count--;
      fprintf(stderr, "Pontuação: %d | Tamanho da cobra: %d\n", score, snake_size);
  }
}

// Função para mudar a direção da cobra
void change_direction(direction new_direction) {
  // Evitar que a cobra vá na direção oposta à atual
  direction current_direction = mapMatrix[snake_headX][snake_headY].snake.forwardDirection;
  if ((new_direction == UP && current_direction == DOWN) ||
    (new_direction == DOWN && current_direction == UP) ||
    (new_direction == LEFT && current_direction == RIGHT) ||
    (new_direction == RIGHT && current_direction == LEFT)) {
      return; // Ignora mudança para direção oposta
  }
  mapMatrix[snake_headX][snake_headY].snake.forwardDirection = new_direction;
}

// --Game Loop--
void setup(){
  // Iniciando toda a matriz como vazia
  for(int i = 0; i < MATRIX_WIDTH; i++){
    for(int j = 0; j < MATRIX_HEIGHT; j++){
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
}
void process_input(){
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type){
      case SDL_QUIT:
        game_is_running = FALSE;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
          game_is_running = FALSE;
        // Controles de direção
        else if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
          change_direction(UP);
        else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
          change_direction(DOWN);
        else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
          change_direction(LEFT);
        else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
          change_direction(RIGHT);
        break;
    }
  }
}
void update()
{
  Uint32 current_time = SDL_GetTicks();

  // Verifica se é hora de mover a cobra
  if (current_time >= next_move_time){
    move_snake();
    next_move_time = current_time + move_delay;
  }

  // Verifica se é hora de gerar uma nova fruta
  if (current_time >= next_fruit_time) {
    spawn_fruit();
    next_fruit_time = current_time + fruit_spawn_delay;
  }
}

void render(SDL_Renderer* renderer){
  SDL_SetRenderDrawColor(renderer, BLACK);
  SDL_RenderClear(renderer);

  SDL_Rect rect;

  int cell_posX;
  int cell_posY;

  cell_posX = snake_tailX;
  cell_posY = snake_tailY;

  for(int i = 0; i < snake_size; i++){
    if(mapMatrix[cell_posX][cell_posY].type != SNAKE_TILE){
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
    // Adiciona detalhe à fruta (um círculo ou ponto no centro)
    SDL_SetRenderDrawColor(renderer, GREEN);
    SDL_Rect stem = {
      MatrixToWindowX(cell_posX) - 2,
      MatrixToWindowY(cell_posY) + CELL_HEIGHT/4,
      4, 4
    };
    SDL_RenderFillRect(renderer, &stem);
    // Coloca a cor a ser desenhada (Vermelha)
    SDL_SetRenderDrawColor(renderer, RED);

    // Salva as direções da cobra
    // Move o ponteiro para a proxima casa
    char dirX = 0;
    char dirY = 0;

    // Atualiza a direção a ser renderizada da cobra e
    // registra o movimento efetuado
    switch (mapMatrix[cell_posX][cell_posY].snake.forwardDirection){
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
  for(int cell_posX = 0; cell_posX < MATRIX_WIDTH; cell_posX++){
    for(int cell_posY = 0; cell_posY < MATRIX_HEIGHT; cell_posY++){
      // Define um quadrado na posição da célula
      rect = rectFromCellPos(cell_posX, cell_posY);
      switch (mapMatrix[cell_posX][cell_posY].type){
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
