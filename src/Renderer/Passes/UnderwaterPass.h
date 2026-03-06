#pragma once

#include "RenderPass.h"
#include <vector>

class UnderwaterPass : public RenderPass
{
public:
    UnderwaterPass();
    ~UnderwaterPass() override;

    void Init() override;
    void Execute(const RenderContext& context) override;
    void Resize(int width, int height) override;

private:
    void SetupQuad();
    void ResizeResolveFBO(int width, int height);

    unsigned int m_UnderwaterShaderID = 0;
    unsigned int m_QuadVAO = 0;
    unsigned int m_QuadVBO = 0;

    
    unsigned int m_ResolveFBO = 0;
    unsigned int m_ResolveColorObj = 0;
    unsigned int m_ResolveDepthObj = 0;

    int m_LastWidth = 0;
    int m_LastHeight = 0;
};
