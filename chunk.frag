#version 330 core

layout (location = 0) out vec4 fragColor;

const vec3 gamma = vec3(2.2);
const vec3 inv_gamma = 1 / gamma;

uniform sampler2D u_texture_0;

in vec3 voxel_color;
in vec2 uv;
in float shading;

flat in int face_id;
flat in int voxel_id;

void main() {
    vec2 face_uv;
    face_uv.x = uv.x / 3.0 - min(2, face_id) / 3.0;
    face_uv.y = uv.y / 8.0 + voxel_id / 8.0;

    vec3 tex_col = texture(u_texture_0, face_uv).rgb;
    tex_col = pow(tex_col, gamma);

    tex_col *= shading;

    tex_col = pow(tex_col, inv_gamma);
    fragColor = vec4(tex_col, 1);
}