#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "world.h"


int main(void) {
    SetTargetFPS(0);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(WIN_RES.x, WIN_RES.y, "Voxel Engine");
    DisableCursor();
    SetMousePosition(WIN_RES.x / 2, WIN_RES.y / 2);
    InitChunkGL();

    Vector3 playerPosition = {
        .x = CENTER_XZ,
        .y = WORLD_H * CHUNK_SIZE,
        .z = CENTER_XZ,
    };

    Camera camera = {
        .fovy = 60.0f,
        .position = playerPosition,
        .projection = CAMERA_PERSPECTIVE,
        .target = Vector3Subtract(playerPosition, (Vector3){0,0,1}),
        .up = (Vector3){ 0, 1, 0 },
    };

    World* world = CreateWorld();

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
                DrawWorld(world, &camera);
            }
            EndMode3D();

            DrawFPS(10, 10);
            char buf[50] = {0};
            snprintf(buf, sizeof(buf), 
                     "X %.2f  Y %.2f  Z %.2f", 
                     camera.position.x, 
                     camera.position.y, 
                     camera.position.z);
            DrawText(buf, 10, 50, 20, RAYWHITE);
        }
        EndDrawing();
    }

    DestroyWorld(world);
    CloseChunkGL();
    CloseWindow();

    return 0;
}
