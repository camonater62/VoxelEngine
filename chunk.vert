#version 330 core

layout (location = 0) in uvec3 in_position;
layout (location = 1) in uint voxel_id;
layout (location = 2) in uint face_id;

uniform mat4 mvp;

out vec3 color;

void main() {
    gl_Position = mvp * vec4(in_position, 1.0);
    color = vec3(1);
}