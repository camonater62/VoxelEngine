#include "world.h"

#include "chunk.h"

#include <stdlib.h>

World *CreateWorld(void)
{
    World *w = malloc(sizeof(World));
    assert(w != NULL);
    w->chunks = calloc(WORLD_VOL, sizeof(Chunk*));
    assert(w->chunks != NULL);
    w->voxels = calloc(WORLD_VOL, sizeof(uint8_t*));
    assert(w->voxels != NULL);

    BuildChunks(w);
    GenWorldMeshes(w);

    return w;
}

void BuildChunks(World *w) {
    for (int x = 0; x < WORLD_W; x++) {
        for (int y = 0; y < WORLD_H; y++) {
            for (int z = 0; z < WORLD_D; z++) {
                int chunk_index = x + WORLD_W * z + WORLD_AREA * y;
                Vector3 position = {
                    .x = x * CHUNK_SIZE,
                    .y = y * CHUNK_SIZE,
                    .z = z * CHUNK_SIZE,
                };
                Chunk *chunk = CreateChunk(w, position, chunk_index);
                assert(chunk != NULL);
                w->chunks[chunk_index] = chunk;
            }
        } 
    }
}

void GenWorldMeshes(World *w) {
    for (int x = 0; x < WORLD_W; x++) {
        for (int y = 0; y < WORLD_H; y++) {
            for (int z = 0; z < WORLD_D; z++) {
                int chunk_index = x + WORLD_W * z + WORLD_AREA * y;
                GenChunkMesh(w->chunks[chunk_index]);
            }
        } 
    }
}

void DrawWorld(World *w, Camera *camera) {
    for (int x = 0; x < WORLD_W; x++) {
        for (int y = 0; y < WORLD_H; y++) {
            for (int z = 0; z < WORLD_D; z++) {
                int chunk_index = x + WORLD_W * z + WORLD_AREA * y;
                Chunk *chunk = w->chunks[chunk_index];
                DrawChunk(chunk, camera);
            }
        } 
    }
}

void DestroyWorld(World *w) {
    if (w) {
        if (w->chunks) {
            for (int i = 0; i < WORLD_VOL; i++) {
                DestroyChunk(w->chunks[i]);
            }
        }
        if (w->voxels) {
            free(w->voxels);
        }
        free(w);
    }
}
