#version 460 core
#pragma shader_stage fragment

#extension GL_EXT_nonuniform_qualifier : require

#include "common.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) flat in uint in_texture;

layout (location = 0) out vec4 out_color;

void main() {
    vec4 albedo = texture(u_samplers[in_texture], in_texcoord);
    float darkness = 0.5 - 0.5 * in_normal.z;
    vec3 color = darkness * albedo.xyz;
    out_color = vec4(color, 1);
}
