#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
// #include "raymath.h" this has vector2 math if needed
#include "vectormath2i.h"
#include "shapes.h"
#include "screen_manager.h"
#include "highscore.h"

#define BLOCK_SIZE 20
#define WELL_HEIGHT 20
#define WELL_WIDTH 10

#define HOLDER_SIZE 5

#define DOWNPRESS_COOLDOWN_START 0.25 
#define DOWNPRESS_COOLDOWN 0.04

typedef enum {
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_PAUSED
} GameState;

typedef enum {
    RED_BLOCK = 1,
    BLUE_BLOCK = 2,
    GREEN_BLOCK = 3
} BlockColor;

Color Colors[4] = {
    WHITE, RED, BLUE, (Color){ 0, 190, 48, 255 }
};
Color DarkerColors[4] = {
    WHITE, MAROON, DARKBLUE, DARKGREEN
};
Color LighterColors[4] = {
    WHITE,
    (Color){ 250, 81, 75, 180 },
    (Color){ 20, 141, 255, 180 },
    (Color){ 20, 210, 68, 180 }
};

GameState currentState;
GameState unPausedState;

Sound blip;
Sound land;
Sound clear;
Sound rotate;
Sound down;

Music stackingTime;

static int staticBlocks[WELL_HEIGHT][WELL_WIDTH] = {{0}};
Vector2i tetrisHolderPos;
static int nextPiece;
static int screenWidth;
static int screenHeight;
static Vector2i holderStartPos;
static Vector2 wellPosition;
static int points;
static char pointText[100];
static int tetrisHolder[HOLDER_SIZE][HOLDER_SIZE];
static float tickLength;
static float lastTick;
static bool newHighScore;

static float downPressCoolDown;
static float leftPressCoolDown;
static float rightPressCoolDown;

//for clear soundeffect
static float timerClear;
static int cleared;

void rotateGrid90(int src[HOLDER_SIZE][HOLDER_SIZE], int dest[HOLDER_SIZE][HOLDER_SIZE], int rotation) {
    // Normalize rotation to 0, 90, 180, 270
    rotation = ((rotation % 360) + 360) % 360;
    
    if (rotation == 0) {
        memcpy(dest, src, HOLDER_SIZE * HOLDER_SIZE * sizeof(int));
        return;
    }   
    
    for (int y = 0; y < HOLDER_SIZE; y++) {
        for (int x = 0; x < HOLDER_SIZE; x++) {
            int val = src[y][x];
            
            switch (rotation) {
                case 90:
                    dest[x][HOLDER_SIZE - 1 - y] = val;
                    break;
                case 180:
                    dest[HOLDER_SIZE - 1 - y][HOLDER_SIZE - 1 - x] = val;
                    break;
                case 270:
                    dest[HOLDER_SIZE - 1 - x][y] = val;
                    break;
            }
        }
    }
}

bool holderCollides(int holder[HOLDER_SIZE][HOLDER_SIZE], Vector2i position) {
    for(int i = 0; i < HOLDER_SIZE; i++) {
        for( int j = 0; j < HOLDER_SIZE; j++) {
            int x = j+position.x;
            int y = i+position.y;
            if(holder[i][j]) {
                if( x < 0 || x >= WELL_WIDTH || y >= WELL_HEIGHT)
                    return true;
    
                if(y >= 0 && y < WELL_HEIGHT && x >= 0 && x < WELL_WIDTH 
                    && staticBlocks[y][x]) 
                    return true;
            }
        }
    }
    return false;
}

void colorHolder(int holder[HOLDER_SIZE][HOLDER_SIZE], BlockColor color) {
    for(int i = 0; i < HOLDER_SIZE; i++) {
        for( int j = 0; j < HOLDER_SIZE; j++) {
            if (holder[i][j]) {
                holder[i][j] = (int)color;
            }
        }
    }
}

bool holderTooHigh(int holder[HOLDER_SIZE][HOLDER_SIZE], Vector2i position) {
    for(int i = 0; i < HOLDER_SIZE; i++) {
        for( int j = 0; j < HOLDER_SIZE; j++) {
            int y = i+position.y;
            if(holder[i][j]) {
                if(y < 0 )
                    return true;
            }
        }
    }
    return false;
}
void DrawBlock(Rectangle block, BlockColor color) {
    DrawRectanglePro(block, (Vector2){0,0}, 0.0f, Colors[color]);
    Rectangle innerRect = {
        .x = block.x + BLOCK_SIZE*0.2,
        .y = block.y + BLOCK_SIZE*0.2,
        .height = block.height*0.6,
        .width = block.width*0.6
    };
    DrawTriangle(
        (Vector2){block.x, block.y},                       // Top-left
        (Vector2){block.x, block.y + block.height},        // Bottom-left
        (Vector2){block.x + block.width, block.y + block.height}, // Bottom-right
        DarkerColors[color]
    );
    DrawRectanglePro(innerRect, (Vector2){0,0}, 0.0f, LighterColors[color]);

}
void DrawBlockInWell(int x, int y, BlockColor color) {
    if(x < 0 || x >= WELL_WIDTH || y < 0 || y >= WELL_HEIGHT) 
        return;

    Rectangle rect = {
        .x = wellPosition.x + BLOCK_SIZE * x,
        .y = wellPosition.y + BLOCK_SIZE * y,
        .height = BLOCK_SIZE,
        .width = BLOCK_SIZE
    };

    DrawBlock(rect, color);
}


void gameTick(void) {
     //------------ONCE EVERY TICK-------------------------------
    //move tetetrisshape down, or add it to staticBlocks
    // PlaySound(down);
    Vector2i newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){0,1});
    if(!holderCollides(tetrisHolder, newPos)) {
        tetrisHolderPos = newPos;
    } else {
        tickLength = tickLength * 0.98f;
        points += 10;
        if(holderTooHigh(tetrisHolder, tetrisHolderPos)) {
            // GAME OVER
            currentState = STATE_GAME_OVER;
            int highscore = LoadHighscore();
            if(points > highscore) {
                SaveHighscore(points);
                newHighScore = true;
            }
            return;
        }
        //add holder content to static blocks
        for (int y = 0; y < HOLDER_SIZE; y++) {
            for (int x = 0; x < HOLDER_SIZE; x++) {
                if(tetrisHolder[y][x]) {
                    staticBlocks[y+tetrisHolderPos.y][x+tetrisHolderPos.x] = tetrisHolder[y][x];
                }
            }
        }
        int newShapeIndex = nextPiece;
        nextPiece = GetRandomValue(0,6);
        int color = GetRandomValue(1, 3);
        memcpy(tetrisHolder, shapes[newShapeIndex], sizeof(tetrisHolder));
        colorHolder(tetrisHolder, color);
        tetrisHolderPos = holderStartPos;

        int linesDeleted = 0;
        //check for lines to delete
        for(int i = 0; i < WELL_HEIGHT; i++) {
            bool foundEmpty = false;
            for( int j = 0; j < WELL_WIDTH; j++) {
                if( !staticBlocks[i][j] ) {
                    foundEmpty = true;
                    break;
                }
            }
            if(!foundEmpty){
                //remove line by moving lines above down
                for(int k = i-1; k > -1; k--) {
                    memcpy(staticBlocks[k+1], staticBlocks[k], sizeof(staticBlocks[k]));
                }
                linesDeleted++;
            }
        }
        points += (int)pow(3, linesDeleted-1) * 100;
        PlaySound(land);
        cleared = 4 - linesDeleted; //this will play sound effect
    }
    //------------ TICK-------------------------------
}

void ScreenGameplay_Init(void)
{
    // Check if the sound file exists before loading
    if (FileExists("resources/blip.wav")) {
        blip = LoadSound("resources/blip.wav");
    } else {
        // Handle the case where file is not found
        printf("Warning: Could not find resources/blip.wav\n");
        // Initialize with a dummy sound or handle gracefully
    }
    land = LoadSound("resources/land.wav");
    clear = LoadSound("resources/clear.wav");
    down = LoadSound("resources/down.wav");
    rotate = LoadSound("resources/rotate.wav");

    stackingTime = LoadMusicStream("resources/stacking_time.mp3");
    SetMusicVolume(stackingTime, 0.4);
    // PlayMusicStream(stackingTime);

    timerClear = 0.0f;
    cleared = 4; //four means its done, 

    memset(staticBlocks, 0, sizeof(staticBlocks));    
    
    currentState = STATE_PLAYING;
    // double startTime = GetTime();  // Record start time

    screenWidth = 800;
    screenHeight = 450;
    
    holderStartPos = (Vector2i){WELL_WIDTH/2 - HOLDER_SIZE/2, -3};
    tetrisHolderPos = holderStartPos;

    wellPosition = (Vector2){60, 30}; //offset 
    
    //players points
    points = 0;
    newHighScore = false;

    // int emptyHolder[HOLDER_SIZE][HOLDER_SIZE] = {0};
    
    memcpy(tetrisHolder, four, sizeof(grib));
    nextPiece = GetRandomValue(0,6);


    tickLength = 0.5;
    lastTick = tickLength;

    // InitWindow(screenWidth, screenHeight, "Tetris");
    // SetTargetFPS(60);

    downPressCoolDown = 0.0;
    leftPressCoolDown = 0.0;
    rightPressCoolDown = 0.0;
}

void ScreenGameplay_Update(void) {
    //--------------------------------------------------------------
    // ------------- UPDATE ----------------------
    //--------------------------------------------------------------
    lastTick -= GetFrameTime();

    UpdateMusicStream(stackingTime);   // Update music buffer with new stream data


    Vector2i newPos = {0,0};
    if(lastTick <= 0.0f) {
        switch (currentState) {
            case STATE_PLAYING: gameTick(); break;
            default: break;
        }
        lastTick = tickLength;
    }

    //Input handling -----------------------------------------
    if(IsKeyPressed(KEY_P)) {
        
        if(currentState == STATE_PAUSED) {
            currentState = unPausedState;
        } else {
            unPausedState = currentState;
            currentState = STATE_PAUSED;
        }
    }

    switch (currentState) {
        case STATE_PLAYING: {
            if(IsKeyPressed(KEY_DOWN)) {
                lastTick = 0.0f;
                PlaySound(blip);
                downPressCoolDown = DOWNPRESS_COOLDOWN_START;
            }
            downPressCoolDown -= GetFrameTime();
            if(IsKeyDown(KEY_DOWN) && downPressCoolDown <= 0.0) {
                lastTick = 0.0f;
                PlaySound(blip);
                downPressCoolDown = DOWNPRESS_COOLDOWN;
            }

            //Rotation
            int newHolder[HOLDER_SIZE][HOLDER_SIZE] = {0};
            if(IsKeyPressed(KEY_D)) {
                PlaySound(rotate);
                rotateGrid90(tetrisHolder, newHolder, 90);
                if(!holderCollides(newHolder, tetrisHolderPos)) {
                    memcpy(tetrisHolder, newHolder, sizeof(newHolder));
                }
            }
            if(IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP)) {
                PlaySound(rotate);
                rotateGrid90(tetrisHolder, newHolder, 270);
                if(!holderCollides(newHolder, tetrisHolderPos)) {
                    memcpy(tetrisHolder, newHolder, sizeof(newHolder));
                }
            }

            //left right
            if(IsKeyPressed(KEY_RIGHT)) {
                PlaySound(blip);
                newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){1, 0});
                if(!holderCollides(tetrisHolder, newPos)) {
                    tetrisHolderPos = newPos;
                }
                rightPressCoolDown = DOWNPRESS_COOLDOWN_START;
            }
            rightPressCoolDown -= GetFrameTime();
            if(IsKeyDown(KEY_RIGHT) && rightPressCoolDown <= 0.0) {
                PlaySound(blip);
                newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){1, 0});
                if(!holderCollides(tetrisHolder, newPos)) {
                    tetrisHolderPos = newPos;
                }
                rightPressCoolDown = DOWNPRESS_COOLDOWN;
            }

            if(IsKeyPressed(KEY_LEFT)) {
                PlaySound(blip);
                newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){-1,0});
                if(!holderCollides(tetrisHolder, newPos)) {
                    tetrisHolderPos = newPos;
                }
                leftPressCoolDown = DOWNPRESS_COOLDOWN_START;
            }
            leftPressCoolDown -= GetFrameTime();
            if(IsKeyDown(KEY_LEFT) && leftPressCoolDown <= 0.0) {
                PlaySound(blip);
                newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){-1,0});
                if(!holderCollides(tetrisHolder, newPos)) {
                    tetrisHolderPos = newPos;
                }
                leftPressCoolDown = DOWNPRESS_COOLDOWN;
            }
            break;
        }
        case STATE_GAME_OVER: {
            if(IsKeyPressed(KEY_ENTER)) {
                ChangeToScreen(SCREEN_GAMEPLAY);
            }
            break;
        }
        default: break;
    }

    if(cleared < 4) {
        timerClear -= GetFrameTime();
        if(timerClear <= 0.0f) {
            SetSoundPitch(clear, 1.0 + 0.2
                 * cleared);
            PlaySound(clear);
            cleared++;
            timerClear = 0.4f;
        }
    }

    //--------------------------------------------------------------
}

void ScreenGameplay_Draw(void) {
    //--------------------------------------------------------------
    // ------------- DRAW ----------------------
    //--------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        snprintf(pointText, 100, "points: %d", points );
        DrawText(pointText, 300, 20, 30, LIGHTGRAY);
        
        for(int i = 0; i < WELL_HEIGHT; i++) {
            for( int j = 0; j < WELL_WIDTH; j++) {
                Rectangle rect = {wellPosition.x + j*BLOCK_SIZE, wellPosition.y + i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
                
                if(staticBlocks[i][j]) {
                    BlockColor color = staticBlocks[i][j];
                    DrawBlockInWell(j, i, color);
                } else {
                    DrawRectangleLinesEx(rect, 3.0, LIGHTGRAY);
                }
            }
        }

        Rectangle block = {0,0, BLOCK_SIZE, BLOCK_SIZE};
        for(int i = 0; i < HOLDER_SIZE; i++) {
            for(int j = 0; j < HOLDER_SIZE; j++){
                if(tetrisHolder[i][j]) {
                    BlockColor color = tetrisHolder[i][j];
                    block.x = wellPosition.x + tetrisHolderPos.x*BLOCK_SIZE + BLOCK_SIZE*j,
                    block.y = wellPosition.y + tetrisHolderPos.y*BLOCK_SIZE + BLOCK_SIZE*i,
                    DrawBlock(block, color);
                }
            }
        }

        if(currentState == STATE_PAUSED)
            DrawText("PAUSED", 300, 20, 120, BLACK);

        if(currentState == STATE_GAME_OVER) {
            if(newHighScore) {
                DrawText("NEW HIGH SCORE!", 30, 100, 80, BLACK);
                DrawText(pointText, 30, 250, 50, RED);
            }else {
                DrawText("GAME OVER", 30, 100, 120, BLACK);
            }
            DrawText("press enter to restart", 30, 200, 30, BLACK);
        }


    EndDrawing();

    //--------------------------------------------------------------
}

void ScreenGameplay_Unload(void){
    StopMusicStream(stackingTime);

    UnloadSound(blip);
    UnloadSound(land);
    UnloadSound(clear);
    UnloadSound(rotate);
    UnloadSound(down);
    UnloadMusicStream(stackingTime);
}