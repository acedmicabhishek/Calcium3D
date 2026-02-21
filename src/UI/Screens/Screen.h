#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include "../UI/UIElement.h"

class Screen {
public:
    virtual ~Screen() = default;
    virtual void Init() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) = 0;

    void LoadFromLayout(const std::string& screenName);
    
protected:
    std::string m_ScreenName;
    std::vector<UIElement> m_UIElements;
};

#endif
