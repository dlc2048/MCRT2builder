
#include "color.hpp"

namespace shader {

    std::vector<float3> COLOR_PALETTE;

    inline float rgbLinearize(uint8_t value) {
        float vf = (float)value / 255.f;
        return vf > 0.04045 ? std::pow((vf + 0.055) / 1.055, 2.4) : vf / 12.92;
    }

    inline float3 rgbToLinearRgb(uint8_t r, uint8_t g, uint8_t b) {
        return make_float3(
            rgbLinearize(r),
            rgbLinearize(g),
            rgbLinearize(b)
        );
    }

    void initializeColorSpace() {
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 128, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(128, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 000));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 128));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 255, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 128, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(000, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(128, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 255));
        COLOR_PALETTE.push_back(rgbToLinearRgb(255, 000, 128));
    }

}