#version 460 core
#pragma shader_stage fragment

#include "common.glsl"

layout (location = 0) out vec4 out_color;

void main() {
    vec2 viewport = vec2(1024, 768);
    vec4 tex_color = texture(u_samplers[0], 2 * gl_FragCoord.xy / viewport - 0.5);
    out_color = tex_color;
}