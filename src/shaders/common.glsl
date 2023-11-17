#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

layout(std140, set = 0, binding = 0) uniform ViewUniforms {
    // Components of u_viewport are
    //  (width, height, tan_fovx, tan_fovy)
    vec4 u_viewport;
    vec4 u_sun_dir;
    mat4 u_projection;
    mat4 u_view;
    mat4 u_view_inverse;
    mat4 u_instance[512];
};

#ifndef NO_SAMPLERS
layout(set = 1, binding = 0) uniform sampler2D u_samplers[];
#endif

#ifdef SHADER_STAGE_FRAGMENT
vec3 primary_ray_dir() {
    vec2 ndc = 2 * gl_FragCoord.xy / u_viewport.xy - vec2(1);
    vec4 world_space = u_view_inverse * vec4(u_viewport.zw * ndc, 1, 0);
    return normalize(world_space.xyz);
}
#endif

#endif
