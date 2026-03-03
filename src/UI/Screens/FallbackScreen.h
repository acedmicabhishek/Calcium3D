#ifndef FALLBACK_SCREEN_H
#define FALLBACK_SCREEN_H

#include "../../Core/State.h"

class FallbackScreen : public State {
public:
    FallbackScreen();
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;

private:
    int m_StateId = 0;
};

#endif
