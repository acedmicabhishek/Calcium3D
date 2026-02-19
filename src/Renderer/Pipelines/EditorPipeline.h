#pragma once

#include "RenderPipeline.h"

class EditorPipeline : public RenderPipeline
{
public:
    EditorPipeline();
    ~EditorPipeline() override = default;

    void Init();
};
