#ifndef UI_CREATION_ENGINE_H
#define UI_CREATION_ENGINE_H

#include "UIElement.h"
#include <vector>
#include <string>

class UICreationEngine {
public:
    static void AddElement(const UIElement& element) { m_Elements.push_back(element); }
    static std::vector<UIElement>& GetElements() { return m_Elements; }
    
    static std::vector<UIElement> GetElementsByScreen(const std::string& screenName) {
        std::vector<UIElement> result;
        for (const auto& el : m_Elements) {
            if (el.screenName == screenName) result.push_back(el);
        }
        return result;
    }
    
    static void SaveLayout(const std::string& path);
    static void LoadLayout(const std::string& path);
    static void Clear() { m_Elements.clear(); }

private:
    static std::vector<UIElement> m_Elements;
};

#endif
