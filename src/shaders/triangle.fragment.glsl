#version 460 core
#pragma shader_stage fragment
#extension GL_EXT_nonuniform_qualifier : require

#include "common.glsl"

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) flat in uint in_texture;

layout(location = 0) out vec4 out_color;

void main() {
    vec2 viewport = vec2(1024, 768);
    vec4 tex_color = texture(u_samplers[nonuniformEXT(in_texture)], 2 * gl_FragCoord.xy / viewport - 0.5);
    out_color = tex_color;
}