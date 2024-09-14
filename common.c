#include "common.h"

const Vector2 WIN_RES = {1600, 900};
const Color BG_COLOR = {
    .r = (uint8_t) (255 * 0.10f),
    .g = (uint8_t) (255 * 0.16f),
    .b = (uint8_t) (255 * 0.25f),
    .a = (uint8_t) (255 * 1.00f),
};
const int CHUNK_SIZE = 32;
const int H_CHUNK_SIZE = CHUNK_SIZE / 2;
const int CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
const int CHUNK_VOLUME = CHUNK_AREA * CHUNK_SIZE;
