
struct ParamsCorefine {
    bool*                  is_inside;
    float3*                beam_pos;
    float3*                beam_dir;
    unsigned int           size;
    OptixTraversableHandle handle;
};

struct RayGenDataCorefine {
    // No data needed
};

struct MissDataCorefine {
    // No data needed
};

struct HitGroupDataCorefine {
    // No data needed
};

template <typename T>
struct SbtRecord
{
    __align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    T data;
};

typedef SbtRecord<RayGenDataCorefine>     RayGenSbtRecordCorefine;
typedef SbtRecord<MissDataCorefine>       MissSbtRecordCorefine;
typedef SbtRecord<HitGroupDataCorefine>   HitGroupSbtRecordCorefine;