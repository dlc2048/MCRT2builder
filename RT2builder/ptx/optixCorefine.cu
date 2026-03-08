#include <optix.h>

#include "optixCorefine.h"
#include <cuda/helpers.h>

extern "C" {
    __constant__ ParamsCorefine params;
}


extern "C" __global__ void __raygen__core() {
    const uint3 idx = optixGetLaunchIndex();
    unsigned int p0;
    float3 beam_pos = params.beam_pos[idx.x];
    float3 beam_dir = params.beam_dir[idx.x];
    optixTrace(
        params.handle,
        beam_pos,
        beam_dir,
        0.0f,
        1e16f,
        0.0f,
        OptixVisibilityMask(1),
        OPTIX_RAY_FLAG_NONE,
        0,
        0,
        0,
        p0
    );
    params.is_inside[idx.x] = (bool)p0;
}

extern "C" __global__ void __miss__core() {
    optixSetPayload_0(0);
}

extern "C" __global__ void __closesthit__core() {
    unsigned int p;
    if (optixIsBackFaceHit())
        p = 1;
    else
        p = 0;
    optixSetPayload_0(p);
}