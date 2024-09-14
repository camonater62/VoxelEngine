#include "chunk.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

Chunk* CreateChunk(void) {
    Chunk* c = malloc(sizeof(Chunk));
    c->voxels = calloc(CHUNK_VOLUME, sizeof(uint8_t));
    c->vertices = NULL;
    c->vertexCount = 0;
    c->vertexSize = 0;

    // Temporarily give all voxels a value of 1
    memset(c->voxels, 1, CHUNK_VOLUME);

    GenChunkMesh(c);

    return c;
}

void DestroyChunk(Chunk* c) {
    if (c) {
        if (c->voxels) {
            free(c->voxels);
        }
        if (c->vertices) {
            free(c->vertices);
        }
        free(c);
    }
}

bool isVoid(Chunk *c, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE 
     || y < 0 || y >= CHUNK_SIZE 
     || z < 0 || z >= CHUNK_SIZE) {
        return true;
    }

    return (c->voxels[x + CHUNK_SIZE * y + CHUNK_SIZE * z] == 0);
}

void GenChunkMesh(Chunk *chunk)
{
    // 5 vertex attributes:
    // 1) x
    // 2) y
    // 3) z
    // 4) voxel_id
    // 5) face_id
    chunk->vertexSize = 5;
    uint8_t* vertex_data = (uint8_t*) calloc(CHUNK_VOLUME * 18 * chunk->vertexSize, sizeof(uint8_t));
    int index = 0;

    #define PUSH_VERTEX(vertex) { \
        memcpy(vertex_data + index, vertex, sizeof(vertex)); \
        index += sizeof(vertex); \
    }

    #define PUSH_FACE(a, b, c, d, e, f) { \
        PUSH_VERTEX(a); \
        PUSH_VERTEX(b); \
        PUSH_VERTEX(c); \
        PUSH_VERTEX(d); \
        PUSH_VERTEX(e); \
        PUSH_VERTEX(f); \
    }

    for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
        for (uint8_t y = 0; y < CHUNK_SIZE; y++) {
            for (uint8_t z = 0; z < CHUNK_SIZE; z++) {
                uint8_t voxel_id = chunk->voxels[x + CHUNK_SIZE * z + CHUNK_AREA * y];
                if (voxel_id == 0) {
                    continue;
                }

                // Top face
                if (isVoid(chunk, x, y + 1, z)) {
                    uint8_t v0[5] = {x, y + 1, z, voxel_id, 0};
                    uint8_t v1[5] = { x + 1, y + 1, z, voxel_id, 0};
                    uint8_t v2[5] = { x + 1, y + 1, z + 1, voxel_id, 0};
                    uint8_t v3[5] = { x, y + 1, z + 1, voxel_id, 0};

                    PUSH_FACE(v0, v3, v2, v0, v2, v1);
                }

                // Bottom face
                if (isVoid(chunk, x, y - 1, z)) {
                    uint8_t v0[5] = {x, y, z, voxel_id, 1}; 
                    uint8_t v1[5] = {x + 1, y, z, voxel_id, 1}; 
                    uint8_t v2[5] = {x + 1, y, z + 1, voxel_id, 1}; 
                    uint8_t v3[5] = {x, y, z + 1, voxel_id, 1}; 
                    
                    PUSH_FACE(v0, v2, v3, v0, v1, v2);                    
                }

                // Right face
                if (isVoid(chunk, x + 1, y, z)) {
                    uint8_t v0[5] = {x + 1, y, z, voxel_id, 2};
                    uint8_t v1[5] = {x + 1, y + 1, z, voxel_id, 2};
                    uint8_t v2[5] = {x + 1, y + 1, z + 1, voxel_id, 2};
                    uint8_t v3[5] = {x + 1, y, z + 1, voxel_id, 2};

                    PUSH_FACE(v0, v1, v2, v0, v2, v3);
                }

                // Left face
                if (isVoid(chunk, x - 1, y, z)) {
                    uint8_t v0[5] = {x, y, z, voxel_id, 3};
                    uint8_t v1[5] = {x, y + 1, z, voxel_id, 3};
                    uint8_t v2[5] = {x, y + 1, z + 1, voxel_id, 3};
                    uint8_t v3[5] = {x, y, z + 1, voxel_id, 3};

                    PUSH_FACE(v0, v2, v1, v0, v3, v2);
                }

                // Back face
                if (isVoid(chunk, x, y, z - 1)) {
                    uint8_t v0[5] = {x    , y    , z, voxel_id, 4};
                    uint8_t v1[5] = {x    , y + 1, z, voxel_id, 4};
                    uint8_t v2[5] = {x + 1, y + 1, z, voxel_id, 4};
                    uint8_t v3[5] = {x + 1, y    , z, voxel_id, 4};

                    PUSH_FACE(v0, v1, v2, v0, v2, v3);
                }

                // Front face
                if (isVoid(chunk, x, y, z + 1)) {
                    uint8_t v0[5] = {x    , y    , z + 1, voxel_id, 5};
                    uint8_t v1[5] = {x    , y + 1, z + 1, voxel_id, 5};
                    uint8_t v2[5] = {x + 1, y + 1, z + 1, voxel_id, 5};
                    uint8_t v3[5] = {x + 1, y    , z + 1, voxel_id, 5};

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

    printf("Used %i / %i\n", index, CHUNK_VOLUME * 18 * 5);
}
