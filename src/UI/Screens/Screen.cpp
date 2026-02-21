#include "Screen.h"
#include "../UICreationEngine.h"

void Screen::LoadFromLayout(const std::string& screenName) {
    m_ScreenName = screenName;
    m_UIElements = UICreationEngine::GetElementsByScreen(screenName);
}
