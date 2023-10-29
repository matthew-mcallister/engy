#version 460 core

layout (std140, binding = 0) uniform ViewUniforms {
    mat4 u_projection;
    mat4 u_view;
    vec3 u_highlight;
    mat4 u_instance[512];
};

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 0) out vec3 out_normal;

void main() {
    mat4 instance = u_instance[gl_BaseInstance + gl_InstanceID];
    vec4 pos = vec4(in_pos, 1);
    gl_Position = u_projection * u_view * instance * pos;
    out_normal = (u_view * instance * vec4(in_normal, 0)).xyz;
}