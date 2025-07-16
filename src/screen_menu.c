#include "screen_menu.h"
#include "raylib.h"
#include "screen_manager.h"
#include "highscore.h"

int highScore;
char scoreText[50];

void ScreenMenu_Init(void)
{
    // Initialize menu screen
    highScore = LoadHighscore();
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
    
    DrawText("TETRIS", 300, 200, 40, DARKGRAY);
    DrawText("Press ENTER to start", 290, 280, 20, GRAY);

    if(highScore > 0) {
        snprintf(scoreText, 50, "Highscore: %d", highScore );
        DrawText(scoreText, 290, 310, 30, MAROON);
    }


    
    EndDrawing();
}

void ScreenMenu_Unload(void)
{
    // Cleanup menu screen
}
