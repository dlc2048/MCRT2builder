
#include <optix.h>

#include "render.h"
#include <cuda/helpers.h>

#define RENDER_ALPHA 0.6f

/*
   Payload items
   0: is loop finished
   1: track length (if loop is not finished)
   2: shader red
   3: shader green
   4: shader blue
*/

extern "C" {
    __constant__ Params params;
}

extern "C" __global__ void __raygen__rg() {

    const uint3 idx = optixGetLaunchIndex();
    const uint3 dim = optixGetLaunchDimensions();

    const int    w = params.width;
    const float3 eye = params.eye;
    const float3 U = params.U;
    const float3 V = params.V;
    const float3 W = params.W;
    const float2 d = 2.0f * make_float2(
        static_cast<float>(idx.x) / static_cast<float>(dim.x),
        static_cast<float>(idx.y) / static_cast<float>(dim.y)
    ) - 1.0f;
    const int   method = params.method;
    

    float3 ray_direction = normalize(d.x * U + d.y * V + W);
    float3 ray_origin = eye;
    float3 color = make_float3(0.e0, 0.e0, 0.e0);

    unsigned int p0, p1, p2, p3, p4, p5;
    p5 = 1;

    bool first_hit = false;
    float alpha_total = 0.f;

    do {
        optixTrace(
            params.handle,
            ray_origin,
            ray_direction,
            0.0f,
            1e16,
            0.0f,                    // rayTime
            OptixVisibilityMask(255),
            OPTIX_RAY_FLAG_NONE,
            0,                       // SBT offset
            0,                       // SBT stride
            0,                       // missSBTIndex
            p0, p1, p2, p3, p4, p5   // payload
        );
        float track = __uint_as_float(p1);
        track += 1e-4f;
        ray_origin.x += track * ray_direction.x;
        ray_origin.y += track * ray_direction.y;
        ray_origin.z += track * ray_direction.z;
        if (p0) {  // hit but continue
            if (!first_hit && method) {
                color.x += __uint_as_float(p2) * RENDER_ALPHA;
                color.y += __uint_as_float(p3) * RENDER_ALPHA;
                color.z += __uint_as_float(p4) * RENDER_ALPHA;
                alpha_total += RENDER_ALPHA;
            }
        }
        else {  // end
            color.x += __uint_as_float(p2);
            color.y += __uint_as_float(p3);
            color.z += __uint_as_float(p4);
            alpha_total += 1.f;
        }
        first_hit = true;
    } while (p0);

    color.x /= alpha_total;
    color.y /= alpha_total;
    color.z /= alpha_total;

    params.image[idx.y * w + idx.x]
        = make_color(color);
    params.hit_position[idx.y * w + idx.x]
        = ray_origin;
    params.hit_activated[idx.y * w + idx.x]
        = (bool)p5;
}

extern "C" __global__ void __miss__ms() {
    optixSetPayload_0(0);
    // gray background
    optixSetPayload_2(__float_as_uint(0.2));
    optixSetPayload_3(__float_as_uint(0.2));
    optixSetPayload_4(__float_as_uint(0.2));
    optixSetPayload_5(0);
}

extern "C" __global__ void __closesthit__ch() {
    const int prim_idx = optixGetPrimitiveIndex();
    int region_idx;
    // facet normal calculation
    const float3 v0      = params.vertices[3 * prim_idx + 0];
    const float3 v1      = params.vertices[3 * prim_idx + 1];
    const float3 v2      = params.vertices[3 * prim_idx + 2];
    const float3 n0      = normalize(cross(v1 - v0, v2 - v0));
    const float3 ray_dir = optixGetWorldRayDirection();
    float        weight  = abs(n0.x * ray_dir.x + n0.y * ray_dir.y + n0.z * ray_dir.z);
    // region index
    region_idx = 
          optixIsBackFaceHit() 
        ? params.region_idx_frontface[prim_idx] 
        : params.region_idx_backface[prim_idx];
    optixSetPayload_1(__float_as_uint(optixGetRayTmax()));
    optixSetPayload_2(__float_as_uint(weight * params.color_palette[region_idx].x));
    optixSetPayload_3(__float_as_uint(weight * params.color_palette[region_idx].y));
    optixSetPayload_4(__float_as_uint(weight * params.color_palette[region_idx].z));
    if (region_idx == params.target || params.target < 0)
        optixSetPayload_0(0);
    else
        optixSetPayload_0(1);
}