
struct Params {
    unsigned int           subframe_index;
    uchar4*                image;
    float3*                hit_position;
    int*                   hit_activated;
    unsigned int           width;
    unsigned int           height;
    int                    target;
    int                    method;

    int*                   surface_idx;
    int*                   region_idx_backface;
    int*                   region_idx_frontface;
    float3*                vertices;
    float3*                color_palette;

    float3                 eye;
    float3                 U;
    float3                 V;
    float3                 W;

    OptixTraversableHandle handle;
};

struct RayGenData
{
    // No data needed
};


struct MissData
{
    // No data needed
};


struct HitGroupData
{
    // No data needed
};
