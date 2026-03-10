#pragma once

#include "RenderPass.h"

class ShadowPass : public RenderPass {
public:
  ShadowPass();
  ~ShadowPass() override;

  void Init() override;
  void Execute(const RenderContext &context) override;
  void Resize(int width, int height) override;

private:
  unsigned int m_DirShadowFBO = 0;
  unsigned int m_DirShadowMap = 0;
  int m_DirShadowRes = 2048;

  unsigned int m_PointShadowFBO = 0;
  unsigned int m_PointShadowMaps[4] = {0};
  int m_PointShadowRes = 1024;
  int m_CurrentPointShadowRes[4] = {1024, 1024, 1024, 1024};
};
