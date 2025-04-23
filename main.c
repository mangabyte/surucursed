#include "./game.h"
#include "./windowManager.h"
#include "./constants.h"

int game_is_running = FALSE;

int main(int argc, char** argv)
{
  game_is_running = initialize_window();

  load_textures(renderer);

  setup();

  while (game_is_running)
  {
      process_input();
      update();
      render(renderer);
  }
  cleanup_textures();
  destroy_window();

  return 0;
}
