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

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) flat in uint in_texture;
layout (location = 0) out vec4 out_color;

void main() {
    // XXX: Ughhh not sure if non-uniform indexing is supported in OpenGL
    // Might have to do one draw call per material per chunk
    // Fortunately everything is already sorted by material
    //vec4 albedo = texture(u_samplers[in_texture], in_texcoord);
    vec4 albedo = texture(u_samplers[0], in_texcoord);
    float darkness = 0.5 - 0.5 * in_normal.z;
    vec3 color = darkness * albedo.xyz;
    color += (vec3(1) - color) * u_highlight;
    out_color = vec4(color, 1);
}