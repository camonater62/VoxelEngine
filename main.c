#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "rcamera.h"

#include "glad.h"

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

    UnloadFileText(vShaderStr);
    UnloadFileText(fShaderStr);

    Chunk* chunk = CreateChunk();
    int chunkvao = rlLoadVertexArray();
    assert(chunkvao > 0);
    int chunkvbo = rlLoadVertexBuffer(chunk->vertices, chunk->vertexCount * chunk->vertexSize, false);
    assert(chunkvbo > 0);

    rlEnableVertexArray(chunkvao);
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, 5, (void *) 0);
    rlEnableVertexAttribute(0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 5, (void *) 3);
    rlEnableVertexAttribute(1);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 5, (void *) 4);
    rlEnableVertexAttribute(2);

    int mvpUniformLoc = rlGetLocationUniform(chunkShaderId, "mvp");
    assert(mvpUniformLoc >= 0);
    int texture0Loc = rlGetLocationUniform(chunkShaderId, "u_texture_0");
    assert(texture0Loc >= 0);
    
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableShader();

    Image testImage = LoadImage("frame.png");
    ImageFlipHorizontal(&testImage);
    Texture testTexture = LoadTextureFromImage(testImage);
    rlTextureParameters(testTexture.id, RL_TEXTURE_FILTER_ANISOTROPIC, 16);
    assert(IsTextureReady(testTexture));

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
                rlEnableShader(chunkShaderId);

                Matrix matModelView = GetCameraViewMatrix(&camera);
                Matrix matProjection = GetCameraProjectionMatrix(&camera, WIN_RES.x / WIN_RES.y);
                Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
                
                rlEnableTexture(testTexture.id);
                rlSetUniformMatrix(mvpUniformLoc, matModelViewProjection);
                rlSetUniform(texture0Loc, &testTexture.id, RL_SHADER_UNIFORM_UINT, 1);
                assert(rlEnableVertexArray(chunkvao));
                rlDrawVertexArray(0, chunk->vertexCount);
            }
            EndMode3D();

            DrawFPS(10, 10);
        }
        EndDrawing();
    }

    UnloadImage(testImage);
    UnloadTexture(testTexture);

    rlUnloadVertexArray(chunkvao);
    rlUnloadVertexBuffer(chunkvbo);
    rlUnloadShaderProgram(chunkShaderId);

    DestroyChunk(chunk);
    CloseWindow();

    return 0;
}
