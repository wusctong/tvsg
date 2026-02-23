#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define SPRITE_SCALE 2
#define PLAYER_IMAGE_NUMBER 30
#define MAX_ENEMY_NUMBER 5
#define PLAYER_ACCEL 2
#define PLAYER_MAX_SPEED 7
#define PLAYER_SDF 0.8
#define ZOMBIE_ACCEL 1.5
#define ZOMBIE_MAX_SPEED 4.5
#define ZOMBIE_SDF 0.6

// Core Variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
Vector2 w_camera_pos;

// Core Functions
float absf(float n) { return (n < 0) ? -n : n; }

// Core Struct
typedef struct Sprite {
  Vector2 w_pos, velocity;
  Texture texture;
  float acceleration, max_speed, slow_down_factor;
  bool is_alive;
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
float get_sprite_speed(Sprite *sprite) {
  return sqrt(pow(sprite->velocity.x, 2) + pow(sprite->velocity.y, 2));
}
void draw_sprite(Sprite sprite) {
  Vector2 s_pos = get_sprite_s_pos(&sprite);
  DrawTexture(sprite.texture, s_pos.x - sprite.texture.width / 2.0,
              s_pos.y - sprite.texture.height / 2.0, WHITE);
}
void init_sprite(Sprite *sprite, Vector2 w_pos, Image image, float accel,
                 float max_speed, float sdf, bool is_alive) {
  *sprite =
      (Sprite){w_pos, {0, 0},  LoadTextureFromImage(image), accel, max_speed,
               sdf,   is_alive};
}
void handle_sprite_movement(Sprite *sprite) {
  float speed = get_sprite_speed(sprite);
  if (speed > sprite->max_speed) {
    sprite->velocity.x *= sprite->max_speed / speed;
    sprite->velocity.y *= sprite->max_speed / speed;
  }
  sprite->w_pos.x += sprite->velocity.x;
  sprite->w_pos.y += sprite->velocity.y;
}
Sprite player, map;
Sprite enemies[MAX_ENEMY_NUMBER];
Sprite *spawner;

Vector2 get_random_w_pos(Vector2 lt, Vector2 rb) {
  Vector2 result;
  result.x = lt.x + rand() % (int)(rb.x - lt.x + 1);
  result.y = lt.y + rand() % (int)(rb.y - lt.y + 1);
  return result;
}

void spawn_zombie(Sprite *target, Image image) {
  init_sprite(target,
              get_random_w_pos((Vector2){-400, -300}, (Vector2){400, 300}),
              image, ZOMBIE_ACCEL, ZOMBIE_MAX_SPEED, ZOMBIE_SDF, true);
}
void handle_zombie_spawn(Image image) {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    if (!enemies[i].is_alive) {
      spawn_zombie(&enemies[i], image);
    }
  }
}
void unload_zombie_texture() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    unload_sprite_texture(&enemies[i]);
  }
}
void draw_zombie() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++)
    if (enemies[i].is_alive)
      draw_sprite(enemies[i]);
}
void handle_zombie_movement() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    if (enemies[i].is_alive) {
      float delta_x = player.w_pos.x - enemies[i].w_pos.x;
      float delta_y = player.w_pos.y - enemies[i].w_pos.y;

      if (absf(delta_x) < enemies[i].acceleration * 4) {
        enemies[i].velocity.x *= enemies[i].slow_down_factor;
      } else {
        if (delta_x > 0) {
          enemies[i].velocity.x += enemies[i].acceleration;
        } else if (delta_x < 0) {
          enemies[i].velocity.x -= enemies[i].acceleration;
        }
      }
      if (absf(delta_y) < enemies[i].acceleration * 4) {
        enemies[i].velocity.y *= enemies[i].slow_down_factor;
      } else {
        if (delta_y > 0) {
          enemies[i].velocity.y += enemies[i].acceleration;
        } else if (delta_y < 0) {
          enemies[i].velocity.y -= enemies[i].acceleration;
        }
      }
      handle_sprite_movement(&enemies[i]);
    }
  }
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

void camera_follow(Sprite sprite) {
  w_camera_pos.x = sprite.w_pos.x;
  w_camera_pos.y = sprite.w_pos.y;
}

int main() {
  // Initialize
  srand((unsigned int)time(NULL));
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TVSG");
  SetTraceLogLevel(LOG_WARNING);
  SetTargetFPS(60);

  Image image_zombie = LoadImage("./assets/zombie0.png");
  ImageResizeNN(&image_zombie, image_zombie.width * SPRITE_SCALE,
                image_zombie.height * SPRITE_SCALE);
  Image image_map = LoadImage("./assets/map0.png");
  ImageResizeNN(&image_map, image_map.width * SPRITE_SCALE * 1.5,
                image_map.height * SPRITE_SCALE * 1.5);
  Image image_player[PLAYER_IMAGE_NUMBER];
  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    image_player[i] = LoadImage(TextFormat("./assets/player%d.png", i));
    ImageResizeNN(&image_player[i], image_player[i].width * SPRITE_SCALE,
                  image_player[i].height * SPRITE_SCALE);
  }
  init_sprite(&player, (Vector2){0, 0}, image_player[0], PLAYER_ACCEL,
              PLAYER_MAX_SPEED, PLAYER_SDF, true);
  init_sprite(&map, (Vector2){-512, -256}, image_map, 0, 0, 0, false);
  unload_sprite_texture(&player);

  w_camera_pos = (Vector2){0, 0};

  // Game loop
  while (!WindowShouldClose()) {
    camera_follow(player);
    handle_zombie_spawn(image_zombie);
    handle_player_movement();
    handle_zombie_movement();

    player.texture =
        LoadTextureFromImage(image_player[(int)(GetTime() * 30) % 30]);

    BeginDrawing();
    ClearBackground(BLACK);
    draw_sprite(map);
    draw_zombie();
    draw_sprite(player);
    EndDrawing();

    unload_sprite_texture(&player);
  }

  unload_sprite_texture(&map);
  unload_zombie_texture();

  UnloadImage(image_map);
  UnloadImage(image_zombie);
  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    UnloadImage(image_player[i]);
  }
  CloseWindow();

  return 0;
}
