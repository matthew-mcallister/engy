#version 460 core
#pragma shader_stage fragment

#define NO_SAMPLERS
#include "fragment_common.glsl"

layout(location = 0) out vec4 out_color;

// N.B. assume red = 610nm, green = 555nm, blue = 465nm

void main() {
    // TODO: Rayleigh scattering
    vec3 primary = primary_ray_dir();
    float z = primary.z;
    vec3 col = vec3(0.5 * (1 + sign(primary.z)));
    out_color = vec4(col, 1);
}