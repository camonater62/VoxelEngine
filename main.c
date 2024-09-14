#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "rcamera.h"

int main(void)
{
    SetTargetFPS(120);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(WIN_RES.x, WIN_RES.y, "Voxel Engine");
    DisableCursor();
    SetMousePosition(WIN_RES.x / 2, WIN_RES.y / 2);

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

    char* vShaderStr = LoadFileText("chunk.vert");
    char* fShaderStr = LoadFileText("chunk.frag");
    int chunkShaderId = rlLoadShaderCode(vShaderStr, fShaderStr);
    assert(chunkShaderId > 0);

    Chunk* chunk = CreateChunk();
    int chunkvao = rlLoadVertexArray();
    assert(chunkvao > 0);
    int chunkvbo = rlLoadVertexBuffer(chunk->vertices, chunk->vertexCount * chunk->vertexSize, false);
    assert(chunkvbo > 0);


    rlEnableVertexArray(chunkvao);
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 1, RL_UNSIGNED_BYTE, false, 5, 0);
    rlEnableVertexAttribute(1);
    rlSetVertexAttribute(1, 1, RL_UNSIGNED_BYTE, false, 5, 1);
    rlEnableVertexAttribute(2);
    rlSetVertexAttribute(2, 1, RL_UNSIGNED_BYTE, false, 5, 2);
    rlEnableVertexAttribute(3);
    rlSetVertexAttribute(3, 1, RL_UNSIGNED_BYTE, false, 5, 3);
    rlEnableVertexAttribute(4);
    rlSetVertexAttribute(4, 1, RL_UNSIGNED_BYTE, false, 5, 4);

    int mvpUniformLoc = rlGetLocationUniform(chunkShaderId, "mvp");
    assert(mvpUniformLoc >= 0);

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
            GetMouseWheelMove()*2.0f);                              

        ClearBackground(BG_COLOR);

        BeginDrawing();
        {
            BeginMode3D(camera);
            {
                rlDisableBackfaceCulling();
                rlEnableShader(chunkShaderId);
                // Matrix matView = rlGetMatrixModelview();
                // Matrix matProjection = MatrixPerspective(camera.fovy * DEG2RAD, 16.0/9, 0.01, 1000); // rlGetMatrixProjection();
                // Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
                Matrix matModel = MatrixIdentity();
                Matrix matView = GetCameraViewMatrix(&camera);
                Matrix matModelView = MatrixMultiply(matModel, matView);
                Matrix matProjection = GetCameraProjectionMatrix(&camera, WIN_RES.x / WIN_RES.y);
                Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
                
                rlSetUniformMatrix(mvpUniformLoc, matModelViewProjection);
                // rlSetUniformMatrix(mvpUniformLoc, matModelView);
                assert(rlEnableVertexArray(chunkvao));
                rlDrawVertexArray(0, chunk->vertexCount);
            }
            EndMode3D();
        }
        EndDrawing();

        char titlebuf[50] = {0};
        snprintf(titlebuf, 50, "%0.0f FPS", 1.0f / GetFrameTime());
        SetWindowTitle(titlebuf);
    }

    rlUnloadVertexArray(chunkvao);
    rlUnloadVertexBuffer(chunkvbo);

    DestroyChunk(chunk);
    CloseWindow();

    return 0;
}
