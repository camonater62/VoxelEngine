#include <stdio.h>

#include "common.h"

int main(void)
{
    SetTargetFPS(60);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIN_RES.x, WIN_RES.y, "Voxel Engine");
    DisableCursor();
    SetMousePosition(WIN_RES.x / 2, WIN_RES.y / 2);

    Shader quadShader = LoadShader("quad.vert", "quad.frag");
    Material quadMaterial = LoadMaterialDefault();

    Mesh quadMesh = {0};
    float quadVertices[6 * 3] = {
        // Triangle 1
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        // Triangle 2
        0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
    };
    uint8_t quadColors[6 * 4] = {
        // Triangle 1
        0, 255, 0, 255,
        255, 0, 0, 255,
        255, 255, 0, 255,
        // Triangle 2
        0, 255, 0, 255,
        255, 255, 0, 255,
        0, 0, 255, 255,
    }; 

    quadMesh.triangleCount = 2;
    quadMesh.vertexCount = 6;
    quadMesh.vertices = quadVertices;
    quadMesh.colors = quadColors;
    UploadMesh(&quadMesh, false);

    Model quadModel = LoadModelFromMesh(quadMesh);

    Camera camera = {
        .fovy = 60.0f,
        .position = (Vector3){ 0, 0, 1 },
        .projection = CAMERA_PERSPECTIVE,
        .target = (Vector3){ 0, 0, 0},
        .up = (Vector3){ 0, 1, 0 },
    };


    while (!WindowShouldClose())
    {
        UpdateCameraPro(&camera,
            (Vector3){
                // Move forward-backward
                (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 5.0f * GetFrameTime() -      
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 5.0f * GetFrameTime(),    
                // Move right-left
                (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 5.0f * GetFrameTime() -   
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 5.0f * GetFrameTime(),
                // Move up-down
                (IsKeyDown(KEY_SPACE)) * 5.0f * GetFrameTime() -      
                (IsKeyDown(KEY_LEFT_SHIFT)) * 5.0f * GetFrameTime(),                                                 
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
            GetMouseWheelMove()*2.0f);                              

        ClearBackground(BACKGROUND_COLOR);

        BeginDrawing();
        {
            BeginMode3D(camera);
            {
                DrawModel(quadModel, (Vector3){0, 0, 0}, 1.0f, WHITE);
            }
            EndMode3D();
        }
        EndDrawing();

        char titlebuf[50] = {0};
        snprintf(titlebuf, 50, "%0.0f FPS", 1.0f / GetFrameTime());
        SetWindowTitle(titlebuf);
    }

    // UnloadModel(quadModel);
    // UnloadMesh(quadMesh);
    UnloadMaterial(quadMaterial);
    UnloadShader(quadShader);

    CloseWindow();

    return 0;
}
