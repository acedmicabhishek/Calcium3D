#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "Screen.h"

class GameScreen : public Screen {
public:
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;
};

#endif
