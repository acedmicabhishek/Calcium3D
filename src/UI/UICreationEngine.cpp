#include "UICreationEngine.h"
#include <fstream>

std::vector<UIElement> UICreationEngine::m_Elements;

void UICreationEngine::SaveLayout(const std::string &path) {
  nlohmann::json j = nlohmann::json::array();
  for (const auto &el : m_Elements) {
    j.push_back(el.Serialize());
  }

  std::ofstream file(path);
  if (file.is_open()) {
    file << j.dump(4);
    file.close();
  }
}

void UICreationEngine::LoadLayout(const std::string &path) {
  std::ifstream file(path);
  if (file.is_open()) {
    nlohmann::json j = nlohmann::json::parse(file);
    Clear();
    for (const auto &elJson : j) {
      UIElement el;
      el.Deserialize(elJson);
      m_Elements.push_back(el);
    }
    file.close();
  }
}
