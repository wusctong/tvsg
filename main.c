#include "raylib.h"

int main() {
  InitWindow(800, 600, "TVSG");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("TVSG", 50, 50, 20, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
