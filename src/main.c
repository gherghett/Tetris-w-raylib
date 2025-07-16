#include "raylib.h"
#include "screen_manager.c"
#include "screen_menu.c"
#include "sceen_gameplay.c"

int main(void)
{
    int screenWidth = 800;
    int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "Tetris");
    SetTargetFPS(60);

    ChangeToScreen(SCREEN_MENU);

    while (!WindowShouldClose()) {
        //-------------- INIT ---------------------------------
        ProcessScreenChange();

        //--------------------------------------------------------------
        // ------------- UPDATE ----------------------
        //--------------------------------------------------------------
        Update();
        //--------------------------------------------------------------

        //--------------------------------------------------------------
        // ------------- DRAW ----------------------
        //--------------------------------------------------------------
        Draw();
        //--------------------------------------------------------------
    }

    //clean up
    Unload();

    CloseWindow();
    return 0;
}
