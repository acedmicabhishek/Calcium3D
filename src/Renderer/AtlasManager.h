#ifndef ATLASMANAGER_H
#define ATLASMANAGER_H

#include "../Scene/Scene.h"
#include "Material.h"
#include "TextureAtlas.h"
#include <memory>

class AtlasManager {
public:
  static void BakeSceneTextures(Scene &scene, int atlasWidth = 4096,
                                int atlasHeight = 4096);
  static bool IsAtlased() { return s_GlobalAtlas != nullptr; }
  static unsigned int GetAtlasID() {
    return s_GlobalAtlas ? s_GlobalAtlas->GetID() : 0;
  }

private:
  static std::unique_ptr<TextureAtlas> s_GlobalAtlas;
};

#endif
