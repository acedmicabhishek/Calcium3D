#pragma once

#include "RenderPass.h"

class SkyPass : public RenderPass
{
public:
    SkyPass();
    ~SkyPass() override = default;

    void Init() override;
    void Execute(const RenderContext& context) override;
    
    static void Reload();
};
