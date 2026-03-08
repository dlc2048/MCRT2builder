#pragma once

#include <string>
#include <iostream>
#include <sstream>

#include <cuda_runtime.h>


struct RenderSettings {
    std::string input;  // Geometry file path
    int2   resolution;
    float3 eye;         // Camera eye
    float3 look;        // Camera lookat
    float  fov;         // Camera field of view

    // default
    RenderSettings() :
        input(".cache/world.bin"),
        resolution({768, 768}),
        eye       ({ 0.f, 0.f, 1000.f }),
        look      ({ 0.f, 0.f, 0.f }),
        fov       (35.f) {}
};


RenderSettings interpretCommandLine(int argc, char* argv[]);
void printHelp(int err);