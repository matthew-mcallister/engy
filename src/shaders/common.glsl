layout(set = 0, binding = 0) uniform sampler2D u_samplers[];

layout(set = 1, binding = 0) uniform ViewUniforms {
    mat4 u_projection;
    mat4 u_view;
    mat4 u_instance[512];
};
