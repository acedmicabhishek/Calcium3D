#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <string>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <functional>

enum class UIElementType {
    BUTTON,
    TEXT,
    SLIDER,
    CHECKBOX,
    IMAGE
};

struct UIElement {
    std::string name = "New Element";
    std::string screenName = "StartScreen";
    UIElementType type = UIElementType::TEXT;
    glm::vec2 position = { 0.0f, 0.0f }; 
    glm::vec2 size = { 100.0f, 30.0f };
    
    glm::vec2 anchorMin = { 0.5f, 0.5f }; 
    glm::vec2 anchorMax = { 0.5f, 0.5f };
    glm::vec2 pivot = { 0.5f, 0.5f };

    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::string text = "New Text";
    
    
    std::string actionType = "None"; 
    std::string targetState = "";
    std::string targetAudioObject = "";
    std::string targetVideoObject = "";
    
    std::function<void()> onClick;

    nlohmann::json Serialize() const {
        nlohmann::json j;
        j["name"] = name;
        j["screenName"] = screenName;
        j["type"] = (int)type;
        j["position"] = { position.x, position.y };
        j["size"] = { size.x, size.y };
        j["anchorMin"] = { anchorMin.x, anchorMin.y };
        j["anchorMax"] = { anchorMax.x, anchorMax.y };
        j["pivot"] = { pivot.x, pivot.y };
        j["color"] = { color.r, color.g, color.b, color.a };
        j["text"] = text;
        j["actionType"] = actionType;
        j["targetState"] = targetState;
        j["targetAudioObject"] = targetAudioObject;
        j["targetVideoObject"] = targetVideoObject;
        return j;
    }

    void Deserialize(const nlohmann::json& j) {
        if (j.contains("name")) name = j["name"];
        if (j.contains("screenName")) screenName = j["screenName"];
        if (j.contains("type")) type = (UIElementType)j["type"];
        if (j.contains("position")) position = { j["position"][0], j["position"][1] };
        if (j.contains("size")) size = { j["size"][0], j["size"][1] };
        if (j.contains("anchorMin")) anchorMin = { j["anchorMin"][0], j["anchorMin"][1] };
        if (j.contains("anchorMax")) anchorMax = { j["anchorMax"][0], j["anchorMax"][1] };
        if (j.contains("pivot")) pivot = { j["pivot"][0], j["pivot"][1] };
        if (j.contains("color")) color = { j["color"][0], j["color"][1], j["color"][2], j["color"][3] };
        if (j.contains("text")) text = j["text"];
        if (j.contains("actionType")) actionType = j["actionType"];
        if (j.contains("targetState")) targetState = j["targetState"];
        if (j.contains("targetAudioObject")) targetAudioObject = j["targetAudioObject"];
        if (j.contains("targetVideoObject")) targetVideoObject = j["targetVideoObject"];
    }
};

#endif
