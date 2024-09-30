#include "world.h"

#include "chunk.h"
#include <rcamera.h>
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

RayCollision RayCast(World *w, Camera *camera) {
    RayCollision result;
    result.hit = false;
    result.distance = 0;
    result.normal = Vector3Zero();
    result.point = Vector3Zero();

    #define SIGN(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
    #define FRAC0(x) (x - floorf(x))
    #define FRAC1(x) (1 - x + floorf(x))

    float tMaxX, tMaxY, tMaxZ, tDeltaX, tDeltaY, tDeltaZ;
    Vector3 voxel;
    int step_dir;

    Vector3 camForward = GetCameraForward(camera);
    Vector3 ray = Vector3Scale(camForward, MAX_RAY_DIST);
    Vector3 start = camera->position;
    Vector3 end = Vector3Add(start, ray);

    float x1 = start.x;
    float y1 = start.y;
    float z1 = start.z;

    float x2 = end.x;
    float y2 = end.y;
    float z2 = end.z;

    int dx = SIGN(x2 - x1);
    if (dx != 0) tDeltaX = fmin(dx / (x2 - x1), 10000000.0f); else tDeltaX = 10000000.0f;
    if (dx > 0) tMaxX = tDeltaX * FRAC1(x1); else tMaxX = tDeltaX * FRAC0(x1);
    voxel.x = (int) x1;

    int dy = SIGN(y2 - y1);
    if (dy != 0) tDeltaY = fmin(dy / (y2 - y1), 10000000.0f); else tDeltaY = 10000000.0f;
    if (dy > 0) tMaxY = tDeltaY * FRAC1(y1); else tMaxY = tDeltaY * FRAC0(y1);
    voxel.y = (int) y1;

    int dz = SIGN(z2 - z1);
    if (dz != 0) tDeltaZ = fmin(dz / (z2 - z1), 10000000.0f); else tDeltaZ = 10000000.0f;
    if (dz > 0) tMaxZ = tDeltaZ * FRAC1(z1); else tMaxZ = tDeltaZ * FRAC0(z1);
    voxel.z = (int) z1;

    while (!(tMaxX > 1 && tMaxY > 1 && tMaxZ > 1)) {
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                voxel.x += dx;
                tMaxX += tDeltaX;
                step_dir = 0;
            } else {
                voxel.z += dz;
                tMaxZ += tDeltaZ;
                step_dir = 2;
            }
        } else {
            if (tMaxY < tMaxZ) {
                voxel.y += dy;
                tMaxY += tDeltaY;
                step_dir = 1;
            } else {
                voxel.z += dz;
                tMaxZ += tDeltaZ;
                step_dir = 2;
            }
        }

        // Check collision
        if (voxel.x >= 0 && voxel.x < WORLD_W * CHUNK_SIZE
         && voxel.y >= 0 && voxel.y < WORLD_H * CHUNK_SIZE
         && voxel.z >= 0 && voxel.z < WORLD_D * CHUNK_SIZE) 
        {
            int chunk_index = GetChunkIndex(voxel);
            int voxel_index = GetVoxelIndex(voxel);

            int voxel_id = w->chunks[chunk_index]->voxels[voxel_index];

            if (voxel_id != 0) {
                result.hit = true;
                result.distance = Vector3Distance(voxel, camera->position);
                result.point = voxel;
                result.normal = Vector3Zero();

                // printf("%i, %i, %i\n", (int)voxel.x, (int)voxel.y, (int)voxel.z);

                switch (step_dir)
                {
                default:
                case 0:
                    result.normal.x = -dx;
                    break;
                
                case 1:
                    result.normal.y = -dy;
                    break;
                
                case 2:
                    result.normal.z = -dz;
                    break;
                }


                break;
            }
         }
    }

    return result;
}

int GetChunkIndex(Vector3 worldpos) {
    int cx = (int)(worldpos.x / CHUNK_SIZE);
    int cy = (int)(worldpos.y / CHUNK_SIZE);
    int cz = (int)(worldpos.z / CHUNK_SIZE);
    int chunk_index = cx + WORLD_W * cz + WORLD_AREA * cy;
    return chunk_index;
}

int GetVoxelIndex(Vector3 worldpos) {
    int vx = (int)(worldpos.x) % CHUNK_SIZE;
    int vy = (int)(worldpos.y) % CHUNK_SIZE;
    int vz = (int)(worldpos.z) % CHUNK_SIZE;
    int voxel_index = vx + CHUNK_SIZE * vz + CHUNK_AREA * vy;
    return voxel_index;
}

void RemoveVoxel(World *w, Vector3 voxel)
{
    if (voxel.x < 0 || voxel.x >= WORLD_W * CHUNK_SIZE 
     || voxel.y < 0 || voxel.y >= WORLD_H * CHUNK_SIZE
     || voxel.z < 0 || voxel.z >= WORLD_D * CHUNK_SIZE) {
        return;
    }

    int chunk_index = GetChunkIndex(voxel);

    int vx = (int)(voxel.x) % CHUNK_SIZE;
    int vy = (int)(voxel.y) % CHUNK_SIZE;
    int vz = (int)(voxel.z) % CHUNK_SIZE;
    int voxel_index = vx + CHUNK_SIZE * vz + CHUNK_AREA * vy; 

    if (w->chunks[chunk_index]->voxels[voxel_index] != 0) {
        w->chunks[chunk_index]->voxels[voxel_index] = 0;
        GenChunkMesh(w->chunks[chunk_index]);

        if (vx == 0 && voxel.x >= CHUNK_SIZE) {
            Vector3 offset = {-1, 0, 0};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }
        if (vx == CHUNK_SIZE - 1 && voxel.x < CHUNK_SIZE * WORLD_W) {
            Vector3 offset = {1, 0, 0};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }

        if (vy == 0 && voxel.y >= CHUNK_SIZE) {
            Vector3 offset = {0, -1, 0};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }
        if (vy == CHUNK_SIZE - 1  && voxel.y < CHUNK_SIZE * WORLD_H) {
            Vector3 offset = {0, 1, 0};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }

        if (vz == 0 && voxel.z >= CHUNK_SIZE) {
            Vector3 offset = {0, 0, -1};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }
        if (vz == CHUNK_SIZE - 1  && voxel.z < CHUNK_SIZE * WORLD_D) {
            Vector3 offset = {0, 0, 1};
            Vector3 adjPos = Vector3Add(voxel, offset);
            int adjChunk = GetChunkIndex(adjPos);
            if (adjChunk != chunk_index) {
                GenChunkMesh(w->chunks[adjChunk]);
            }
        }
    }

}

void AddVoxel(World *w, Vector3 voxel)
{
    if (voxel.x < 0 || voxel.x >= WORLD_W * CHUNK_SIZE 
     || voxel.y < 0 || voxel.y >= WORLD_H * CHUNK_SIZE
     || voxel.z < 0 || voxel.z >= WORLD_D * CHUNK_SIZE) {
        return;
    }

    int chunk_index = GetChunkIndex(voxel);
    int voxel_index = GetVoxelIndex(voxel);

    if (w->chunks[chunk_index]->voxels[voxel_index] == 0) {
        w->chunks[chunk_index]->voxels[voxel_index] = 1;
        GenChunkMesh(w->chunks[chunk_index]);
    }
}
