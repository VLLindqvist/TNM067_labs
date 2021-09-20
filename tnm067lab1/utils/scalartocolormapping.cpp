#include <modules/tnm067lab1/utils/scalartocolormapping.h>

namespace inviwo {

void ScalarToColorMapping::clearColors() { baseColors_.clear(); }
void ScalarToColorMapping::addBaseColors(vec4 color) { baseColors_.push_back(color); }

vec4 ScalarToColorMapping::sample(float t) const {
    if (baseColors_.size() == 0) return vec4(t);
    if (baseColors_.size() == 1) return vec4(baseColors_[0]);
    if (t <= 0) return vec4(baseColors_.front());
    if (t >= 1) return vec4(baseColors_.back());

    // TODO: use t to select which two base colors to interpolate in-between
    float intervalSize = 1.0f / (baseColors_.size() - 1.0f);
    size_t firstIdx = std::floorf(t / intervalSize);

    vec4 firstColor = baseColors_[firstIdx];
    vec4 secondColor = baseColors_[firstIdx + 1];

    // TODO: Interpolate colors in baseColors_ and set dummy color to result
    float intervalWeight = (t / intervalSize);
    intervalWeight = intervalWeight - std::floorf(intervalWeight);
    return (firstColor * (1.0f - intervalWeight)) + (secondColor * intervalWeight);
}

}  // namespace inviwo
