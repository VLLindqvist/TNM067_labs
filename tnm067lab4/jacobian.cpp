#include <modules/tnm067lab4/jacobian.h>

namespace inviwo {

namespace util {

mat2 jacobian(const ImageSampler& sampler, vec2 position, vec2 offset) {
    vec2 ox{offset.x, 0}; // delta x
    vec2 oy{0, offset.y}; // delta y

    // TASK 1: Calculate the Jacobian J
    vec2 dVdx = (sampler.sample(position + ox) - sampler.sample(position - ox)) / (2.0 * ox.x);
    vec2 dVdy = (sampler.sample(position + oy) - sampler.sample(position - oy)) / (2.0 * oy.y);

    mat2 J{dVdx, dVdy};

    return J;
}

}  // namespace util

}  // namespace inviwo
