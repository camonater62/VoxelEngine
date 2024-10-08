#version 330 core

layout (location = 0) in uint packed_data;

uniform mat4 mvp;

out vec3 voxel_color;
out vec2 uv;
out float shading;

const float ao_values[4] = float[4](0.1, 0.25, 0.5, 1.0);

const float face_shading[6] = float[6](
    1.0, 0.5, // top bottom
    0.5, 0.8, // right left
    0.5, 0.8  // front back
);

const vec2 uv_coords[4] = vec2[4](
    vec2(0, 0), vec2(0, 1),
    vec2(1, 0), vec2(1, 1)
);

const int uv_indices[24] = int[24](
    1, 0, 2, 1, 2, 3, // tex coords indices for vertices of an even face 
    3, 0, 2, 3, 1, 0, // odd face
    3, 1, 0, 3, 0, 2, // even flipped face
    1, 2, 3, 1, 0, 2  // odd flipped face
);

vec3 hash31(float p) {
    vec3 p3 = fract(vec3(p * 21.2) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xxy + p3.yzz) * p3.zyx) + 0.05f;
}

void main() {
    int x = int((packed_data >> 26) & 63u);
    int y = int((packed_data >> 20) & 63u); 
    int z = int((packed_data >> 14) & 63u);
    int voxel_id = int((packed_data >> 6) & 255u);
    int face_id = int((packed_data >> 3) & 7u);
    int ao_id = int((packed_data >> 1) & 3u);
    int flip_id = int(packed_data & 1u);

    int uv_index = gl_VertexID % 6 + ((face_id & 1) + flip_id * 2) * 6;
    uv = uv_coords[uv_indices[uv_index]];
    voxel_color = hash31(voxel_id);
    shading = face_shading[face_id] * ao_values[ao_id];
    gl_Position = mvp * vec4(x, y, z, 1.0);
}