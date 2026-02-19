#include "RenderPipeline.h"
#include <iostream>

void RenderPipeline::AddPass(std::unique_ptr<RenderPass> pass)
{
    m_Passes.push_back(std::move(pass));
}

void RenderPipeline::Execute(const RenderContext& context)
{
    for (auto& pass : m_Passes)
    {
        pass->Execute(context);
    }
}

void RenderPipeline::Resize(int width, int height)
{
    for (auto& pass : m_Passes)
    {
        pass->Resize(width, height);
    }
}
