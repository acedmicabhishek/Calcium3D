#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

struct AtlasEntry {
  glm::vec2 offset;
  glm::vec2 size; 
};

class TextureAtlas {
public:
  TextureAtlas(int width, int height);
  ~TextureAtlas();

  bool AddTexture(const std::string &path);
  void Bake();

  unsigned int GetID() const { return m_AtlasID; }
  AtlasEntry GetEntry(const std::string &path) const;

private:
  int m_Width, m_Height;
  unsigned int m_AtlasID;
  std::map<std::string, AtlasEntry> m_Entries;

  struct RawTexture {
    std::string path;
    int w, h, channels;
    unsigned char *data;
  };
  std::vector<RawTexture> m_PendingTextures;

  struct Shelf {
    int x, y, h;
  };
  std::vector<Shelf> m_Shelves;
};

#endif
