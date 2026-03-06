#include "StandardPipeline.h"
#include "Passes/ShadowPass.h"
#include "Passes/SkyPass.h"
#include "Passes/GeometryPass.h"
#include "Passes/SSRPass.h"
#include "Passes/TransparencyPass.h"
#include "Passes/UnderwaterPass.h"

StandardPipeline::StandardPipeline()
{
}

void StandardPipeline::Init()
{
    ClearPasses();
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

    // Pass 3.5: Screen Space Reflections (Forward G-Buffer, after Geometry)
    auto ssrPassGeometry = std::make_unique<SSRPass>(SSRPass::PassType::Geometry);
    ssrPassGeometry->Init();
    AddPass(std::move(ssrPassGeometry));

    // Pass 3.6: Cubemap Reflection on Geometry — fires BEFORE TransparencyPass clears normals
    auto cubemapGeoPass = std::make_unique<SSRPass>(SSRPass::PassType::CubemapOnly);
    cubemapGeoPass->Init();
    AddPass(std::move(cubemapGeoPass));

    // Pass 4: TransparencyPass — clouds and water (alpha blended)
    auto transparencyPass = std::make_unique<TransparencyPass>();
    transparencyPass->Init();
    AddPass(std::move(transparencyPass));
    
    // Pass 4.5: Screen Space Reflections (Forward G-Buffer, after Transparency)
    auto ssrPassTransparency = std::make_unique<SSRPass>(SSRPass::PassType::Transparency);
    ssrPassTransparency->Init();
    AddPass(std::move(ssrPassTransparency));
    
    // Pass 5: SSR Reflect-All — runs after all previous passes so geometry reflects clouds/stars too
    auto ssrPassAll = std::make_unique<SSRPass>(SSRPass::PassType::All);
    ssrPassAll->Init();
    AddPass(std::move(ssrPassAll));

    // Pass 6: Cubemap Reflection on Transparency — handles water after transparency pass
    auto cubemapTransPass = std::make_unique<SSRPass>(SSRPass::PassType::CubemapOnly);
    cubemapTransPass->Init();
    AddPass(std::move(cubemapTransPass));

    // Pass 7: UnderwaterPass — Final post-process if camera is submerged
    auto underwaterPass = std::make_unique<UnderwaterPass>();
    underwaterPass->Init();
    AddPass(std::move(underwaterPass));
}
