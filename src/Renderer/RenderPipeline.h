#pragma once

#include <vector>
#include <memory>
#include "RenderPass.h"

class RenderPipeline
{
public:
    RenderPipeline() = default;
    virtual ~RenderPipeline() = default;

    void AddPass(std::unique_ptr<RenderPass> pass);
    void ClearPasses();
    void Execute(const RenderContext& context);
    void Resize(int width, int height);

private:
    std::vector<std::unique_ptr<RenderPass>> m_Passes;
};
