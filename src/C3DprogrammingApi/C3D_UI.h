#ifndef C3D_UI_H
#define C3D_UI_H

#include <string>
#include <glm/glm.hpp>
#include <functional>

namespace C3D {
    namespace UI {
        void CreateButton(const std::string& name, const std::string& text, const glm::vec2& pos, const glm::vec2& size, std::function<void()> onClick);
        void CreateText(const std::string& name, const std::string& text, const glm::vec2& pos, const glm::vec4& color = {1,1,1,1});
        void CreateSlider(const std::string& name, const std::string& label, float* value, float min, float max, const glm::vec2& pos, const glm::vec2& size);
        void CreateCheckbox(const std::string& name, const std::string& label, bool* value, const glm::vec2& pos);
        void CreateImage(const std::string& name, const std::string& texturePath, const glm::vec2& pos, const glm::vec2& size);
        
        void SetElementAction(const std::string& name, const std::string& actionType, const std::string& target);
        void RemoveElement(const std::string& name);
        void ClearElements();
        
        void SetScreen(const std::string& screenName);
        
        
        void SetOnClick(const std::string& name, std::function<void()> callback);
        void SetOnHold(const std::string& name, std::function<void()> callback);
        void SetOnRelease(const std::string& name, std::function<void()> callback);
        void SetOnHoverEnter(const std::string& name, std::function<void()> callback);
        void SetOnHoverExit(const std::string& name, std::function<void()> callback);
    }
}

#endif
