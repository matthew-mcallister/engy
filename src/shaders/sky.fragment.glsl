#version 460 core
#pragma shader_stage fragment

#define SHADER_STAGE_FRAGMENT
#define NO_SAMPLERS
#include "common.glsl"

layout(location = 0) out vec4 out_color;

// Spatial coefficient to Rayleigh scattering. In the plane wave
// regime, this is independent of angle and wavelength and can be
// set empirically.
float rayleigh_coefficient = 1e6;
// Approximate wavelengths of RGB color components (times 100nm)
vec3 wavelength = vec3(6.10, 5.55, 4.65);
vec3 wavelength3 = wavelength * wavelength * wavelength;
vec3 wavelength4 = wavelength * wavelength * wavelength * wavelength;
// Blackbody power spectrum at 5900K (approx temperature of sun's surface)
// Coefficient is rolled into the Rayleigh coeff
float temperature = 5900;
float hc_over_kt = 1.439e5 / temperature; // In units of 100nm
vec3 blackbody_intensity = (1 / wavelength3) * 1 / (exp(hc_over_kt / wavelength) - 1);

// Tonemapping/postprocess
float exposure = 1;
float gamma = 2.2;

void main() {
    // Rayleigh scattering
    // TODO: Sunsets (light loss due to scattering)
    // TODO: Mie scattering?
    vec3 primary = primary_ray_dir();
    float z = primary.z;
    float s = dot(primary, u_sun_dir.xyz);
    vec3 intensity = rayleigh_coefficient * (blackbody_intensity / wavelength4) * (1 + s * s) / z;
    // Convert from HDR to linear color
    vec3 color = vec3(1) - exp(-gamma * exposure * intensity);
    out_color = vec4(color, 1);
}