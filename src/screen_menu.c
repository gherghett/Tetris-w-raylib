#include "screen_menu.h"
#include "raylib.h"
#include "screen_manager.h"

void ScreenMenu_Init(void)
{
    // Initialize menu screen
}

void ScreenMenu_Update(void)
{
    // Update menu screen
    if (IsKeyPressed(KEY_ENTER))
    {
        ChangeToScreen(SCREEN_GAMEPLAY);
    }
}

void ScreenMenu_Draw(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    DrawText("TETRIS", 350, 200, 40, DARKGRAY);
    DrawText("Press ENTER to start", 320, 280, 20, GRAY);
    
    EndDrawing();
}

void ScreenMenu_Unload(void)
{
    // Cleanup menu screen
}
