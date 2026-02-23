#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define PLAYER_SCALE 4
#define PLAYER_IMAGE_NUMBER 2
#define MAX_ENEMY_NUMBER 5
#define PLAYER_ACCEL 3
#define PLAYER_

// Core Variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
Vector2 w_camera_pos;

// Core Struct
typedef struct Sprite {
  Vector2 w_pos, velocity;
  Texture texture;
  float acceleration, max_speed, slow_down_factor;
} Sprite;
void load_sprite_texture(Sprite *sprite, Image image) {
  sprite->texture = LoadTextureFromImage(image);
}
void unload_sprite_texture(Sprite *sprite) { UnloadTexture(sprite->texture); }
Vector2 get_sprite_s_pos(Sprite *sprite) {
  Vector2 result;
  result.x = sprite->w_pos.x - w_camera_pos.x + WINDOW_WIDTH / 2.0;
  result.y = sprite->w_pos.y - w_camera_pos.y + WINDOW_HEIGHT / 2.0;
  return result;
}
void draw_sprite(Sprite sprite) {
  Vector2 s_pos = get_sprite_s_pos(&sprite);
  DrawTexture(sprite.texture, s_pos.x - sprite.texture.width / 2.0,
              s_pos.y - sprite.texture.height / 2.0, WHITE);
}
void init_sprite(Sprite *sprite, Vector2 w_pos, Image image, float accel,
                 float max_speed, float sdf) {
  *sprite = (Sprite){w_pos, {0, 0},    LoadTextureFromImage(image),
                     accel, max_speed, sdf};
}
void handle_sprite_movement(Sprite *sprite) {
  float speed = sqrt(pow(sprite->velocity.x, 2) + pow(sprite->velocity.y, 2));
  if (speed > sprite->max_speed) {
    sprite->velocity.x *= sprite->max_speed / speed;
    sprite->velocity.y *= sprite->max_speed / speed;
  }
  sprite->w_pos.x += sprite->velocity.x;
  sprite->w_pos.y += sprite->velocity.y;
}
Sprite player;
Sprite enemies[MAX_ENEMY_NUMBER];
Sprite *spawner;

Vector2 get_random_w_pos(Vector2 lt, Vector2 rb) {
  Vector2 result;
  result.x = lt.x + rand() % (int)(rb.x - lt.x + 1);
  result.y = rb.y + rand() % (int)(lt.y - rb.y + 1);
  return result;
}

void spawn_zombie(Sprite *target, Image image) {
  init_sprite(target, get_random_w_pos(), image, );
}

void handle_player_movement() {
  bool pressed = false;
  if (IsKeyDown(KEY_W)) {
    player.velocity.y -= player.acceleration;
    pressed = true;
  }
  if (IsKeyDown(KEY_S)) {
    player.velocity.y += player.acceleration;
    pressed = true;
  }
  if (IsKeyDown(KEY_A)) {
    player.velocity.x -= player.acceleration;
    pressed = true;
  }
  if (IsKeyDown(KEY_D)) {
    player.velocity.x += player.acceleration;
    pressed = true;
  }
  if (!pressed) {
    player.velocity.x *= player.slow_down_factor;
    player.velocity.y *= player.slow_down_factor;
  }
  handle_sprite_movement(&player);
}

int main() {
  // Initialize
  srand((unsigned int)time(NULL));
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TVSG");
  SetTraceLogLevel(LOG_WARNING);
  SetTargetFPS(60);
  Image image_player[PLAYER_IMAGE_NUMBER];
  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    image_player[i] = LoadImage(TextFormat("./assets/player%d.png", i));
    ImageResizeNN(&image_player[i], image_player[i].width * PLAYER_SCALE,
                  image_player[i].height * PLAYER_SCALE);
  }
  init_sprite(&player, (Vector2){0, 0}, image_player[0], 3, 10, 0.9);
  unload_sprite_texture(&player);
  w_camera_pos = (Vector2){0, 0};

  // Game loop
  while (!WindowShouldClose()) {

    handle_player_movement();

    player.texture =
        LoadTextureFromImage(image_player[(int)(GetTime() * 5) % 2 == 0]);

    BeginDrawing();
    ClearBackground(BLACK);
    draw_sprite(player);
    EndDrawing();

    unload_sprite_texture(&player);
  }

  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    UnloadImage(image_player[i]);
  }
  CloseWindow();

  return 0;
}
