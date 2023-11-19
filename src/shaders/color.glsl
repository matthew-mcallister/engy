// Units are 100nm
vec3 RGB_WAVELENGTHS = vec3(5.95, 5.57, 4.83);

vec3 intensity_to_XYZ(vec3 intensity) {
    // Tristimulus data pulled from the official CIE table
    mat3 cie = mat3(
        1.1343,   0.720353, 0,
        0.651901, 1,        0.000407,
        0.055694, 0.279645, 0.64553
    );
    return cie * intensity;
}

vec3 XYZ_to_xyY(vec3 XYZ) {
    float norm = XYZ.x + XYZ.y + XYZ.z;
    return vec3(XYZ.xy / norm, XYZ.y);
}

vec3 XYZ_to_rgb(vec3 XYZ) {
    mat3 to_rgb = mat3(
         2.3646, -0.8965, -0.4680,
        -0.5152, 1.4264, 0.0887,
        0.0052, 0.0144, 1.0092
    );
    return to_rgb * XYZ;
}

vec3 tonemap(vec3 hdr, float exposure) {
    vec3 exposed = exposure * hdr;
    return exposed / (1 + exposed);
}

const float GAMMA = 2.4;

vec3 srgb_to_rgb(vec3 srgb) {
    // For each component c, formula is
    //   1.055 * pow(c, 1 / 2.4) - 0.055    c > 0.0031308
    //   12.92 * c                          c > 0.0031308
    vec3 cmp = vec3(greaterThan(srgb, vec3(0.0031308)));
    vec3 linear = 12.92 * srgb;
    vec3 nonlinear = 1.055 * pow(srgb, vec3(1 / GAMMA)) - 0.055;
    return cmp * nonlinear + (1 - cmp) * linear;
}

vec3 rgb_to_srgb(vec3 srgb) {
    // TODO
    return vec3(0);
}
