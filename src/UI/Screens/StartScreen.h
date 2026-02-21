#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "Screen.h"

class StartScreen : public Screen {
public:
    void Init() override;
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;
};

#endif
