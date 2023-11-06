#version 460 core
#extension GL_ARB_bindless_texture : enable
#extension GL_NV_uniform_buffer_std430_layout : enable

layout (bindless_sampler) uniform;

layout (binding = 0, std430) uniform ViewUniforms {
    mat4 u_projection;
    mat4 u_view;
    vec3 u_highlight;
    mat4 u_instance[512];
};
layout (binding = 1, std430) uniform TextureUniforms {
    sampler2D u_samplers[4096];
};

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;
layout (location = 3) in uint in_texture;
layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_texcoord;
layout (location = 2) flat out uint out_texture;

void main() {
    mat4 instance = u_instance[gl_BaseInstance + gl_InstanceID];
    vec4 pos = vec4(in_pos, 1);
    gl_Position = u_projection * u_view * instance * pos;
    out_normal = (u_view * instance * vec4(in_normal, 0)).xyz;
    out_texcoord = in_texcoord;
    out_texture = in_texture;
}