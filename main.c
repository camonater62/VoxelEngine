#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "chunk.h"


int main(void)
{
    SetTargetFPS(120);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(WIN_RES.x, WIN_RES.y, "Voxel Engine");
    DisableCursor();
    SetMousePosition(WIN_RES.x / 2, WIN_RES.y / 2);
    InitChunkGL();

    Vector3 playerPosition = {
        .x = H_CHUNK_SIZE,
        .y = CHUNK_SIZE,
        .z = 2.f * CHUNK_SIZE,
    };

    Camera camera = {
        .fovy = 60.0f,
        .position = playerPosition,
        .projection = CAMERA_PERSPECTIVE,
        .target = Vector3Subtract(playerPosition, (Vector3){0,0,1}),
        .up = (Vector3){ 0, 1, 0 },
    };

    Chunk* chunk = CreateChunk();
    Chunk* chunk2 = CreateChunk();

    while (!WindowShouldClose())
    {
        UpdateCameraPro(&camera,
            (Vector3){
                // Move forward-backward
                (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 50.0f * GetFrameTime() -      
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 50.0f * GetFrameTime(),    
                // Move right-left
                (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 50.0f * GetFrameTime() -   
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 50.0f * GetFrameTime(),
                // Move up-down
                (IsKeyDown(KEY_SPACE)) * 50.0f * GetFrameTime() -      
                (IsKeyDown(KEY_LEFT_SHIFT)) * 50.0f * GetFrameTime(),                                                 
            },
            (Vector3){
                // Rotation: yaw
                GetMouseDelta().x * 0.05f,       
                // Rotation: pitch                     
                GetMouseDelta().y * 0.05f,     
                // Rotation: roll                       
                0.0f                                                
            },
            // Move to target (zoom)
            0.0f);                              

        ClearBackground(BG_COLOR);

        BeginDrawing();
        {
            BeginMode3D(camera);
            {
                DrawChunk(chunk, &camera, Vector3Zero());
                DrawChunk(chunk2, &camera, (Vector3){50, 0, 0});
            }
            EndMode3D();

            DrawFPS(10, 10);
        }
        EndDrawing();
    }

    DestroyChunk(chunk);
    CloseChunkGL();
    CloseWindow();

    return 0;
}
