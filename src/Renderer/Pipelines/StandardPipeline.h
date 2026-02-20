#pragma once

#include "RenderPipeline.h"

class StandardPipeline : public RenderPipeline
{
public:
    StandardPipeline();
    ~StandardPipeline() override = default;

    void Init();
};
