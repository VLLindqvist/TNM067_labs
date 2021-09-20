/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>
#include <modules/tnm067lab2/processors/marchingtetrahedra.h>

namespace inviwo {

static constexpr std::array<std::pair<ivec3, int>, 8> toTestIndex = {
    {
        {ivec3(0, 0, 0), 0},
        {ivec3(1, 0, 0), 1},
        {ivec3(0, 1, 0), 2},
        {ivec3(1, 1, 0), 3},
        {ivec3(0, 0, 1), 4},
        {ivec3(1, 0, 1), 5},
        {ivec3(0, 1, 1), 6},
        {ivec3(1, 1, 1), 7}
    }
};

static constexpr std::array<std::pair<std::tuple<vec3, ivec3, ivec3>, vec3>, 64> toTestCalcVoxelPos = {
    {
        {{vec3(0, 0, 0), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0, 0, 0)},
        {{vec3(0, 0, 0), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0, 0)},
        {{vec3(0, 0, 0), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0, 0.5, 0)},
        {{vec3(0, 0, 0), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0)},
        {{vec3(0, 0, 0), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0, 0, 0.5)},
        {{vec3(0, 0, 0), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0, 0.5)},
        {{vec3(0, 0, 0), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0, 0.5, 0.5)},
        {{vec3(0, 0, 0), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(1, 0, 0), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0, 0)},
        {{vec3(1, 0, 0), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(1, 0, 0)},
        {{vec3(1, 0, 0), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0)},
        {{vec3(1, 0, 0), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(1, 0.5, 0)},
        {{vec3(1, 0, 0), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0, 0.5)},
        {{vec3(1, 0, 0), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(1, 0, 0.5)},
        {{vec3(1, 0, 0), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(1, 0, 0), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(1, 0.5, 0.5)},
        {{vec3(0, 1, 0), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0, 0.5, 0)},
        {{vec3(0, 1, 0), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0)},
        {{vec3(0, 1, 0), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0, 1, 0)},
        {{vec3(0, 1, 0), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 1, 0)},
        {{vec3(0, 1, 0), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0, 0.5, 0.5)},
        {{vec3(0, 1, 0), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(0, 1, 0), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0, 1, 0.5)},
        {{vec3(0, 1, 0), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 1, 0.5)},
        {{vec3(1, 1, 0), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0)},
        {{vec3(1, 1, 0), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(1, 0.5, 0)},
        {{vec3(1, 1, 0), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 1, 0)},
        {{vec3(1, 1, 0), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(1, 1, 0)},
        {{vec3(1, 1, 0), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(1, 1, 0), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(1, 0.5, 0.5)},
        {{vec3(1, 1, 0), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 1, 0.5)},
        {{vec3(1, 1, 0), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(1, 1, 0.5)},
        {{vec3(0, 0, 1), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0, 0, 0.5)},
        {{vec3(0, 0, 1), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0, 0.5)},
        {{vec3(0, 0, 1), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0, 0.5, 0.5)},
        {{vec3(0, 0, 1), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(0, 0, 1), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0, 0, 1)},
        {{vec3(0, 0, 1), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0, 1)},
        {{vec3(0, 0, 1), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0, 0.5, 1)},
        {{vec3(0, 0, 1), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 1)},
        {{vec3(1, 0, 1), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0, 0.5)},
        {{vec3(1, 0, 1), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(1, 0, 0.5)},
        {{vec3(1, 0, 1), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(1, 0, 1), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(1, 0.5, 0.5)},
        {{vec3(1, 0, 1), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0, 1)},
        {{vec3(1, 0, 1), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(1, 0, 1)},
        {{vec3(1, 0, 1), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 1)},
        {{vec3(1, 0, 1), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(1, 0.5, 1)},
        {{vec3(0, 1, 1), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0, 0.5, 0.5)},
        {{vec3(0, 1, 1), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(0, 1, 1), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0, 1, 0.5)},
        {{vec3(0, 1, 1), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 1, 0.5)},
        {{vec3(0, 1, 1), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0, 0.5, 1)},
        {{vec3(0, 1, 1), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 1)},
        {{vec3(0, 1, 1), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0, 1, 1)},
        {{vec3(0, 1, 1), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 1, 1)},
        {{vec3(1, 1, 1), ivec3(0, 0, 0), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 0.5)},
        {{vec3(1, 1, 1), ivec3(1, 0, 0), ivec3(3, 3, 3)}, vec3(1, 0.5, 0.5)},
        {{vec3(1, 1, 1), ivec3(0, 1, 0), ivec3(3, 3, 3)}, vec3(0.5, 1, 0.5)},
        {{vec3(1, 1, 1), ivec3(1, 1, 0), ivec3(3, 3, 3)}, vec3(1, 1, 0.5)},
        {{vec3(1, 1, 1), ivec3(0, 0, 1), ivec3(3, 3, 3)}, vec3(0.5, 0.5, 1)},
        {{vec3(1, 1, 1), ivec3(1, 0, 1), ivec3(3, 3, 3)}, vec3(1, 0.5, 1)},
        {{vec3(1, 1, 1), ivec3(0, 1, 1), ivec3(3, 3, 3)}, vec3(0.5, 1, 1)},
        {{vec3(1, 1, 1), ivec3(1, 1, 1), ivec3(3, 3, 3)}, vec3(1, 1, 1)}
    }
};

#if ENABLE_DATAPOINT_INDEX_TEST == 1
TEST(MarchingTetrahedraTests, CalculateVoxelIndexInCell) {
    for (const auto& p : toTestIndex) {
        int res = MarchingTetrahedra::calculateDataPointIndexInCell(p.first);
        EXPECT_EQ(p.second, res);
    }
}
#endif

#if ENABLE_DATAPOINT_POS_TEST == 1
TEST(MarchingTetrahedraTests, CalculateVoxelPos) { 
     for (const auto& p : toTestCalcVoxelPos) {
        vec3 res = MarchingTetrahedra::calculateDataPointPos(std::get<0>(p.first), std::get<1>(p.first), std::get<2>(p.first));
         EXPECT_NEAR(p.second.x, res.x, 0.000001);
         EXPECT_NEAR(p.second.y, res.y, 0.000001);
         EXPECT_NEAR(p.second.z, res.z, 0.000001);
      }
}
#endif

}  // namespace inviwo 
