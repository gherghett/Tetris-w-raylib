#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
// #include "raymath.h"
#include "vectormath2i.h"

#define BLOCK_SIZE 20
#define WELL_HEIGHT 20
#define WELL_WIDTH 10

#define HOLDER_SIZE 5

static int staticBlocks[WELL_HEIGHT][WELL_WIDTH] = {{0}};
Vector2i tetrisHolderPos;

void rotateGrid90(int src[HOLDER_SIZE][HOLDER_SIZE], int dest[HOLDER_SIZE][HOLDER_SIZE], int rotation) {
    // Normalize rotation to 0, 90, 180, 270
    rotation = ((rotation % 360) + 360) % 360;
    
    if (rotation == 0) return; // No rotation needed
    
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
    
                if(staticBlocks[y][x]) 
                    return true;
            }
        }
    }
    return false;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    // double startTime = GetTime();  // Record start time

    Vector2i holderStartPos = {WELL_WIDTH/2 - HOLDER_SIZE/2, -3};
    tetrisHolderPos = holderStartPos;

    Vector2 wellPosition = {60, 30};

    char pointText[100];

    staticBlocks[19][9] = 1;

    int points = 0;
    
    int grib[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };

    int zig[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 1, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };


    int zag[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };

    int el[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 1, 0},
        {0, 0, 0, 0, 0}
    };

    int le[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 0, 0, 0}
    };

    int square[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 0, 0, 0}
    };

    int four[HOLDER_SIZE][HOLDER_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    int (*shapes[7])[HOLDER_SIZE] = {
        grib, zig, zag, el, le, square, four
    };

    // int emptyHolder[HOLDER_SIZE][HOLDER_SIZE] = {0};
    
    int tetrisHolder[HOLDER_SIZE][HOLDER_SIZE];
    memcpy(tetrisHolder, four, sizeof(grib));

    float tickLength = 0.5;
    float lastTick = tickLength;

    InitWindow(screenWidth, screenHeight, "Tetris");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        int newHolder[HOLDER_SIZE][HOLDER_SIZE] = {0};
        // double time = startTime - GetTime();
        lastTick -= GetFrameTime();
        if(IsKeyPressed(KEY_DOWN)) {
            lastTick = 0.0f;
        }
        Vector2i newPos = {0,0};
        if(lastTick <= 0.0f) {
            newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){0,1});

            if(!holderCollides(tetrisHolder, newPos)) {
                tetrisHolderPos = newPos;
            } else {
                points += 10;

                //add holder contett to static blocks
                for (int y = 0; y < HOLDER_SIZE; y++) {
                    for (int x = 0; x < HOLDER_SIZE; x++) {
                        if(tetrisHolder[y][x]) {
                            staticBlocks[y+tetrisHolderPos.y][x+tetrisHolderPos.x] = 1;
                        }
                    }
                }
                int newShapeIndex = GetRandomValue(0,6);
                memcpy(tetrisHolder, shapes[newShapeIndex], sizeof(tetrisHolder));
                tetrisHolderPos = holderStartPos;

                int linesDeleted = 0;
                //check for lines
                for(int i = 0; i < WELL_HEIGHT; i++) {
                    bool foundEmpty = false;
                    for( int j = 0; j < WELL_WIDTH; j++) {
                        if( !staticBlocks[i][j] ) {
                            foundEmpty = true;
                            break;
                        }
                    }
                    if(!foundEmpty){
                        //remove line
                        for(int k = i-1; k > -1; k--) {
                            memcpy(staticBlocks[k+1], staticBlocks[k], sizeof(staticBlocks[k]));
                        }
                        linesDeleted++;
                    }
                }
                points += (int)pow(2, linesDeleted-1) * 100;
            }
            lastTick = tickLength;
        }

        if(IsKeyPressed(KEY_D)) {
            rotateGrid90(tetrisHolder, newHolder, 90);
            if(!holderCollides(newHolder, tetrisHolderPos)) {
                memcpy(tetrisHolder, newHolder, sizeof(newHolder));
            }
        }
        if(IsKeyPressed(KEY_A)) {
            rotateGrid90(tetrisHolder, newHolder, 270);
            if(!holderCollides(newHolder, tetrisHolderPos)) {
                memcpy(tetrisHolder, newHolder, sizeof(newHolder));
            }
        }
        if(IsKeyPressed(KEY_RIGHT)) {
            newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){1, 0});
            if(!holderCollides(tetrisHolder, newPos)) {
                tetrisHolderPos = newPos;
            }
        }
        if(IsKeyPressed(KEY_LEFT)) {
            newPos = Vector2iAdd(tetrisHolderPos, (Vector2i){-1,0});
            if(!holderCollides(tetrisHolder, newPos)) {
                tetrisHolderPos = newPos;
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            snprintf(pointText, 100, "points: %d", points );
            DrawText(pointText, 300, 20, 30, LIGHTGRAY);
            
            for(int i = 0; i < WELL_HEIGHT; i++) {
                for( int j = 0; j < WELL_WIDTH; j++) {
                    Rectangle rect = {wellPosition.x + j*BLOCK_SIZE, wellPosition.y + i*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
                    if(staticBlocks[i][j]) {
                        DrawRectanglePro(rect, (Vector2){0,0}, 0.0f, RED);
                    } else {
                        DrawRectangleLinesEx(rect, 3.0, LIGHTGRAY);
                    }
                }
            }

            Rectangle block ={0,0, BLOCK_SIZE, BLOCK_SIZE};
            for(int i = 0; i < HOLDER_SIZE; i++) {
                for(int j = 0; j < HOLDER_SIZE; j++){
                    if(tetrisHolder[i][j]) {
                        block.x = wellPosition.x + tetrisHolderPos.x*BLOCK_SIZE + BLOCK_SIZE*j,
                        block.y = wellPosition.y + tetrisHolderPos.y*BLOCK_SIZE + BLOCK_SIZE*i,
                        DrawRectanglePro(block, (Vector2){0,0}, 0.0f, BLUE);
                    }
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
