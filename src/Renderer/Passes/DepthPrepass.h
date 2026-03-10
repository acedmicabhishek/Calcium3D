#ifndef DEPTH_PREPASS_H
#define DEPTH_PREPASS_H

#include "../RenderContext.h"
#include "../RenderPipeline.h"

class DepthPrepass : public RenderPass {
public:
  DepthPrepass();
  virtual void Init() override;
  virtual void Execute(const RenderContext &context) override;
};

#endif
