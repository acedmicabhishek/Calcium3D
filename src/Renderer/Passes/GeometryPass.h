#pragma once

#include "../Camera.h"
#include "../Shader.h"
#include "RenderPass.h"

class GeometryPass : public RenderPass {
public:
  GeometryPass();
  ~GeometryPass() override = default;

  void Init() override;
  void Execute(const RenderContext &context) override;
  void Resize(int width, int height) override {}

private:
  Shader *m_DefaultShader = nullptr;
};
