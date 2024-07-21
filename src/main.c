#include <raylib.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Olive");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float scale = MIN((float)GetScreenWidth()/screenWidth, (float)GetScreenHeight()/screenHeight);

    BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
    EndTextureMode();

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                   (Rectangle){ (GetScreenWidth() - ((float)screenWidth*scale))*0.5f, (GetScreenHeight() - ((float)screenHeight*scale))*0.5f,
                   (float)screenWidth*scale, (float)screenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
