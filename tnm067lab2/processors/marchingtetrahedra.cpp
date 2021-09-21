#include <modules/tnm067lab2/processors/marchingtetrahedra.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/tnm067lab1/utils/interpolationmethods.h>

namespace inviwo {

size_t MarchingTetrahedra::HashFunc::max = 1;

const ProcessorInfo MarchingTetrahedra::processorInfo_{
    "org.inviwo.MarchingTetrahedra",  // Class identifier
    "Marching Tetrahedra",            // Display name
    "TNM067",                         // Category
    CodeState::Stable,                // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo MarchingTetrahedra::getProcessorInfo() const { return processorInfo_; }

MarchingTetrahedra::MarchingTetrahedra()
    : Processor()
    , volume_("volume")
    , mesh_("mesh")
    , isoValue_("isoValue", "ISO value", 0.5f, 0.0f, 1.0f) {

    addPort(volume_);
    addPort(mesh_);

    addProperty(isoValue_);

    isoValue_.setSerializationMode(PropertySerializationMode::All);

    volume_.onChange([&]() {
        if (!volume_.hasData()) {
            return;
        }
        NetworkLock lock(getNetwork());
        float iso = (isoValue_.get() - isoValue_.getMinValue()) /
                    (isoValue_.getMaxValue() - isoValue_.getMinValue());
        const auto vr = volume_.getData()->dataMap_.valueRange;
        isoValue_.setMinValue(static_cast<float>(vr.x));
        isoValue_.setMaxValue(static_cast<float>(vr.y));
        isoValue_.setIncrement(static_cast<float>(glm::abs(vr.y - vr.x) / 50.0));
        isoValue_.set(static_cast<float>(iso * (vr.y - vr.x) + vr.x));
        isoValue_.setCurrentStateAsDefault();
    });
}

vec3 MarchingTetrahedra::TriangleCreator::interpolatePosition(const DataPoint& dp1, const DataPoint& dp2) {
    if(dp1.value == iso) return dp1.pos;
    if(dp2.value == iso) return dp2.pos;

    // float interpolationValue = (iso - dp1.value) / (dp2.value - dp1.value);
    // if (dp1.value > dp2.value) interpolationValue = 1.0f - interpolationValue;
    // return (dp1.pos * (1.0f - interpolationValue)) + (dp2.pos * interpolationValue);
    
    return dp1.pos + ((dp2.pos - dp1.pos) * (iso - dp1.value)) / (dp2.value - dp1.value);
}

void MarchingTetrahedra::TriangleCreator::createTriangle(bool inverted, std::pair<int, int> line1, std::pair<int, int> line2, std::pair<int, int> line3) {
    auto v0 = mesh.addVertex(
        interpolatePosition(tetrahedra.dataPoints[line1.first], tetrahedra.dataPoints[line1.second]),
        tetrahedra.dataPoints[line1.first].index,
        tetrahedra.dataPoints[line1.second].index
    );
    auto v1 = mesh.addVertex(
        interpolatePosition(tetrahedra.dataPoints[line2.first], tetrahedra.dataPoints[line2.second]),
        tetrahedra.dataPoints[line2.first].index,
        tetrahedra.dataPoints[line2.second].index
    );
    auto v2 = mesh.addVertex(
        interpolatePosition(tetrahedra.dataPoints[line3.first], tetrahedra.dataPoints[line3.second]),
        tetrahedra.dataPoints[line3.first].index,
        tetrahedra.dataPoints[line3.second].index
    );

    if (inverted) {
        mesh.addTriangle(v0, v2, v1);
    } else {
        mesh.addTriangle(v0, v1, v2);
    }
}

void MarchingTetrahedra::process() {
    auto volume = volume_.getData()->getRepresentation<VolumeRAM>();
    MeshHelper mesh(volume_.getData());

    const auto& dims = volume->getDimensions();
    MarchingTetrahedra::HashFunc::max = dims.x * dims.y * dims.z;

    const float iso = isoValue_.get();

    util::IndexMapper3D indexInVolume(dims);

    const static size_t tetrahedraIds[6][4] = {{0, 1, 2, 5}, {1, 3, 2, 5}, {3, 2, 5, 7},
                                               {0, 2, 4, 5}, {6, 4, 2, 5}, {6, 7, 5, 2}};

    size3_t pos{};
    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                // Step 1: create current cell
                
                // The DataPoint index should be the 1D-index for the DataPoint in the cell
                // Use volume->getAsDouble to query values from the volume
                // Spatial position should be between 0 and 1

                Cell c;
                for (int z = 0; z < 2; z++)
                    for(int y = 0; y < 2; y++)
                        for (int x = 0; x < 2; x++) {
                            const ivec3 cellPos{x, y, z};
                            const size3_t cellPosInVolume{pos.x + x, pos.y + y, pos.z + z};

                            size_t index = indexInVolume(cellPosInVolume);
                            vec3 dpPos = calculateDataPointPos(pos, cellPos, dims);
                            float value = static_cast<float>(volume->getAsDouble(cellPosInVolume));
                            c.dataPoints[calculateDataPointIndexInCell(cellPos)] = {dpPos, value, index};
                        }

                // Step 2: Subdivide cell into tetrahedra (hint: use tetrahedraIds)
                std::vector<Tetrahedra> tetrahedras;
                for (const auto tetrahedraId : tetrahedraIds) {
                    Tetrahedra tetrahedra;

                    for (size_t tetrahedraIdx = 0; tetrahedraIdx < 4; ++tetrahedraIdx)
                        tetrahedra.dataPoints[tetrahedraIdx] = c.dataPoints[tetrahedraId[tetrahedraIdx]];

                    tetrahedras.push_back(tetrahedra);
                }

                for (const Tetrahedra& tetrahedra : tetrahedras) {
                    // Step three: Calculate for tetra case index
                    int caseId = 0;

                    for (size_t i = 0; i < 4; ++i)
                        if (tetrahedra.dataPoints[i].value > iso)
                            caseId |= (int) pow(2, i); // pow(2, i) = 1, 2, 4 or 8

                    // step four: Extract triangles
                    TriangleCreator tc{mesh, iso, tetrahedra};

					switch (caseId) {
                        case 0:
                        case 15: {
                            break;
                        }
                        case 1:
                        case 14: {
                            tc.createTriangle(caseId == 14, {0, 1}, {0, 3}, {0, 2});
                            break;
                        }
                        case 2:
                        case 13: {
                            tc.createTriangle(caseId == 13, {1, 0}, {1, 2}, {1, 3});
                            break;
                        }
                        case 3:
                        case 12: {
                            tc.createTriangle(caseId == 12, {1, 2}, {1, 3}, {0, 3});
                            tc.createTriangle(caseId == 12, {1, 2}, {0, 3}, {0, 2});
                            break;
                        }
                        case 4:
                        case 11: {
                            tc.createTriangle(caseId == 11, {2, 3}, {2, 1}, {2, 0});
                            break;
                        }
                        case 5:
                        case 10: {
                            tc.createTriangle(caseId == 10, {2, 1}, {0, 1}, {0, 3});
                            tc.createTriangle(caseId == 10, {2, 3}, {2, 1}, {0, 3});
                            break;
                        }
                        case 6:
                        case 9: {
                            tc.createTriangle(caseId == 9, {2, 0}, {1, 3}, {1, 0});
                            tc.createTriangle(caseId == 9, {2, 0}, {1, 3}, {2, 3});
                            break;
                        }
                        case 7:
                        case 8: {
                            tc.createTriangle(caseId == 8, {3, 1}, {3, 0}, {3, 2});
                            break;
                        }
                    }
                }
            }
        }
    }

    mesh_.setData(mesh.toBasicMesh());
}

int MarchingTetrahedra::calculateDataPointIndexInCell(ivec3 index3D) {
    // TODO: TASK 5: map 3D index to 2D
    // Sting to binary ex (0,0,1) --> "100" = index 4
    return std::stoi(std::to_string(index3D.z) + std::to_string(index3D.y) + std::to_string(index3D.x), nullptr, 2);
}

vec3 MarchingTetrahedra::calculateDataPointPos(size3_t posVolume, ivec3 posCell, ivec3 dims) {
    // TODO: TASK 5: scale DataPoint position with dimensions to be between 0 and 1
    const float x = (posVolume.x + posCell.x) / (dims.x - 1.0f);
    const float y = (posVolume.y + posCell.y) / (dims.y - 1.0f);
    const float z = (posVolume.z + posCell.z) / (dims.z - 1.0f);
    
    return {x, y, z};
}

MarchingTetrahedra::MeshHelper::MeshHelper(std::shared_ptr<const Volume> vol)
    : edgeToVertex_()
    , vertices_()
    , mesh_(std::make_shared<BasicMesh>())
    , indexBuffer_(mesh_->addIndexBuffer(DrawType::Triangles, ConnectivityType::None)) {
    mesh_->setModelMatrix(vol->getModelMatrix());
    mesh_->setWorldMatrix(vol->getWorldMatrix());
}

void MarchingTetrahedra::MeshHelper::addTriangle(size_t i0, size_t i1, size_t i2) {
    IVW_ASSERT(i0 != i1, "i0 and i1 should not be the same value");
    IVW_ASSERT(i0 != i2, "i0 and i2 should not be the same value");
    IVW_ASSERT(i1 != i2, "i1 and i2 should not be the same value");

    indexBuffer_->add(static_cast<glm::uint32_t>(i0));
    indexBuffer_->add(static_cast<glm::uint32_t>(i1));
    indexBuffer_->add(static_cast<glm::uint32_t>(i2));

    const auto a = std::get<0>(vertices_[i0]);
    const auto b = std::get<0>(vertices_[i1]);
    const auto c = std::get<0>(vertices_[i2]);

    const vec3 n = glm::normalize(glm::cross(b - a, c - a));
    std::get<1>(vertices_[i0]) += n;
    std::get<1>(vertices_[i1]) += n;
    std::get<1>(vertices_[i2]) += n;
}

std::shared_ptr<BasicMesh> MarchingTetrahedra::MeshHelper::toBasicMesh() {
    for (auto& vertex : vertices_) {
        // Normalize the normal of the vertex
        std::get<1>(vertex) = glm::normalize(std::get<1>(vertex));
    }
    mesh_->addVertices(vertices_);
    return mesh_;
}

std::uint32_t MarchingTetrahedra::MeshHelper::addVertex(vec3 pos, size_t i, size_t j) {
    IVW_ASSERT(i != j, "i and j should not be the same value");
    if (j < i) std::swap(i, j);

    auto [edgeIt, inserted] = edgeToVertex_.try_emplace(std::make_pair(i, j), vertices_.size());
    if (inserted) {
        vertices_.push_back({pos, vec3(0, 0, 0), pos, vec4(0.7f, 0.7f, 0.7f, 1.0f)});
    }
    return static_cast<std::uint32_t>(edgeIt->second);
}

}  // namespace inviwo
