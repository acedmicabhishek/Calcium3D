#ifndef FALLBACK_SCREEN_H
#define FALLBACK_SCREEN_H

#include "Screen.h"

class FallbackScreen : public Screen {
public:
    FallbackScreen(int stateId);
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;

private:
    int m_StateId = 0;
};

#endif
