#version 330 core

layout (location = 0) in uint x;
layout (location = 1) in uint y;
layout (location = 2) in uint z;
layout (location = 3) in uint voxel_id;
layout (location = 4) in uint face_id;

uniform mat4 mvp;

out vec3 color;

void main() {
    float scale = 1.0; // 0.000000001;
    vec3 in_position = scale * vec3(x, y, z);
    // if (x > 1.) in_position.x = 1;
    // if (y > 1.) in_position.y = 1;
    // if (z > 1.) in_position.z = 1;
    gl_Position = mvp * vec4(in_position, 1.0);
    color = scale * vec3(x, y, z);
}