#include "chunk.h"

#include "world.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "rcamera.h"
#include "math.h"

#include "glad.h"
#include <stb_perlin.h>
#include <stb_ds.h>

static int chunkShaderId = -1;
static int chunkMvpLoc = -1;
static int chunkTextureLoc = -1;

static Texture chunkTexture;

Chunk* CreateChunk(World *w, Vector3 position, int chunk_index) {
    Chunk* c = malloc(sizeof(Chunk));
    c->voxels = calloc(CHUNK_VOLUME, sizeof(uint8_t));
    w->voxels[chunk_index] = c->voxels;
    c->vertices = NULL;
    c->vertexCount = 0;
    c->vertexSize = 0;
    c->chunk_index = chunk_index;
    c->position = position;
    c->world = w;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            float wx = 0.01f * (x + position.x);
            float wz = 0.01f * (z + position.z);
            int world_height = (int)(64 * stb_perlin_noise3(wx, 0, wz, 0, 0, 0) + 64);
            int local_height = world_height - position.y;
            if (local_height > CHUNK_SIZE) {
                local_height = CHUNK_SIZE;
            }
            for (int y = 0; y < local_height; y++) {
                c->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y] = y + position.y + 1;
            }
        }
    }

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

void DrawChunk(Chunk *c, Camera* camera) {
    if (c->vertexCount == 0) {
        return;
    }

    rlEnableShader(chunkShaderId);

    Matrix matModel = MatrixTranslate(c->position.x, c->position.y, c->position.z);
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
        arrfree(c->vertices);

        rlUnloadVertexArray(c->vao);
        rlUnloadVertexBuffer(c->vbo);

        free(c);
    }
}

Vector4 getAO(Chunk *chunk, FacePlane plane, int x, int y, int z) {
    int a,b,c,d,e,f,g,h;

    switch (plane)
    {
    case PLANE_X:
        a = (int) isVoid(chunk, x, y    , z - 1);
        b = (int) isVoid(chunk, x, y - 1, z - 1);
        c = (int) isVoid(chunk, x, y - 1, z    );
        d = (int) isVoid(chunk, x, y - 1, z + 1);
        e = (int) isVoid(chunk, x, y    , z + 1);
        f = (int) isVoid(chunk, x, y + 1, z + 1);
        g = (int) isVoid(chunk, x, y + 1, z    );
        h = (int) isVoid(chunk, x, y + 1, z - 1);
        break;
    case PLANE_Y:
        a = (int) isVoid(chunk, x    , y, z - 1);
        b = (int) isVoid(chunk, x - 1, y, z - 1);
        c = (int) isVoid(chunk, x - 1, y, z    );
        d = (int) isVoid(chunk, x - 1, y, z + 1);
        e = (int) isVoid(chunk, x    , y, z + 1);
        f = (int) isVoid(chunk, x + 1, y, z + 1);
        g = (int) isVoid(chunk, x + 1, y, z    );
        h = (int) isVoid(chunk, x + 1, y, z - 1);
        break;
    case PLANE_Z:
        a = (int) isVoid(chunk, x - 1, y    , z);
        b = (int) isVoid(chunk, x - 1, y - 1, z);
        c = (int) isVoid(chunk, x    , y - 1, z);
        d = (int) isVoid(chunk, x + 1, y - 1, z);
        e = (int) isVoid(chunk, x + 1, y    , z);
        f = (int) isVoid(chunk, x + 1, y + 1, z);
        g = (int) isVoid(chunk, x    , y + 1, z);
        h = (int) isVoid(chunk, x - 1, y + 1, z);
        break;
    default:
        assert(false);
        break;
    }

    return (Vector4){
        .x = (a + b + c),
        .y = (g + h + a),
        .z = (e + f + g),
        .w = (c + d + e),
    };
}

bool isVoid(Chunk *c, int x, int y, int z) {
    int wx = x + c->position.x;
    int wy = y + c->position.y;
    int wz = z + c->position.z;

    if (wx < 0 || wx >= WORLD_W * CHUNK_SIZE 
     || wy < 0 || wy >= WORLD_H * CHUNK_SIZE
     || wz < 0 || wz >= WORLD_D * CHUNK_SIZE) {
        return true;
    }

    int cx = (x + c->position.x) / CHUNK_SIZE;
    int cy = (y + c->position.y) / CHUNK_SIZE;
    int cz = (z + c->position.z) / CHUNK_SIZE;

    int chunk_index = cx + WORLD_W * cz + WORLD_AREA * cy;
    uint8_t *chunk_voxels = c->world->voxels[chunk_index];

    x = (x + CHUNK_SIZE) % CHUNK_SIZE;
    y = (y + CHUNK_SIZE) % CHUNK_SIZE;
    z = (z + CHUNK_SIZE) % CHUNK_SIZE;
    
    int voxel_index = x + z * CHUNK_SIZE + y * CHUNK_AREA;

    return (chunk_voxels[voxel_index] == 0);
}

uint32_t packVertexData(uint8_t x, uint8_t y, uint8_t z, 
                        uint8_t voxel_id, uint8_t face_id, 
                        uint8_t ao_id, uint8_t flip_id) {
    
    x &= 63;
    y &= 63;
    z &= 63;
    voxel_id &= 255;
    face_id &= 7;
    ao_id &= 3;
    flip_id &= 1;

    uint32_t data = 
        x        << 26 |
        y        << 20 |
        z        << 14 |
        voxel_id << 6 |
        face_id  << 3 |
        ao_id    << 1 |
        flip_id  << 0;

    return data;
}

void GenChunkMesh(Chunk *chunk)
{
    // vertex attributes:
    // 0-2 x,y,z
    // 3   voxel_id
    // 4   face_id
    // 5   ambient occlusion
    // 6   flip_id
    // all attributes are packed into 32 bits
    chunk->vertexSize = 4;
    uint32_t* vertex_data = NULL;

    #define PUSH_VERTEX(vertex) \
        arrpush(vertex_data, packVertexData( \
                vertex[0], \
                vertex[1], \
                vertex[2], \
                vertex[3], \
                vertex[4], \
                vertex[5], \
                vertex[6]  \
            )); 

    #define PUSH_FACE(a, b, c, d, e, f) \
        PUSH_VERTEX(a); \
        PUSH_VERTEX(b); \
        PUSH_VERTEX(c); \
        PUSH_VERTEX(d); \
        PUSH_VERTEX(e); \
        PUSH_VERTEX(f); 

    for (uint8_t y = 0; y < CHUNK_SIZE; y++) {
        for (uint8_t z = 0; z < CHUNK_SIZE; z++) {
            for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
                uint8_t voxel_id = chunk->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y];
                if (voxel_id == 0) {
                    continue;
                }

                // Top face
                if (isVoid(chunk, x, y + 1, z)) {
                    Vector4 ao = getAO(chunk, PLANE_Y, x, y + 1, z);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = { x,     y + 1, z,     voxel_id, 0, ao.x, flip_id};
                    uint8_t v1[7] = { x + 1, y + 1, z,     voxel_id, 0, ao.y, flip_id};
                    uint8_t v2[7] = { x + 1, y + 1, z + 1, voxel_id, 0, ao.z, flip_id};
                    uint8_t v3[7] = { x,     y + 1, z + 1, voxel_id, 0, ao.w, flip_id};
                    
                    if (flip_id) {
                        PUSH_FACE(v1, v0, v3, v1, v3, v2);
                    } else {
                        PUSH_FACE(v0, v3, v2, v0, v2, v1);
                    }
                }

                // Bottom face
                if (isVoid(chunk, x, y - 1, z)) {
                    Vector4 ao = getAO(chunk, PLANE_Y, x, y - 1, z);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = {x,     y, z,     voxel_id, 1, ao.x, flip_id}; 
                    uint8_t v1[7] = {x + 1, y, z,     voxel_id, 1, ao.y, flip_id}; 
                    uint8_t v2[7] = {x + 1, y, z + 1, voxel_id, 1, ao.z, flip_id}; 
                    uint8_t v3[7] = {x,     y, z + 1, voxel_id, 1, ao.w, flip_id}; 
                    
                    if (flip_id) {
                        PUSH_FACE(v1, v3, v0, v1, v2, v3);
                    } else {
                        PUSH_FACE(v0, v2, v3, v0, v1, v2);                    
                    }
                }

                // Right face
                if (isVoid(chunk, x + 1, y, z)) {
                    Vector4 ao = getAO(chunk, PLANE_X, x + 1, y, z);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = {x + 1, y,     z,     voxel_id, 2, ao.x, flip_id};
                    uint8_t v1[7] = {x + 1, y + 1, z,     voxel_id, 2, ao.y, flip_id};
                    uint8_t v2[7] = {x + 1, y + 1, z + 1, voxel_id, 2, ao.z, flip_id};
                    uint8_t v3[7] = {x + 1, y,     z + 1, voxel_id, 2, ao.w, flip_id};

                    if (flip_id) {
                        PUSH_FACE(v3, v0, v1, v3, v1, v2);
                    } else {
                        PUSH_FACE(v0, v1, v2, v0, v2, v3);
                    }
                }

                // Left face
                if (isVoid(chunk, x - 1, y, z)) {
                    Vector4 ao = getAO(chunk, PLANE_X, x - 1, y, z);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = {x, y,     z,     voxel_id, 3, ao.x, flip_id};
                    uint8_t v1[7] = {x, y + 1, z,     voxel_id, 3, ao.y, flip_id};
                    uint8_t v2[7] = {x, y + 1, z + 1, voxel_id, 3, ao.z, flip_id};
                    uint8_t v3[7] = {x, y,     z + 1, voxel_id, 3, ao.w, flip_id};

                    if (flip_id) {
                        PUSH_FACE(v3, v1, v0, v3, v2, v1);
                    } else {
                        PUSH_FACE(v0, v2, v1, v0, v3, v2);
                    }
                }

                // Back face
                if (isVoid(chunk, x, y, z - 1)) {
                    Vector4 ao = getAO(chunk, PLANE_Z, x, y, z - 1);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = {x,     y,     z, voxel_id, 4, ao.x, flip_id};
                    uint8_t v1[7] = {x,     y + 1, z, voxel_id, 4, ao.y, flip_id};
                    uint8_t v2[7] = {x + 1, y + 1, z, voxel_id, 4, ao.z, flip_id};
                    uint8_t v3[7] = {x + 1, y,     z, voxel_id, 4, ao.w, flip_id};

                    if (flip_id) {
                        PUSH_FACE(v3, v0, v1, v3, v1, v2);
                    } else {
                        PUSH_FACE(v0, v1, v2, v0, v2, v3);
                    }
                }

                // Front face
                if (isVoid(chunk, x, y, z + 1)) {
                    Vector4 ao = getAO(chunk, PLANE_Z, x, y, z + 1);
                    bool flip_id = ao.y + ao.w > ao.x + ao.z;

                    uint8_t v0[7] = {x,     y,     z + 1, voxel_id, 5, ao.x, flip_id};
                    uint8_t v1[7] = {x,     y + 1, z + 1, voxel_id, 5, ao.y, flip_id};
                    uint8_t v2[7] = {x + 1, y + 1, z + 1, voxel_id, 5, ao.z, flip_id};
                    uint8_t v3[7] = {x + 1, y,     z + 1, voxel_id, 5, ao.w, flip_id};

                    if (flip_id) {
                        PUSH_FACE(v3, v1, v0, v3, v2, v1);
                    } else {
                        PUSH_FACE(v0, v2, v1, v0, v3, v2);
                    }
                }
            }
        }
    }

    if (chunk->vertices) {
        free(chunk->vertices);
    }

    chunk->vertices = vertex_data;
    chunk->vertexCount = arrlen(vertex_data);

    chunk->vao = rlLoadVertexArray();
    assert(chunk->vao > 0);
    chunk->vbo = rlLoadVertexBuffer(chunk->vertices, chunk->vertexCount * chunk->vertexSize, false);
    assert(chunk->vbo > 0);

    rlEnableVertexArray(chunk->vao);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, chunk->vertexSize, (void *)0);
    rlEnableVertexAttribute(0);

    rlDisableVertexArray();
    rlDisableVertexBuffer();
}
