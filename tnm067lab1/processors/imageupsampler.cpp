#include <inviwo/core/util/logcentral.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/tnm067lab1/processors/imageupsampler.h>
#include <modules/tnm067lab1/utils/interpolationmethods.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/imageramutils.h>

namespace inviwo {

namespace detail {

template <typename T>
void upsample(ImageUpsampler::IntepolationMethod method, const LayerRAMPrecision<T>& inputImage,
              LayerRAMPrecision<T>& outputImage) {
    using F = typename float_type<T>::type;

    const size2_t inputSize = inputImage.getDimensions();
    const size2_t outputSize = outputImage.getDimensions();

    const T* inPixels = inputImage.getDataTyped();
    T* outPixels = outputImage.getDataTyped();

    auto inIndex = [&inputSize](auto pos) -> size_t {
        pos = glm::clamp(pos, decltype(pos)(0), decltype(pos)(inputSize - size2_t(1)));
        return pos.x + pos.y * inputSize.x;
    };
    auto outIndex = [&outputSize](auto pos) -> size_t {
        pos = glm::clamp(pos, decltype(pos)(0), decltype(pos)(outputSize - size2_t(1)));
        return pos.x + pos.y * outputSize.x;
    };

    util::forEachPixel(outputImage, [&](ivec2 outImageCoords) {
        // outImageCoords: Exact pixel coordinates in the output image currently writing to
        // inImageCoords: Relative coordinates of outImageCoords in the input image, might be
        // between pixels
        dvec2 inImageCoords =
            ImageUpsampler::convertCoordinate(outImageCoords, inputSize, outputSize);

        T finalColor(0);

        finalColor = inPixels[inIndex(outImageCoords)];

        switch (method) {
            case ImageUpsampler::IntepolationMethod::PiecewiseConstant: {
                // Task 6
                // Update finalColor
                ivec2 roundedInImageCoords { std::round(inImageCoords[0]), std::round(inImageCoords[1]) };

                finalColor = inPixels[inIndex(roundedInImageCoords)];
                break;
            }
            case ImageUpsampler::IntepolationMethod::Bilinear: {
                // Update finalColor
                ivec2 pixel0Idx {std::floor(inImageCoords.x), std::floor(inImageCoords.y) };
                ivec2 pixel1Idx {std::ceil(inImageCoords.x), std::floor(inImageCoords.y) };
                ivec2 pixel2Idx {std::floor(inImageCoords.x), std::ceil(inImageCoords.y) };
                ivec2 pixel3Idx {std::ceil(inImageCoords.x), std::ceil(inImageCoords.y) };

                std::array<T, 4> inArray {
                    inPixels[inIndex(pixel0Idx)],
                    inPixels[inIndex(pixel1Idx)],
                    inPixels[inIndex(pixel2Idx)],
                    inPixels[inIndex(pixel3Idx)]
                };

                double x = inImageCoords.x - std::floor(inImageCoords.x);
                double y = inImageCoords.y - std::floor(inImageCoords.y);

                finalColor = TNM067::Interpolation::bilinear(inArray, x, y);
                break;
            }
            case ImageUpsampler::IntepolationMethod::Biquadratic: {
                ivec2 pixel0Idx {std::floor(inImageCoords.x), std::floor(inImageCoords.y)};
                ivec2 pixel1Idx { pixel0Idx.x + 1, pixel0Idx.y };
                ivec2 pixel2Idx { pixel0Idx.x + 2, pixel0Idx.y };
                ivec2 pixel3Idx { pixel0Idx.x, pixel0Idx.y + 1 };
                ivec2 pixel4Idx { pixel0Idx.x + 1, pixel0Idx.y + 1 };
                ivec2 pixel5Idx { pixel0Idx.x + 2, pixel0Idx.y + 1 };
                ivec2 pixel6Idx { pixel0Idx.x, pixel0Idx.y + 2 };
                ivec2 pixel7Idx { pixel0Idx.x + 1, pixel0Idx.y + 2 };
                ivec2 pixel8Idx { pixel0Idx.x + 2, pixel0Idx.y + 2 };

                std::array<T, 9> inArray {
                    inPixels[inIndex(pixel0Idx)],
                    inPixels[inIndex(pixel1Idx)],
                    inPixels[inIndex(pixel2Idx)],
                    inPixels[inIndex(pixel3Idx)],
                    inPixels[inIndex(pixel4Idx)],
                    inPixels[inIndex(pixel5Idx)],
                    inPixels[inIndex(pixel6Idx)],
                    inPixels[inIndex(pixel7Idx)],
                    inPixels[inIndex(pixel8Idx)]
                };
                
                double x = (inImageCoords.x - std::floor(inImageCoords.x)) / 2;
                double y = (inImageCoords.y - std::floor(inImageCoords.y)) / 2;

                finalColor = TNM067::Interpolation::biQuadratic(inArray, x, y);
                break;
            }
            case ImageUpsampler::IntepolationMethod::Barycentric: {
                // Update finalColor
                ivec2 pixel0Idx {std::floor(inImageCoords.x), std::floor(inImageCoords.y) };
                ivec2 pixel1Idx {std::ceil(inImageCoords.x), std::floor(inImageCoords.y) };
                ivec2 pixel2Idx {std::floor(inImageCoords.x), std::ceil(inImageCoords.y) };
                ivec2 pixel3Idx {std::ceil(inImageCoords.x), std::ceil(inImageCoords.y) };

                std::array<T, 4> inArray {
                    inPixels[inIndex(pixel0Idx)],
                    inPixels[inIndex(pixel1Idx)],
                    inPixels[inIndex(pixel2Idx)],
                    inPixels[inIndex(pixel3Idx)]
                };

                double x = inImageCoords.x - std::floor(inImageCoords.x);
                double y = inImageCoords.y - std::floor(inImageCoords.y);

                finalColor = TNM067::Interpolation::barycentric(inArray, x, y);
                break;
            }
            default:
                break;
        }

        outPixels[outIndex(outImageCoords)] = finalColor;
    });
}

}  // namespace detail

const ProcessorInfo ImageUpsampler::processorInfo_{
    "org.inviwo.imageupsampler",  // Class identifier
    "Image Upsampler",            // Display name
    "TNM067",                     // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo ImageUpsampler::getProcessorInfo() const { return processorInfo_; }

ImageUpsampler::ImageUpsampler()
    : Processor()
    , inport_("inport", true)
    , outport_("outport", true)
    , interpolationMethod_("interpolationMethod", "Interpolation Method",
                           {
                               {"piecewiseconstant", "Piecewise Constant (Nearest Neighbor)",
                                IntepolationMethod::PiecewiseConstant},
                               {"bilinear", "Bilinear", IntepolationMethod::Bilinear},
                               {"biquadratic", "Biquadratic", IntepolationMethod::Biquadratic},
                               {"barycentric", "Barycentric", IntepolationMethod::Barycentric},
                           }) {
    addPort(inport_);
    addPort(outport_);
    addProperty(interpolationMethod_);
}

void ImageUpsampler::process() {
    auto inputImage = inport_.getData();
    if (inputImage->getDataFormat()->getComponents() != 1) {
        LogError("The ImageUpsampler processor does only support single channel images");
    }

    auto inSize = inport_.getData()->getDimensions();
    auto outDim = outport_.getDimensions();

    auto outputImage = std::make_shared<Image>(outDim, inputImage->getDataFormat());
    outputImage->getColorLayer()->setSwizzleMask(inputImage->getColorLayer()->getSwizzleMask());
    outputImage->getColorLayer()
        ->getEditableRepresentation<LayerRAM>()
        ->dispatch<void, dispatching::filter::Scalars>([&](auto outRep) {
            auto inRep = inputImage->getColorLayer()->getRepresentation<LayerRAM>();
            detail::upsample(interpolationMethod_.get(), *(const decltype(outRep))(inRep), *outRep);
        });

    outport_.setData(outputImage);
}

dvec2 ImageUpsampler::convertCoordinate(ivec2 outImageCoords, size2_t inputSize, size2_t outputSize) {
    // TODO implement
    dvec2 c(outImageCoords);

    // TASK 5: Convert the outImageCoords to its coordinates in the input image
    c[0] *= inputSize[0] / (outputSize[0] * 1.0);
    c[1] *= inputSize[1] / (outputSize[1] * 1.0);

    return c;
}

}  // namespace inviwo
