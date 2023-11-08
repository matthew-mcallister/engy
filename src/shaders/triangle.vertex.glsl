#version 460 core
#pragma shader_stage vertex

layout(binding = 0) uniform Uniforms {
    vec4 u_color;
};

vec2 VERTICES[6] = {
    vec2(-0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(0.5, -0.5)
};

void main() {
    gl_Position = vec4(VERTICES[gl_VertexIndex], 0, 1);
}
