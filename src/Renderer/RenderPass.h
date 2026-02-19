#pragma once

#include "RenderContext.h"
#include <string>

class RenderPass
{
public:
    virtual ~RenderPass() = default;

    virtual void Init() = 0;
    virtual void Execute(const RenderContext& context) = 0;
    virtual void Resize(int width, int height) {}

    std::string m_Name;
};
