#ifndef PLAY_MODE_H
#define PLAY_MODE_H

#include <string>
#include <vector>
#include <imgui.h>
#include "../UI/UIElement.h"

class PlayMode {
public:
    
    static void RenderAndHandleClicks(
        std::vector<UIElement>& elements,
        float viewportWidth, float viewportHeight,
        ImVec2 cursorPos,
        std::string& activeScreen
    );
};

#endif
