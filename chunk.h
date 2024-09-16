#include "common.h"

struct Chunk {
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

Chunk* CreateChunk(void);
void DrawChunk(Chunk *c, Camera* camera, Vector3 position);
void DestroyChunk(Chunk *c);

void GenChunkMesh(Chunk *chunk);
