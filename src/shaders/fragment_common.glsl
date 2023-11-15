#ifndef FRAGMENT_COMMON_INCLUDED
#define FRAGMENT_COMMON_INCLUDED

#include "common.glsl"

vec3 primary_ray_dir() {
    vec2 pos2 = u_viewport.w * ((gl_FragCoord.xy - u_viewport.xy / 2) / u_viewport.y);
    vec4 world_space = u_view_inverse * vec4(pos2, 1, 0);
    return normalize(world_space.xyz);
}

#endif
