/*
 * TVSG
 * a Top View Shooter Game written in C.
 *
 * author: wusctong
 *
 */

#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants
#define _MAX_TAG_STRING_LENGTH

#define SPRITE_SCALE 3
#define PLAYER_IMAGE_NUMBER 30
#define MAX_ENEMY_NUMBER 10
#define PLAYER_ACCEL 2
#define PLAYER_MAX_SPEED 7
#define PLAYER_SDF 0.9
#define ZOMBIE_ACCEL 0.75
#define ZOMBIE_MAX_SPEED 1.5
#define ZOMBIE_SDF 0.6

// Core Variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
unsigned long corpse_count = 0;
Vector2 w_camera_pos;

// Core Functions
float absf(float n) { return (n < 0) ? -n : n; }
Vector2 get_random_w_pos(Vector2 lt, Vector2 rb) {
  Vector2 result;
  result.x = lt.x + rand() % (int)(rb.x - lt.x + 1);
  result.y = lt.y + rand() % (int)(rb.y - lt.y + 1);
  return result;
}
Vector2 get_s_pos(Vector2 w_pos) {
  Vector2 result;
  result.x = w_pos.x - w_camera_pos.x + WINDOW_WIDTH / 2.0;
  result.y = w_pos.y - w_camera_pos.y + WINDOW_HEIGHT / 2.0;
  return result;
}
Rectangle get_hitbox(Vector2 w_pos, float hb_width, float hb_height) {
  return (Rectangle){w_pos.x - hb_width / 2, w_pos.y - hb_height / 2, hb_width,
                     hb_height};
}
float vel_to_speed(Vector2 velocity) {
  return sqrt(pow(velocity.x, 2) + pow(velocity.y, 2));
}

// Core Struct
typedef struct Sprite {
  Vector2 w_pos, velocity;
  Texture texture;
  float acceleration, max_speed, slow_down_factor;
  float hb_width, hb_height;
  long health;
  char *tag;
} Sprite;
// .initialize
void init_sprite(Sprite *sprite, Vector2 w_pos, Image image, float accel,
                 float max_speed, float sdf, float hb_width, float hb_height,
                 long health, const char *tag) {
  sprite->w_pos = w_pos;
  sprite->velocity = (Vector2){0, 0};
  sprite->texture = LoadTextureFromImage(image);
  sprite->acceleration = accel;
  sprite->max_speed = max_speed;
  sprite->slow_down_factor = sdf;
  sprite->hb_width = hb_width;
  sprite->hb_height = hb_height;
  sprite->health = health;
  sprite->tag = malloc(strlen(tag) + 1);
  strcpy(sprite->tag, tag);
}
// .texture
void load_sprite_texture(Sprite *sprite, Image image) {
  sprite->texture = LoadTextureFromImage(image);
}
void unload_sprite_texture(Sprite *sprite) { UnloadTexture(sprite->texture); }
// .func
bool check_sprite_collision(Sprite a, Sprite b) {
  Rectangle rect_a = get_hitbox(a.w_pos, a.hb_width, a.hb_height);
  Rectangle rect_b = get_hitbox(b.w_pos, b.hb_width, b.hb_height);
  return CheckCollisionRecs(rect_a, rect_b);
}
void retag_sprite(Sprite *sprite, const char *tag) {
  if (!strcmp(sprite->tag, tag)) {
    strcpy(sprite->tag, tag);
  }
}
/*void destroy_sprite(Sprite *sprite) {
  unload_sprite_texture(sprite);
  free(sprite);
}*/
// .draw
void draw_sprite_hitbox(Sprite sprite, Color color) {
  Rectangle hitbox =
      get_hitbox(sprite.w_pos, sprite.hb_width, sprite.hb_height);
  DrawRectangleLines(hitbox.x - w_camera_pos.x + WINDOW_WIDTH / 2.0,
                     hitbox.y - w_camera_pos.y + WINDOW_HEIGHT / 2.0,
                     hitbox.width, hitbox.height, color);
}
void draw_sprite(Sprite sprite) {
  Vector2 s_pos = get_s_pos(sprite.w_pos);
  DrawTexture(sprite.texture, s_pos.x - sprite.texture.width / 2.0,
              s_pos.y - sprite.texture.height / 2.0, WHITE);
}
// .movement
void handle_sprite_movement(Sprite *sprite) {
  float speed = vel_to_speed(sprite->velocity);
  if (speed > sprite->max_speed) {
    sprite->velocity.x *= sprite->max_speed / speed;
    sprite->velocity.y *= sprite->max_speed / speed;
  }
  sprite->w_pos.x += sprite->velocity.x;
  sprite->w_pos.y += sprite->velocity.y;
}

typedef void (*func)(Sprite *sprite);
typedef struct Trigger {
  Vector2 w_pos;
  float hb_width, hb_height;
  func behavior;
} Trigger;
// .initialize
void init_trigger(Trigger *trigger, Vector2 w_pos, float hb_width,
                  float hb_height, func behavior) {
  *trigger = (Trigger){w_pos, hb_width, hb_height, behavior};
}
// .func
bool check_trigger_collision(Trigger a, Sprite b) {
  Rectangle rect_a = get_hitbox(a.w_pos, a.hb_width, a.hb_height);
  Rectangle rect_b = get_hitbox(b.w_pos, b.hb_width, b.hb_height);
  return CheckCollisionRecs(rect_a, rect_b);
}
void handle_trigger_behavior(Trigger a, Sprite *b, const char *tag) {
  if (check_trigger_collision(a, *b) && !strcmp(b->tag, tag)) {
    a.behavior(b);
  }
}
// .draw
void draw_trigger_hitbox(Trigger trigger, Color color) {
  Rectangle hitbox =
      get_hitbox(trigger.w_pos, trigger.hb_width, trigger.hb_height);
  DrawRectangleLines(hitbox.x - w_camera_pos.x + WINDOW_WIDTH / 2.0,
                     hitbox.y - w_camera_pos.y + WINDOW_HEIGHT / 2.0,
                     hitbox.width, hitbox.height, color);
}

typedef struct Particle {
} Particle;

// Special Sprites
Sprite player, map;
Sprite enemies[MAX_ENEMY_NUMBER];
Sprite *corpses = NULL;

// Zombie
void spawn_zombie(Sprite *target, Image image) {
  init_sprite(target,
              get_random_w_pos((Vector2){-512, -256}, (Vector2){512, 256}),
              image, ZOMBIE_ACCEL, ZOMBIE_MAX_SPEED, ZOMBIE_SDF,
              12 * SPRITE_SCALE, 12 * SPRITE_SCALE, 40, "enemy");
}
void handle_zombie_spawn(Image image) {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    if (enemies[i].health <= 0) {
      corpse_count++;
      Sprite *temp = realloc(corpses, corpse_count * sizeof(Sprite));
      if (temp == NULL)
        free(corpses);
      corpses = temp;
      corpses[corpse_count - 1] = enemies[i];
      spawn_zombie(&enemies[i], image);
    }
  }
}
void unload_zombie_texture() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++)
    unload_sprite_texture(&enemies[i]);
}
/*void destroy_zombie() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    destroy_sprite(&enemies[i]);
  }
}*/
void draw_zombie() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++)
    if (enemies[i].health > 0)
      draw_sprite(enemies[i]);
}
void handle_zombie_movement() {
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    if (enemies[i].health > 0) {
      Vector2 delta_velocity = {0, 0};
      float delta_x = player.w_pos.x - enemies[i].w_pos.x;
      float delta_y = player.w_pos.y - enemies[i].w_pos.y;
      if (absf(delta_x) < enemies[i].acceleration * 4) {
        enemies[i].velocity.x *= enemies[i].slow_down_factor;
      } else {
        if (delta_x > 0) {
          delta_velocity.x += enemies[i].acceleration;
        } else if (delta_x < 0) {
          delta_velocity.x -= enemies[i].acceleration;
        }
      }
      if (absf(delta_y) < enemies[i].acceleration * 4) {
        enemies[i].velocity.y *= enemies[i].slow_down_factor;
      } else {
        if (delta_y > 0) {
          delta_velocity.y += enemies[i].acceleration;
        } else if (delta_y < 0) {
          delta_velocity.y -= enemies[i].acceleration;
        }
      }
      for (int j = 0; j < MAX_ENEMY_NUMBER; j++) {
        if (j == i)
          continue;
        if (check_sprite_collision(enemies[i], enemies[j])) {
          delta_velocity.x -= (enemies[j].w_pos.x - enemies[i].w_pos.x) / 2;
          delta_velocity.y -= (enemies[j].w_pos.y - enemies[i].w_pos.y) / 2;
        }
      }
      enemies[i].velocity.x += delta_velocity.x;
      enemies[i].velocity.y += delta_velocity.y;
      handle_sprite_movement(&enemies[i]);
    }
  }
}

// Player
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

// Camera
void camera_follow(Sprite sprite) {
  w_camera_pos.x = sprite.w_pos.x;
  w_camera_pos.y = sprite.w_pos.y;
}

// Trigger Special Funcs
void player_weapon(Sprite *sprite) { sprite->health -= 1; }

int main() {
  // Initialize
  bool player_firing = false;

  srand((unsigned int)time(NULL));
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TVSG");
  SetTraceLogLevel(LOG_WARNING);
  SetTargetFPS(60);

  Image image_zombie = LoadImage("./assets/zombie0.png");
  ImageResizeNN(&image_zombie, image_zombie.width * SPRITE_SCALE,
                image_zombie.height * SPRITE_SCALE);
  Image image_map = LoadImage("./assets/map1.png");
  ImageResizeNN(&image_map, image_map.width * SPRITE_SCALE * 1.5,
                image_map.height * SPRITE_SCALE * 1.5);
  Image image_player[PLAYER_IMAGE_NUMBER];
  for (int i = 0; i < PLAYER_IMAGE_NUMBER; i++) {
    image_player[i] = LoadImage(TextFormat("./assets/player%d.png", i));
    ImageResizeNN(&image_player[i], image_player[i].width * SPRITE_SCALE,
                  image_player[i].height * SPRITE_SCALE);
  }
  for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
    spawn_zombie(&enemies[i], image_zombie);
  }
  init_sprite(&player, (Vector2){0, 0}, image_player[0], PLAYER_ACCEL,
              PLAYER_MAX_SPEED, PLAYER_SDF, 12 * SPRITE_SCALE,
              18 * SPRITE_SCALE, 100, "player");
  init_sprite(&map, (Vector2){0, 0}, image_map, 0, 0, 0, 0, 0, 100, "map");
  unload_sprite_texture(&player);
  w_camera_pos = (Vector2){0, 0};

  Trigger weapon;
  init_trigger(&weapon, player.w_pos, 16 * SPRITE_SCALE, 16 * SPRITE_SCALE,
               player_weapon);

  // Game loop
  while (!WindowShouldClose()) {
    camera_follow(player);

    handle_zombie_spawn(image_zombie);

    handle_player_movement();
    handle_zombie_movement();

    if (IsKeyDown(KEY_SPACE)) {
      player_firing = true;
    } else {
      player_firing = false;
    }

    weapon.w_pos = player.w_pos;

    if (player_firing) {
      for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
        handle_trigger_behavior(weapon, &enemies[i], "enemy");
      }
    }

    player.texture =
        LoadTextureFromImage(image_player[(int)(GetTime() * 30) % 30]);

    BeginDrawing();
    ClearBackground(BLACK);
    draw_sprite(map);
    if (corpses != NULL) {
      for (unsigned long i = 0; i < corpse_count; i++) {
        draw_sprite(*(corpses + i));
      }
    }
    draw_zombie();
    draw_sprite(player);
    DrawText(TextFormat("X: %f\nY: %f\nHEALTH: %d", player.w_pos.x,
                        player.w_pos.y, player.health),
             10, 10, 20, WHITE);
    /*
    for (int i = 0; i < MAX_ENEMY_NUMBER; i++) {
      draw_sprite_hitbox(enemies[i], BLUE);
      DrawText(TextFormat("E%d %s: %d", i, enemies[i].tag, enemies[i].health),
               10, 150 + 20 * i, 20, WHITE);
    }
    draw_sprite_hitbox(player, RED);
    draw_trigger_hitbox(weapon, GREEN);
    */
    EndDrawing();

    unload_sprite_texture(&player);
  }

  // destroy_sprite(&player);
  // destroy_sprite(&map);
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
