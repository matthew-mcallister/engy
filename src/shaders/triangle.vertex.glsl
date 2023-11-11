#version 460 core
#pragma shader_stage vertex

#include "common.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in uint in_texture;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_texcoord;
layout(location = 2) flat out uint out_texture;

void main() {
    gl_Position = vec4(in_pos, 1);
    out_normal = in_normal;
    out_texcoord = in_texcoord;
    out_texture = in_texture;
}
