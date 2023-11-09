#version 460 core
#pragma shader_stage vertex

layout(binding = 0) uniform Uniforms {
    vec4 u_color;
};

layout(location = 0) in vec3 in_pos;

void main() {
    gl_Position = vec4(in_pos, 1);
}
