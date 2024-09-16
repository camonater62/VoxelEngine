#ifndef _WORLD_H
#define _WORLD_H

#include "common.h"

struct Chunk;

struct World {
    struct Chunk **chunks;
    uint8_t **voxels;
};

typedef struct World World;

World *CreateWorld(void);
void BuildChunks(World *w);
void GenWorldMeshes(World *w);
void DrawWorld(World *w, Camera *camera);
void DestroyWorld(World *w);

#endif // _WORLD_H
