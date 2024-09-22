#ifndef _CHUNK_H
#define _CHUNK_H

#include "common.h"

struct World;

struct Chunk {
    int chunk_index;
    Vector3 position;
    struct World *world;
    uint8_t *voxels;
    uint32_t *vertices;
    int vertexCount;
    int vertexSize;
    int vao;
    int vbo;
};

typedef struct Chunk Chunk;

enum FacePlane {
    PLANE_X,
    PLANE_Y,
    PLANE_Z,
};
typedef enum FacePlane FacePlane;

void InitChunkGL(void);
void CloseChunkGL(void);

Chunk* CreateChunk(struct World *world, Vector3 position, int chunk_index);
void DrawChunk(Chunk *c, Camera *camera);
void DestroyChunk(Chunk *c);

Vector4 getAO(Chunk *chunk, FacePlane plane, int x, int y, int z);
bool isVoid(Chunk *c, int x, int y, int z);
void GenChunkMesh(Chunk *chunk);

#endif // _CHUNK_H
