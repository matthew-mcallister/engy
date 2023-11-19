#version 460 core
#pragma shader_stage fragment

#define SHADER_STAGE_FRAGMENT
#define NO_SAMPLERS

#include "color.glsl"
#include "common.glsl"

layout(location = 0) out vec4 out_color;

// Spatial coefficient to Rayleigh scattering. In the plane wave
// regime, this is independent of angle and wavelength and can be
// set empirically.
float rayleigh_coefficient = 2e6;
// Approximate wavelengths of RGB color components (times 100nm)
vec3 wavelength = RGB_WAVELENGTHS;
vec3 wavelength3 = wavelength * wavelength * wavelength;
vec3 wavelength4 = wavelength * wavelength * wavelength * wavelength;
// Blackbody power spectrum at 5900K (approx temperature of sun's surface)
// Coefficient is rolled into the Rayleigh coeff
float temperature = 5900;
float hc_over_kt = 1.439e5 / temperature; // In units of 100nm
vec3 blackbody_intensity = (1 / wavelength3) * 1 / (exp(hc_over_kt / wavelength) - 1);

void main() {
    // Rayleigh scattering
    // TODO: Sunsets (outscattering)
    vec3 primary = primary_ray_dir();
    float z = primary.z;
    float s = dot(primary, u_sun_dir.xyz);
    vec3 intensity = rayleigh_coefficient * (blackbody_intensity / wavelength4) * (1 + s * s) / z;
    vec3 hdr = XYZ_to_rgb(intensity_to_XYZ(intensity));
    vec3 color = tonemap(hdr, 1);
    out_color = vec4(color.zyx, 1);
}