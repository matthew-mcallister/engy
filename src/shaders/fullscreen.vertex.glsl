#version 460 core
#pragma shader_stage vertex

#define NO_SAMPLERS
#include "common.glsl"

const vec2 vertices[] = {
    vec2(-1, -1),
    vec2(-1,  1),
    vec2( 1, -1),
    vec2( 1,  1),
};

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
}
