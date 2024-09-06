#include <stdio.h>

#include "common.h"

int main(void)
{
    InitWindow(WIN_RES.x, WIN_RES.y, "Voxel Engine");
    SetTargetFPS(0);

    while (!WindowShouldClose())
    {
        ClearBackground(BACKGROUND_COLOR);

        BeginDrawing();
        {

        }
        EndDrawing();

        char titlebuf[50] = {0};
        snprintf(titlebuf, 50, "%0.0f FPS", 1.0f / GetFrameTime());
        SetWindowTitle(titlebuf);
    }

    CloseWindow();

    return 0;
}
