#version 460 core

layout (std140, binding = 0) uniform ViewUniforms {
    mat4 u_projection;
    mat4 u_view;
    vec3 u_highlight;
    mat4 u_instance[512];
};

layout (location = 0) in vec3 in_normal;
layout (location = 0) out vec4 out_color;

void main() {
    vec3 color = vec3(0.3 - 0.5 * in_normal.z);
    color += (vec3(1) - color) * u_highlight;
    out_color = vec4(color, 1);
}