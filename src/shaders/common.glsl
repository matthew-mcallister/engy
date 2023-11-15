#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

layout(std140, set = 0, binding = 0) uniform ViewUniforms {
    // Components of u_viewport are
    //  (width, height, tan_fovx, tan_fovy)
    vec4 u_viewport;
    mat4 u_projection;
    mat4 u_view;
    mat4 u_view_inverse;
    mat4 u_instance[512];
};

#ifndef NO_SAMPLERS
layout(set = 1, binding = 0) uniform sampler2D u_samplers[];
#endif

#endif
