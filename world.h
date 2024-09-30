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

int GetChunkIndex(Vector3 worldpos);
int GetVoxelIndex(Vector3 worldpos);

RayCollision RayCast(World *w, Camera *camera);
void RemoveVoxel(World *w, Vector3 voxel);
void AddVoxel(World *w, Vector3 voxel);

#endif // _WORLD_H
