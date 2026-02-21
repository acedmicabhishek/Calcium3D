#include "UIManager.h"
#include <imgui.h>

void UIManager::Render(const std::vector<UIElement>& elements, glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    for (const auto& el : elements) {
        RenderElement(el, canvasSize, baseScreenPos);
    }
}

void UIManager::RenderElement(const UIElement& element, glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    
    float centerX = canvasSize.x * element.anchorMin.x;
    float centerY = canvasSize.y * element.anchorMin.y;
    
    
    float finalX = centerX + element.position.x - (element.size.x * element.pivot.x);
    float finalY = centerY + element.position.y - (element.size.y * element.pivot.y);

    
    ImGui::SetCursorScreenPos(ImVec2(baseScreenPos.x + finalX, baseScreenPos.y + finalY));

    ImGui::PushID(element.name.c_str());

    
    if (element.type == UIElementType::TEXT) {
        ImGui::TextColored(ImVec4(element.color.r, element.color.g, element.color.b, element.color.a), "%s", element.text.c_str());
    } else if (element.type == UIElementType::BUTTON) {
        if (ImGui::Button(element.text.c_str(), ImVec2(element.size.x, element.size.y))) {
            if (element.onClick) element.onClick();
        }
    } else if (element.type == UIElementType::CHECKBOX) {
        bool val = false;
        ImGui::Checkbox(element.text.c_str(), &val);
    } else if (element.type == UIElementType::SLIDER) {
        float fval = 0.5f;
        ImGui::SliderFloat(element.text.c_str(), &fval, 0.0f, 1.0f);
    }
    
    ImGui::PopID();
}
