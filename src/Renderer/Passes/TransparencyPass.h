#pragma once

#include "RenderPass.h"

class TransparencyPass : public RenderPass
{
public:
    TransparencyPass();
    ~TransparencyPass() override = default;

    void Init() override;
    void Execute(const RenderContext& context) override;
};
