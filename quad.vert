#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 3) in vec3 in_color;

out vec3 color;

uniform mat4 mvp;

void main() {
    color = in_color;
    gl_Position = mvp*vec4(in_pos, 1.0);
}