#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>

struct Material {
  glm::vec3 albedo = glm::vec3(0.8f, 0.8f, 0.8f);
  float metallic = 0.0f;
  float roughness = 0.5f;
  float ao = 1.0f;
  float shininess = 32.0f;

  std::string diffuseTexture = "";
  std::string specularTexture = "";
  std::string customShaderName = "";
  float emission = 0.0f;
  glm::vec3 emissionColor = glm::vec3(1.0f);
  bool useEmissionMap = false;

  bool isEmissive = false;
  float intensity = 1.0f;

  bool useAlphaDiscard = false;
  bool useTexture = true;
  bool isTransparent = false;
  bool textureScaling = false;
  float textureScale = 1.0f;
  bool isAtlased = false;
  unsigned int atlasTextureID = 0;
  std::string diffusePath = "";

  nlohmann::json Serialize() const {
    nlohmann::json j;
    j["albedo"] = {albedo.r, albedo.g, albedo.b};
    j["metallic"] = metallic;
    j["roughness"] = roughness;
    j["ao"] = ao;
    j["shininess"] = shininess;
    j["diffuseTexture"] = diffuseTexture;
    j["specularTexture"] = specularTexture;
    j["customShaderName"] = customShaderName;
    j["intensity"] = intensity;
    j["useTexture"] = useTexture;
    j["isTransparent"] = isTransparent;
    j["useAlphaDiscard"] = useAlphaDiscard;
    j["textureScaling"] = textureScaling;
    j["textureScale"] = textureScale;
    return j;
  }

  void Deserialize(const nlohmann::json &j) {
    if (j.contains("albedo")) {
      auto a = j["albedo"];
      albedo =
          glm::vec3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
    }
    if (j.contains("metallic"))
      metallic = j["metallic"].get<float>();
    if (j.contains("roughness"))
      roughness = j["roughness"].get<float>();
    if (j.contains("ao"))
      ao = j["ao"].get<float>();
    if (j.contains("shininess"))
      shininess = j["shininess"].get<float>();
    if (j.contains("diffuseTexture"))
      diffuseTexture = j["diffuseTexture"].get<std::string>();
    if (j.contains("specularTexture"))
      specularTexture = j["specularTexture"].get<std::string>();
    if (j.contains("customShaderName"))
      customShaderName = j["customShaderName"].get<std::string>();
    if (j.contains("intensity"))
      intensity = j["intensity"].get<float>();
    if (j.contains("useTexture"))
      useTexture = j["useTexture"].get<bool>();
    if (j.contains("isTransparent"))
      isTransparent = j["isTransparent"].get<bool>();
    if (j.contains("useAlphaDiscard"))
      useAlphaDiscard = j["useAlphaDiscard"].get<bool>();
    if (j.contains("textureScaling"))
      textureScaling = j["textureScaling"].get<bool>();
    if (j.contains("textureScale"))
      textureScale = j["textureScale"].get<float>();
  }
};

#endif
