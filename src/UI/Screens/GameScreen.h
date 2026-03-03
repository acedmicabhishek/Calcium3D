#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "../../Core/State.h"

class GameScreen : public State {
public:
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;
};

#endif
