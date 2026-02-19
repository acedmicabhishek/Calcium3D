#pragma once

#include "RenderPass.h"

class ShadowPass : public RenderPass
{
public:
    ShadowPass();
    ~ShadowPass() override;

    void Init() override;
    void Execute(const RenderContext& context) override;
    void Resize(int width, int height) override;

private:
    unsigned int m_FBO = 0;
    unsigned int m_DepthMap = 0;
    int m_ShadowWidth = 2048;
    int m_ShadowHeight = 2048;
};
