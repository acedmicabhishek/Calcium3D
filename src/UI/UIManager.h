#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "UIElement.h"
#include <vector>

class UIManager {
public:
    static void Render(const std::vector<UIElement>& elements, glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0});
    static void RenderElement(const UIElement& element, glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0});
};

#endif
