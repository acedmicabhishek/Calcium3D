#include "StandardPipeline.h"
#include "Passes/ShadowPass.h"
#include "Passes/SkyPass.h"
#include "Passes/GeometryPass.h"
#include "Passes/TransparencyPass.h"

StandardPipeline::StandardPipeline()
{
}

void StandardPipeline::Init()
{
    // Pass 1: ShadowPass — depth map from sun (stub, restores FBO)
    auto shadowPass = std::make_unique<ShadowPass>();
    shadowPass->Init();
    AddPass(std::move(shadowPass));

    // Pass 2: SkyPass — gradient sky or cubemap (before geometry)
    auto skyPass = std::make_unique<SkyPass>();
    skyPass->Init();
    AddPass(std::move(skyPass));

    // Pass 3: GeometryPass — opaque scene objects with full lighting
    auto geometryPass = std::make_unique<GeometryPass>();
    geometryPass->Init();
    AddPass(std::move(geometryPass));

    // Pass 4: TransparencyPass — clouds and water (alpha blended)
    auto transparencyPass = std::make_unique<TransparencyPass>();
    transparencyPass->Init();
    AddPass(std::move(transparencyPass));
}
