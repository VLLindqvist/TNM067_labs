#include "utils/structs.glsl"

uniform sampler2D vfColor;
uniform ImageParameters vfParameters;
in vec3 texCoord_;

float passThrough(vec2 coord) {
    return texture(vfColor,coord).x;
}

float magnitude(vec2 coord) {
    //TASK 1: find the magnitude of the vectorfield at the position coords
    vec2 velo = texture(vfColor, coord.xy).xy; 

    return sqrt(pow(velo.x, 2) + pow(velo.y, 2)); // For 3D: just add the third coord. Magnitude is simply the lenght of the vector
}

vec2 dVdx(vec2 pixelSize, vec2 coord) {
    return (texture(vfColor, vec2(coord.x + pixelSize.x, coord.y)).xy - texture(vfColor, vec2(coord.x - pixelSize.x, coord.y)).xy) / (2 * pixelSize.x);
}

vec2 dVdy(vec2 pixelSize, vec2 coord) {
    return (texture(vfColor, vec2(coord.x, coord.y + pixelSize.y)).xy - texture(vfColor, vec2(coord.x, coord.y - pixelSize.y)).xy) / (2 * pixelSize.y);
}

float divergence(vec2 coord) {
    //TASK 2: find the divergence of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;

    return dVdx(pixelSize, coord).x + dVdy(pixelSize, coord).y;

    // How can rotation be extended to 3D?
    // Some parts of the image are black, why?
}

float rotation(vec2 coord) {
    //TASK 3: find the curl of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;

    return dVdx(pixelSize, coord).y - dVdy(pixelSize, coord).x;
}

void main(void) {
    float v = OUTPUT(texCoord_.xy);
    FragData0 = vec4(v);
}
