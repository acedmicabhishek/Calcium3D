#pragma once

#include "RenderPass.h"

class GeometryPass : public RenderPass
{
public:
    GeometryPass();
    ~GeometryPass() override = default;

    void Init() override;
    void Execute(const RenderContext& context) override;
};
