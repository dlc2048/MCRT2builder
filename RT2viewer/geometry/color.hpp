#pragma once

#include <cmath>
#include <vector>
#include <stdint.h>

#include <cuda_runtime.h>

namespace shader {

    extern std::vector<float3> COLOR_PALETTE;

    inline float rgbLinearize(uint8_t value);
    inline float3 rgbToLinearRgb(uint8_t r, uint8_t g, uint8_t b);

    void initializeColorSpace();

}