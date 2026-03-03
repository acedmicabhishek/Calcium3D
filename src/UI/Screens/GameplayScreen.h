#ifndef GAMEPLAY_SCREEN_H
#define GAMEPLAY_SCREEN_H

#include "../../Core/State.h"

class GameplayScreen : public State {
public:
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;
};

#endif
