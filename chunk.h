#ifndef _CHUNK_H
#define _CHUNK_H

#include "common.h"

struct World;

struct Chunk {
    int chunk_index;
    Vector3 position;
    struct World *world;
    uint8_t *voxels;
    uint8_t *vertices;
    int vertexCount;
    int vertexSize;
    int vao;
    int vbo;
};

typedef struct Chunk Chunk;

void InitChunkGL(void);
void CloseChunkGL(void);

Chunk* CreateChunk(struct World *world, Vector3 position, int chunk_index);
void DrawChunk(Chunk *c, Camera *camera);
void DestroyChunk(Chunk *c);

void GenChunkMesh(Chunk *chunk);

#endif // _CHUNK_H
