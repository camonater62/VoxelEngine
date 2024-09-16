#include "chunk.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "rcamera.h"

#include "glad.h"
#include <stb_perlin.h>

static int chunkShaderId = -1;
static int chunkMvpLoc = -1;
static int chunkTextureLoc = -1;

static Texture chunkTexture;

Chunk* CreateChunk(void) {
    Chunk* c = malloc(sizeof(Chunk));
    c->voxels = calloc(CHUNK_VOLUME, sizeof(uint8_t));
    c->vertices = NULL;
    c->vertexCount = 0;
    c->vertexSize = 0;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                if (stb_perlin_noise3(0.1f * x, 0.1f * y, 0.1f * z, 0, 0, 0) > 0) {
                    c->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y] = x + y + z;
                } else {
                    c->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y] = 0;
                }
            }
        }
    }

    GenChunkMesh(c);

    c->vao = rlLoadVertexArray();
    assert(c->vao > 0);
    c->vbo = rlLoadVertexBuffer(c->vertices, c->vertexCount * c->vertexSize, false);
    assert(c->vbo > 0);

    rlEnableVertexArray(c->vao);
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, 5, (void *) 0);
    rlEnableVertexAttribute(0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 5, (void *) 3);
    rlEnableVertexAttribute(1);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 5, (void *) 4);
    rlEnableVertexAttribute(2);

    rlDisableVertexArray();
    rlDisableVertexBuffer();

    return c;
}

void InitChunkGL(void) {
    char* vShaderStr = LoadFileText("chunk.vert");
    char* fShaderStr = LoadFileText("chunk.frag");
    chunkShaderId = rlLoadShaderCode(vShaderStr, fShaderStr);
    assert(chunkShaderId > 0);

    UnloadFileText(vShaderStr);
    UnloadFileText(fShaderStr);

    chunkMvpLoc = rlGetLocationUniform(chunkShaderId, "mvp");
    assert(chunkMvpLoc >= 0);
    chunkTextureLoc = rlGetLocationUniform(chunkShaderId, "u_texture_0");
    assert(chunkTextureLoc >= 0);
    
    rlDisableShader();

    Image image = LoadImage("frame.png");
    ImageFlipHorizontal(&image);
    chunkTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    rlTextureParameters(chunkTexture.id, RL_TEXTURE_FILTER_ANISOTROPIC, 16);
    assert(IsTextureReady(chunkTexture));
}

void CloseChunkGL(void) {
    UnloadTexture(chunkTexture);
    rlUnloadShaderProgram(chunkShaderId);
}

void DrawChunk(Chunk *c, Camera* camera, Vector3 position) {
    rlEnableShader(chunkShaderId);

    Matrix matModel = MatrixTranslate(position.x, position.y, position.z);
    Matrix matView = GetCameraViewMatrix(camera);
    Matrix matModelView = MatrixMultiply(matModel, matView);
    Matrix matProjection = GetCameraProjectionMatrix(camera, WIN_RES.x / WIN_RES.y);
    Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
    
    rlEnableTexture(chunkTexture.id);
    rlSetUniformMatrix(chunkMvpLoc, matModelViewProjection);
    rlSetUniform(chunkTextureLoc, &chunkTexture.id, RL_SHADER_UNIFORM_UINT, 1);
    assert(rlEnableVertexArray(c->vao));
    rlDrawVertexArray(0, c->vertexCount);
}

void DestroyChunk(Chunk* c) {
    if (c) {
        if (c->voxels) {
            free(c->voxels);
        }
        if (c->vertices) {
            free(c->vertices);
        }

        rlUnloadVertexArray(c->vao);
        rlUnloadVertexBuffer(c->vbo);

        free(c);
    }
}

bool isVoid(Chunk *c, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE 
     || y < 0 || y >= CHUNK_SIZE 
     || z < 0 || z >= CHUNK_SIZE) {
        return true;
    }

    return (c->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y] == 0);
}

void GenChunkMesh(Chunk *chunk)
{
    // vertex attributes:
    // 0-2 x,y,z
    // 3   voxel_id
    // 4   face_id
    chunk->vertexSize = 5;
    uint8_t* vertex_data = (uint8_t*) calloc(CHUNK_VOLUME * 18 * chunk->vertexSize, sizeof(uint8_t));
    int index = 0;

    #define PUSH_VERTEX(vertex) \
        memcpy(vertex_data + index, vertex, sizeof(vertex)); \
        index += sizeof(vertex); 

    #define PUSH_FACE(a, b, c, d, e, f) \
        PUSH_VERTEX(a); \
        PUSH_VERTEX(b); \
        PUSH_VERTEX(c); \
        PUSH_VERTEX(d); \
        PUSH_VERTEX(e); \
        PUSH_VERTEX(f); 

    for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
        for (uint8_t y = 0; y < CHUNK_SIZE; y++) {
            for (uint8_t z = 0; z < CHUNK_SIZE; z++) {
                uint8_t voxel_id = chunk->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y];
                if (voxel_id == 0) {
                    continue;
                }

                // Top face
                if (isVoid(chunk, x, y + 1, z)) {
                    uint8_t v0[5] = { x,     y + 1, z,     voxel_id, 0};
                    uint8_t v1[5] = { x + 1, y + 1, z,     voxel_id, 0};
                    uint8_t v2[5] = { x + 1, y + 1, z + 1, voxel_id, 0};
                    uint8_t v3[5] = { x,     y + 1, z + 1, voxel_id, 0};

                    PUSH_FACE(v0, v3, v2, v0, v2, v1);
                }

                // Bottom face
                if (isVoid(chunk, x, y - 1, z)) {
                    uint8_t v0[5] = {x,     y, z,     voxel_id, 1}; 
                    uint8_t v1[5] = {x + 1, y, z,     voxel_id, 1}; 
                    uint8_t v2[5] = {x + 1, y, z + 1, voxel_id, 1}; 
                    uint8_t v3[5] = {x,     y, z + 1, voxel_id, 1}; 
                    
                    PUSH_FACE(v0, v2, v3, v0, v1, v2);                    
                }

                // Right face
                if (isVoid(chunk, x + 1, y, z)) {
                    uint8_t v0[5] = {x + 1, y,     z,     voxel_id, 2};
                    uint8_t v1[5] = {x + 1, y + 1, z,     voxel_id, 2};
                    uint8_t v2[5] = {x + 1, y + 1, z + 1, voxel_id, 2};
                    uint8_t v3[5] = {x + 1, y,     z + 1, voxel_id, 2};

                    PUSH_FACE(v0, v1, v2, v0, v2, v3);
                }

                // Left face
                if (isVoid(chunk, x - 1, y, z)) {
                    uint8_t v0[5] = {x, y,     z,     voxel_id, 3};
                    uint8_t v1[5] = {x, y + 1, z,     voxel_id, 3};
                    uint8_t v2[5] = {x, y + 1, z + 1, voxel_id, 3};
                    uint8_t v3[5] = {x, y,     z + 1, voxel_id, 3};

                    PUSH_FACE(v0, v2, v1, v0, v3, v2);
                }

                // Back face
                if (isVoid(chunk, x, y, z - 1)) {
                    uint8_t v0[5] = {x,     y,     z, voxel_id, 4};
                    uint8_t v1[5] = {x,     y + 1, z, voxel_id, 4};
                    uint8_t v2[5] = {x + 1, y + 1, z, voxel_id, 4};
                    uint8_t v3[5] = {x + 1, y,     z, voxel_id, 4};

                    PUSH_FACE(v0, v1, v2, v0, v2, v3);
                }

                // Front face
                if (isVoid(chunk, x, y, z + 1)) {
                    uint8_t v0[5] = {x,     y,     z + 1, voxel_id, 5};
                    uint8_t v1[5] = {x,     y + 1, z + 1, voxel_id, 5};
                    uint8_t v2[5] = {x + 1, y + 1, z + 1, voxel_id, 5};
                    uint8_t v3[5] = {x + 1, y,     z + 1, voxel_id, 5};

                    PUSH_FACE(v0, v2, v1, v0, v3, v2);
                }
            }
        }
    }

    if (chunk->vertices) {
        free(chunk->vertices);
    }

    chunk->vertices = vertex_data;
    chunk->vertexCount = index / chunk->vertexSize;
}
