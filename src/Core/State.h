#ifndef STATE_H
#define STATE_H

#include <any>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../UI/UIElement.h"


class State {
public:
    virtual ~State() = default;

    virtual void OnEnter(std::any payload) {}
    virtual void OnExit() {}
    virtual void OnPause() {}
    virtual void OnResume() {}

    virtual void Update(float deltaTime) {}
    virtual void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) {}

    
    virtual void Init() {} 
    void LoadFromLayout(const std::string& screenName);
    void ApplyStateConfig(const std::string& stateName);

    void SetStateName(const std::string& name) { m_StateName = name; }
    std::string GetStateName() const { return m_StateName; }

protected:
    std::string m_StateName;
    std::vector<UIElement> m_UIElements;
};

#endif
