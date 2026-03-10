#include "ResourceManager.h"
#include "Application.h"
#include <filesystem>
#include <iostream>

std::unordered_map<std::string, Shader> ResourceManager::Shaders;
std::unordered_map<std::string, Texture> ResourceManager::Textures;

std::string ResourceManager::ResolvePath(const std::string &path) {
  if (path.empty())
    return "";

  if (std::filesystem::path(path).is_absolute()) {
    if (std::filesystem::exists(path))
      return path;
  }

  std::string projectRoot = Application::Get().GetProjectRoot();

  if (!projectRoot.empty() && path.length() >= 11 &&
      path.substr(0, 11) == "../shaders/") {
    std::filesystem::path overridenPath =
        std::filesystem::path(projectRoot) / "Shaders" / path.substr(11);
    if (std::filesystem::exists(overridenPath))
      return overridenPath.string();
  }

  if (!projectRoot.empty()) {
    std::filesystem::path pRootDirect =
        std::filesystem::path(projectRoot) / path;
    if (std::filesystem::exists(pRootDirect))
      return pRootDirect.string();

    std::filesystem::path projectShadersDir =
        std::filesystem::path(projectRoot) / "Shaders";
    if (std::filesystem::exists(projectShadersDir)) {

      std::filesystem::path pDirect = projectShadersDir / path;
      if (std::filesystem::exists(pDirect))
        return pDirect.string();

      std::string filename = std::filesystem::path(path).filename().string();
      try {
        for (const auto &entry :
             std::filesystem::recursive_directory_iterator(projectShadersDir)) {
          if (entry.is_regular_file() &&
              entry.path().filename().string() == filename) {
            return entry.path().string();
          }
        }
      } catch (...) {
      }
    }

    try {
      std::string filename = std::filesystem::path(path).filename().string();
      for (const auto &entry :
           std::filesystem::recursive_directory_iterator(projectRoot)) {
        if (entry.is_regular_file() &&
            entry.path().filename().string() == filename) {
          return entry.path().string();
        }
      }
    } catch (...) {
    }
  }
  std::string engineRoot = Application::Get().GetEngineRoot();

  std::vector<std::pair<std::string, std::string>> engineDirs = {
      {"shaders", ""},
      {"Resource", ""},
      {"../shaders", ""},
      {"../Resource", ""},
      {"Internal/shaders", "../shaders/"},
      {"Internal/Resource", "../Resource/"},
      {"../../shaders", ""},
      {"../../Resource", ""}};

  if (!engineRoot.empty()) {
    engineDirs.insert(engineDirs.begin(), {engineRoot + "/shaders", ""});
    engineDirs.insert(engineDirs.begin() + 1, {engineRoot + "/Resource", ""});
  }

  for (const auto &pair : engineDirs) {
    const std::string &dir = pair.first;
    const std::string &prefix = pair.second;

    if (std::filesystem::exists(dir)) {

      std::filesystem::path eDirect = std::filesystem::path(dir) / path;
      if (std::filesystem::exists(eDirect))
        return eDirect.string();

      if (!prefix.empty() && path.length() > prefix.length() &&
          path.substr(0, prefix.length()) == prefix) {
        std::filesystem::path strippedPath =
            std::filesystem::path(dir) / path.substr(prefix.length());
        if (std::filesystem::exists(strippedPath))
          return strippedPath.string();
      }

      std::string filename = std::filesystem::path(path).filename().string();
      try {
        for (const auto &entry :
             std::filesystem::recursive_directory_iterator(dir)) {
          if (entry.is_regular_file() &&
              entry.path().filename().string() == filename) {
            return entry.path().string();
          }
        }
      } catch (...) {
      }
    }
  }

  if (std::filesystem::exists(path))
    return path;

  std::cerr << "ResourceManager: [ERROR] Failed to resolve path: " << path
            << std::endl;
  return path;
}

Shader &ResourceManager::LoadShader(const std::string &name,
                                    const char *vShaderFile,
                                    const char *fShaderFile,
                                    const char *gShaderFile) {
  if (Shaders.find(name) != Shaders.end()) {
    return Shaders.at(name);
  }

  std::string vPath = ResolvePath(vShaderFile);
  std::string fPath = ResolvePath(fShaderFile);
  std::string gPath = gShaderFile != nullptr ? ResolvePath(gShaderFile) : "";

  if (gShaderFile != nullptr) {
    Shaders.emplace(
        std::piecewise_construct, std::forward_as_tuple(name),
        std::forward_as_tuple(vPath.c_str(), fPath.c_str(), gPath.c_str()));
  } else {
    Shaders.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                    std::forward_as_tuple(vPath.c_str(), fPath.c_str()));
  }

  return Shaders.at(name);
}

Shader &ResourceManager::ReloadShader(const std::string &name,
                                      const char *vShaderFile,
                                      const char *fShaderFile,
                                      const char *gShaderFile) {
  std::string vPath = ResolvePath(vShaderFile);
  std::string fPath = ResolvePath(fShaderFile);
  std::string gPath = gShaderFile != nullptr ? ResolvePath(gShaderFile) : "";

  if (Shaders.find(name) != Shaders.end()) {
    Shaders.at(name).Delete();
    Shaders.erase(name);
  }

  if (gShaderFile != nullptr) {
    Shaders.emplace(
        std::piecewise_construct, std::forward_as_tuple(name),
        std::forward_as_tuple(vPath.c_str(), fPath.c_str(), gPath.c_str()));
  } else {
    Shaders.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                    std::forward_as_tuple(vPath.c_str(), fPath.c_str()));
  }

  return Shaders.at(name);
}

Shader &ResourceManager::GetShader(const std::string &name) {
  if (Shaders.find(name) == Shaders.end()) {
    std::cerr << "ResourceManager: Shader not found: " << name << std::endl;
    return Shaders.begin()->second;
  }
  return Shaders.at(name);
}

bool ResourceManager::HasShader(const std::string &name) {
  return Shaders.find(name) != Shaders.end();
}

Texture &ResourceManager::LoadTexture(const std::string &name, const char *file,
                                      const char *texType, GLuint slot) {
  if (Textures.find(name) != Textures.end()) {
    return Textures.at(name);
  }

  std::string path = ResolvePath(file);
  Textures.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                   std::forward_as_tuple(path.c_str(), texType, slot));

  return Textures.at(name);
}

Texture &ResourceManager::GetTexture(const std::string &name) {
  if (Textures.find(name) == Textures.end()) {
    std::cerr << "ResourceManager: Texture not found: " << name << std::endl;
    return Textures.begin()->second;
  }
  return Textures.at(name);
}

bool ResourceManager::HasTexture(const std::string &name) {
  return Textures.find(name) != Textures.end();
}

void ResourceManager::Clear() {
  for (auto &iter : Shaders)
    iter.second.Delete();
  for (auto &iter : Textures)
    iter.second.Delete();

  Shaders.clear();
  Textures.clear();
}
