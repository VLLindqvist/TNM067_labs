#include <modules/tnm067lab2/processors/hydrogengenerator.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/volumeramutils.h>
#include <modules/base/algorithm/dataminmax.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/base/algorithm/dataminmax.h>
#include <math.h>

namespace inviwo {

const ProcessorInfo HydrogenGenerator::processorInfo_{
    "org.inviwo.HydrogenGenerator",  // Class identifier
    "Hydrogen Generator",            // Display name
    "TNM067",                        // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
};

const ProcessorInfo HydrogenGenerator::getProcessorInfo() const { return processorInfo_; }

HydrogenGenerator::HydrogenGenerator()
    : Processor(), volume_("volume"), size_("size_", "Volume Size", 16, 4, 256) {
    addPort(volume_);
    addProperty(size_);
}

void HydrogenGenerator::process() {
    auto vol = std::make_shared<Volume>(size3_t(size_), DataFloat32::get());

    auto ram = vol->getEditableRepresentation<VolumeRAM>();
    auto data = static_cast<float*>(ram->getData());
    util::IndexMapper3D index(ram->getDimensions());

    util::forEachVoxel(*ram, [&](const size3_t& pos) {
        vec3 cartesian = idTOCartesian(pos);
        data[index(pos)] = static_cast<float>(eval(cartesian));
    });

    auto minMax = util::volumeMinMax(ram);
    vol->dataMap_.dataRange = vol->dataMap_.valueRange = dvec2(minMax.first.x, minMax.second.x);

    volume_.setData(vol);
}

vec3 HydrogenGenerator::cartesianToSpherical(vec3 cartesian) {
    // TASK 1: implement conversion using the equations in the lab script
    const float r = sqrt((cartesian.x * cartesian.x) + (cartesian.y * cartesian.y) + (cartesian.z * cartesian.z));
     
    if (r < std::numeric_limits<float>::epsilon()) return vec3{0.0, 0.0, 0.0}; // if in origo in the cartesian system

    const float theta = glm::acos(cartesian.z / r);
    const float phi = glm::atan(cartesian.y, cartesian.x);

    return vec3{r, theta, phi};
}

double HydrogenGenerator::eval(vec3 cartesian) {
    const vec3 sph = cartesianToSpherical(cartesian); // sph.x = r, sph.y = theta, sph.z = phi

    /* TASK 2: Evaluate wave function */
    // Z = 1, a0 = 1
    const float psi1 = 1 / (81 * sqrt(6 * M_PI));
    const float psi2 = 1; 
    const float psi3 = pow(sph.x, 2);
    const float psi4 = exp((-sph.x) / 3);
    const float psi5 = (3 * pow(cos(sph.y), 2)) - 1;

    const float psiTot = psi1 * psi2 * psi3 * psi4 * psi5;
    
    return pow(abs(psiTot), 2);
}

vec3 HydrogenGenerator::idTOCartesian(size3_t pos) {
    vec3 p(pos);
    p /= size_ - 1;
    return p * (36.0f) - 18.0f;
}

}  // namespace inviwo
