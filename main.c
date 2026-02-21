#include "raylib.h"

// Constants
const int PLAYER_SCALE = 4;
const int PLAYER_IMAGE_NUMBER = 2;

// Core Variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
// w_ -> world
// s_ -> screen
Vector2 w_player_pos, s_player_pos, w_camera_pos;

void set_s_player_pos() {
  s_player_pos.x = w_player_pos.x - w_camera_pos.x + WINDOW_WIDTH / 2.0;
  s_player_pos.y = w_player_pos.y - w_camera_pos.y + WINDOW_HEIGHT / 2.0;
}

int main() {
  // Initialize Core Variables
  w_player_pos = (Vector2){0, 0};
  w_camera_pos = (Vector2){0, 0};
  set_s_player_pos();

  // Initialize window
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TVSG");
  SetTraceLogLevel(LOG_WARNING);
  SetTargetFPS(60);

  // Load player images
  Image image_player[PLAYER_IMAGE_NUMBER];
  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    image_player[i] = LoadImage(TextFormat("./assets/player%d.png", i));
    ImageResizeNN(&image_player[i], image_player[i].width * PLAYER_SCALE,
                  image_player[i].height * PLAYER_SCALE);
  }

  // Game loop
  while (!WindowShouldClose()) {
    Texture texture_player;
    texture_player =
        LoadTextureFromImage(image_player[(int)(GetTime() * 5) % 2 == 0]);

    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexture(texture_player, s_player_pos.x - texture_player.width / 2.0,
                s_player_pos.y - texture_player.height / 2.0, WHITE);
    EndDrawing();

    UnloadTexture(texture_player);
  }

  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    UnloadImage(image_player[i]);
  }
  CloseWindow();

  return 0;
}
