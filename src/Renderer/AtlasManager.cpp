#include "AtlasManager.h"
#include "Core/Logger.h"
#include "Core/ResourceManager.h"
#include <set>

std::unique_ptr<TextureAtlas> AtlasManager::s_GlobalAtlas = nullptr;

void AtlasManager::BakeSceneTextures(Scene &scene, int atlasWidth,
                                     int atlasHeight) {
  Logger::AddLog("[AtlasManager] Starting scene-wide texture baking...");

  auto &objects = scene.GetObjects();
  std::set<std::string> uniqueTextures;

  
  for (auto &obj : objects) {
    if (obj.material.useTexture && !obj.material.diffusePath.empty()) {
      uniqueTextures.insert(obj.material.diffusePath);
    }
  }

  if (uniqueTextures.empty()) {
    Logger::AddLog("[AtlasManager] No textures found to atlas.");
    return;
  }

  
  s_GlobalAtlas = std::make_unique<TextureAtlas>(atlasWidth, atlasHeight);
  for (const auto &path : uniqueTextures) {
    s_GlobalAtlas->AddTexture(path);
  }
  s_GlobalAtlas->Bake();

  
  int remapCount = 0;
  for (auto &obj : objects) {
    if (obj.material.useTexture && !obj.material.diffusePath.empty()) {
      AtlasEntry entry = s_GlobalAtlas->GetEntry(obj.material.diffusePath);
      obj.mesh.RemapUVs(entry.offset, entry.size);

      
      obj.material.isAtlased = true;
      obj.material.atlasTextureID = s_GlobalAtlas->GetID();
      remapCount++;
    }
  }

  Logger::AddLog("[AtlasManager] Completed atlasing for %d objects.",
                 remapCount);
}
