#include "common.h"

struct Chunk {
    uint8_t *voxels;
    uint8_t *vertices;
    int vertexCount;
    int vertexSize;
};

typedef struct Chunk Chunk;

Chunk* CreateChunk(void);
void DestroyChunk(Chunk *c);

void GenChunkMesh(Chunk *chunk);
